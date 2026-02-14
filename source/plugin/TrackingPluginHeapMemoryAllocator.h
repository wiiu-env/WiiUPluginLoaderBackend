#pragma once
#include "DefaultPluginHeapMemoryAllocator.h"

#include <optional>
#include <string>

#include <cstdint>

class TrackingPluginHeapMemoryAllocator : public IPluginHeapMemoryAllocator {
public:
    static std::optional<TrackingPluginHeapMemoryAllocator> Create(std::string_view pluginName, uint32_t stackTraceDepth);

    ~TrackingPluginHeapMemoryAllocator() override;

    TrackingPluginHeapMemoryAllocator(const TrackingPluginHeapMemoryAllocator &) = delete;

    TrackingPluginHeapMemoryAllocator(TrackingPluginHeapMemoryAllocator &&src) noexcept;

    TrackingPluginHeapMemoryAllocator &operator=(TrackingPluginHeapMemoryAllocator &&src) noexcept;

    MEMAllocFromDefaultHeapFn *GetAllocFunctionAddress() const override;
    MEMAllocFromDefaultHeapExFn *GetAllocExFunctionAddress() const override;
    MEMFreeToDefaultHeapFn *GetFreeFunctionAddress() const override;
    std::optional<PluginMemorySnapshot> GetHeapMemoryUsageSnapshot() const override;

private:
    int sIndex = -1;
    TrackingPluginHeapMemoryAllocator(int index);
};
