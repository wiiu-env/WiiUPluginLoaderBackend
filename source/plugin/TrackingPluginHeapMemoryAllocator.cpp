#include "TrackingPluginHeapMemoryAllocator.h"

#include "utils/logger.h"
#include "utils/utils.h"

#include <array>

namespace {
    struct PluginMemoryStatsInternal {
        mutable std::mutex mutex;
        bool inUse               = false;
        uint32_t stackTraceDepth = 0;
        PluginMemorySnapshot data;
    };

    // Generate 64 different thunk functions so we can track 64 plugins at a time which should be more than enough
    constexpr int MAX_SLOTS = 64;
    template<int N>
    struct MemoryThunk {
        inline static PluginMemoryStatsInternal stats;

        static void *Alloc(uint32_t size) {
            const auto ptr = (*DefaultPluginHeapMemoryAllocator::gDefaultPluginHeapMemoryAllocator.GetAllocFunctionAddress())(size);

            if (ptr) {
                std::lock_guard lock(stats.mutex);
                if (stats.inUse) {
                    stats.data.allocCount++;
                    stats.data.currentAllocated += size;
                    if (stats.data.currentAllocated > stats.data.peakAllocated) {
                        stats.data.peakAllocated = stats.data.currentAllocated;
                    }

                    stats.data.allocationMap[ptr] = {size, stats.stackTraceDepth > 0 ? CaptureStackTrace(stats.stackTraceDepth) : std::vector<uint32_t>()};
                }
            }
            return ptr;
        }

        static void *AllocEx(uint32_t size, int align) {
            const auto ptr = (*DefaultPluginHeapMemoryAllocator::gDefaultPluginHeapMemoryAllocator.GetAllocExFunctionAddress())(size, align);
            if (ptr) {
                std::lock_guard lock(stats.mutex);
                if (stats.inUse) {
                    stats.data.allocCount++;
                    stats.data.currentAllocated += size;
                    if (stats.data.currentAllocated > stats.data.peakAllocated) {
                        stats.data.peakAllocated = stats.data.currentAllocated;
                    }
                    stats.data.allocationMap[ptr] = {size, stats.stackTraceDepth > 0 ? CaptureStackTrace(stats.stackTraceDepth) : std::vector<uint32_t>()};
                }
            }
            return ptr;
        }

        static void Free(void *ptr) {
            if (!ptr) return;

            std::lock_guard lock(stats.mutex);
            if (stats.inUse) {
                if (const auto it = stats.data.allocationMap.find(ptr); it != stats.data.allocationMap.end()) {
                    uint32_t size = it->second.size;
                    if (stats.data.currentAllocated >= size) {
                        stats.data.currentAllocated -= size;
                    } else {
                        stats.data.currentAllocated = 0;
                    }
                    stats.data.allocationMap.erase(it);
                } else {
                    DEBUG_FUNCTION_LINE_ERR("free() of for unknown ptr detected (double free?). \"%s\": %p", stats.data.pluginName.c_str(), ptr);
                    auto stackTrace = CaptureStackTrace(16);
                    PrintCapturedStackTrace(stackTrace);
                }
                stats.data.freeCount++;
            }

            (*DefaultPluginHeapMemoryAllocator::gDefaultPluginHeapMemoryAllocator.GetFreeFunctionAddress())(ptr);
        }
    };

    template<int... Is>
    constexpr std::array<MEMAllocFromDefaultHeapFn, sizeof...(Is)> CreateAllocTable(std::integer_sequence<int, Is...>) { return {MemoryThunk<Is>::Alloc...}; }
    template<int... Is>
    constexpr std::array<MEMAllocFromDefaultHeapExFn, sizeof...(Is)> CreateAllocExTable(std::integer_sequence<int, Is...>) { return {MemoryThunk<Is>::AllocEx...}; }
    template<int... Is>
    constexpr std::array<MEMFreeToDefaultHeapFn, sizeof...(Is)> CreateFreeTable(std::integer_sequence<int, Is...>) { return {MemoryThunk<Is>::Free...}; }

    template<int... Is>
    constexpr std::array<PluginMemoryStatsInternal *, sizeof...(Is)> CreateStatsTable(std::integer_sequence<int, Is...>) {
        return {&MemoryThunk<Is>::stats...};
    }

    auto sAllocThunks   = CreateAllocTable(std::make_integer_sequence<int, MAX_SLOTS>{});
    auto sAllocExThunks = CreateAllocExTable(std::make_integer_sequence<int, MAX_SLOTS>{});
    auto sFreeThunks    = CreateFreeTable(std::make_integer_sequence<int, MAX_SLOTS>{});
    auto sStatsTable    = CreateStatsTable(std::make_integer_sequence<int, MAX_SLOTS>{});
} // namespace

TrackingPluginHeapMemoryAllocator::TrackingPluginHeapMemoryAllocator(const int32_t index) : sIndex(index) {
}

std::optional<TrackingPluginHeapMemoryAllocator> TrackingPluginHeapMemoryAllocator::Create(const std::string_view pluginName, uint32_t stackTraceDepth) {
    int32_t index = 0;
    for (auto *stat : sStatsTable) {
        std::lock_guard lock(stat->mutex);
        if (!stat->inUse) {
            stat->inUse           = true;
            stat->stackTraceDepth = stackTraceDepth;
            stat->data            = {};
            stat->data.pluginName = pluginName;
            return TrackingPluginHeapMemoryAllocator{index};
        }
        ++index;
    }
    return std::nullopt;
}

TrackingPluginHeapMemoryAllocator::~TrackingPluginHeapMemoryAllocator() {
    if (sIndex < 0 || sIndex >= MAX_SLOTS) {
        return;
    }
    auto *stats = sStatsTable[sIndex];
    std::lock_guard lock(stats->mutex);
    stats->inUse = false;
}

TrackingPluginHeapMemoryAllocator::TrackingPluginHeapMemoryAllocator(TrackingPluginHeapMemoryAllocator &&src) noexcept {
    this->sIndex = src.sIndex;
    src.sIndex   = -1;
}

TrackingPluginHeapMemoryAllocator &TrackingPluginHeapMemoryAllocator::operator=(TrackingPluginHeapMemoryAllocator &&src) noexcept {
    if (this != &src) {
        // Release current slot if we have one
        if (this->sIndex >= 0 && this->sIndex < MAX_SLOTS) {
            auto *stats = sStatsTable[this->sIndex];
            std::lock_guard lock(stats->mutex);
            stats->inUse = false;
            stats->data  = {};
        }

        this->sIndex = src.sIndex;
        src.sIndex   = -1;
    }
    return *this;
}

MEMAllocFromDefaultHeapFn *TrackingPluginHeapMemoryAllocator::GetAllocFunctionAddress() const {
    if (sIndex < 0 || sIndex >= MAX_SLOTS) {
        return DefaultPluginHeapMemoryAllocator::gDefaultPluginHeapMemoryAllocator.GetAllocFunctionAddress();
    }

    return &sAllocThunks[sIndex];
}

MEMAllocFromDefaultHeapExFn *TrackingPluginHeapMemoryAllocator::GetAllocExFunctionAddress() const {
    if (sIndex < 0 || sIndex >= MAX_SLOTS) {
        return DefaultPluginHeapMemoryAllocator::gDefaultPluginHeapMemoryAllocator.GetAllocExFunctionAddress();
    }
    return &sAllocExThunks[sIndex];
}

MEMFreeToDefaultHeapFn *TrackingPluginHeapMemoryAllocator::GetFreeFunctionAddress() const {
    if (sIndex < 0 || sIndex >= MAX_SLOTS) {
        return DefaultPluginHeapMemoryAllocator::gDefaultPluginHeapMemoryAllocator.GetFreeFunctionAddress();
    }
    return &sFreeThunks[sIndex];
}

std::optional<PluginMemorySnapshot> TrackingPluginHeapMemoryAllocator::GetHeapMemoryUsageSnapshot() const {
    if (sIndex < 0 || sIndex >= MAX_SLOTS) {
        return std::nullopt;
    }
    const auto *stats = sStatsTable[sIndex];
    std::lock_guard lock(stats->mutex);
    return stats->data;
}
