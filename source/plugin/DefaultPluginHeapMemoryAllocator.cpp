#include "DefaultPluginHeapMemoryAllocator.h"

#include "utils/logger.h"

#include <coreinit/dynload.h>

DefaultPluginHeapMemoryAllocator DefaultPluginHeapMemoryAllocator::gDefaultPluginHeapMemoryAllocator;

DefaultPluginHeapMemoryAllocator::DefaultPluginHeapMemoryAllocator() {
    OSDynLoad_Module rplHandle;
    if (OSDynLoad_Acquire("homebrew_memorymapping", &rplHandle) == OS_DYNLOAD_OK) {
        OSDynLoad_FindExport(rplHandle, OS_DYNLOAD_EXPORT_DATA, "MEMAllocFromMappedMemory", reinterpret_cast<void **>(&mMemoryMappingModuleAllocPtr));
        OSDynLoad_FindExport(rplHandle, OS_DYNLOAD_EXPORT_DATA, "MEMAllocFromMappedMemoryEx", reinterpret_cast<void **>(&mMemoryMappingModuleAllocExPtr));
        OSDynLoad_FindExport(rplHandle, OS_DYNLOAD_EXPORT_DATA, "MEMFreeToMappedMemory", reinterpret_cast<void **>(&mMemoryMappingModuleFreePtr));
    }
}

MEMAllocFromDefaultHeapFn *DefaultPluginHeapMemoryAllocator::GetAllocFunctionAddress() const {
    if (mMemoryMappingModuleAllocPtr == nullptr) {
        DEBUG_FUNCTION_LINE_WARN("### mMemoryMappingModuleAllocPtr was NULL!");
        return &MEMAllocFromDefaultHeap;
    }
    return mMemoryMappingModuleAllocPtr;
}

MEMAllocFromDefaultHeapExFn *DefaultPluginHeapMemoryAllocator::GetAllocExFunctionAddress() const {
    if (mMemoryMappingModuleAllocExPtr == nullptr) {
        DEBUG_FUNCTION_LINE_WARN("### mMemoryMappingModuleAllocExPtr was NULL!");
        return &MEMAllocFromDefaultHeapEx;
    }
    return mMemoryMappingModuleAllocExPtr;
}

MEMFreeToDefaultHeapFn *DefaultPluginHeapMemoryAllocator::GetFreeFunctionAddress() const {
    if (mMemoryMappingModuleFreePtr == nullptr) {
        DEBUG_FUNCTION_LINE_WARN("### mMemoryMappingModuleFreePtr was NULL!");
        return &MEMFreeToDefaultHeap;
    }
    return mMemoryMappingModuleFreePtr;
}

std::optional<PluginMemorySnapshot> DefaultPluginHeapMemoryAllocator::GetHeapMemoryUsageSnapshot() const {
    return {};
}