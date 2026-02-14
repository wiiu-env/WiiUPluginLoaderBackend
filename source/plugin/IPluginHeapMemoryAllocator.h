#pragma once

#include <coreinit/memdefaultheap.h>
#include <map>
#include <mutex>
#include <optional>
#include <vector>

#include <cstdint>

struct AllocationInfo {
    uint32_t size;
    std::vector<uint32_t> stackTrace;
};

struct PluginMemorySnapshot {
    uint32_t currentAllocated = 0;
    uint32_t peakAllocated    = 0;
    uint32_t allocCount       = 0;
    uint32_t freeCount        = 0;
    std::string pluginName    = {};

    std::map<void *, AllocationInfo> allocationMap;
};

class IPluginHeapMemoryAllocator {
public:
    virtual ~IPluginHeapMemoryAllocator() = default;

    /**
     * The returned address needs to be valid for the whole time
     */
    virtual MEMAllocFromDefaultHeapFn *GetAllocFunctionAddress() const = 0;

    /**
     * The returned address needs to be valid for the whole time
     */
    virtual MEMAllocFromDefaultHeapExFn *GetAllocExFunctionAddress() const = 0;

    /**
     * The returned address needs to be valid for the whole time
     */
    virtual MEMFreeToDefaultHeapFn *GetFreeFunctionAddress() const = 0;

    virtual std::optional<PluginMemorySnapshot> GetHeapMemoryUsageSnapshot() const = 0;
};