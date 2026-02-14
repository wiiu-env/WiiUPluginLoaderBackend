#pragma once
#include "IPluginHeapMemoryAllocator.h"

class DefaultPluginHeapMemoryAllocator : public IPluginHeapMemoryAllocator {
public:
    ~DefaultPluginHeapMemoryAllocator() override = default;
    MEMAllocFromDefaultHeapFn *GetAllocFunctionAddress() const override;
    MEMAllocFromDefaultHeapExFn *GetAllocExFunctionAddress() const override;
    MEMFreeToDefaultHeapFn *GetFreeFunctionAddress() const override;
    std::optional<PluginMemorySnapshot> GetHeapMemoryUsageSnapshot() const override;

    static DefaultPluginHeapMemoryAllocator gDefaultPluginHeapMemoryAllocator;

private:
    DefaultPluginHeapMemoryAllocator();

    MEMAllocFromDefaultHeapFn *mMemoryMappingModuleAllocPtr     = nullptr;
    MEMAllocFromDefaultHeapExFn *mMemoryMappingModuleAllocExPtr = nullptr;
    MEMFreeToDefaultHeapFn *mMemoryMappingModuleFreePtr         = nullptr;
};
