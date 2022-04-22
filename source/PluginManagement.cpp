#include <coreinit/cache.h>
#include <coreinit/dynload.h>
#include <coreinit/memdefaultheap.h>
#include <memory.h>
#include <memory>

#include "patcher/hooks_patcher_static.h"
#include "plugin/PluginContainer.h"
#include "plugin/PluginInformationFactory.h"
#include "plugin/PluginMetaInformationFactory.h"

#include "PluginManagement.h"
#include "globals.h"
#include "hooks.h"
#include "utils/ElfUtils.h"

bool PluginManagement::doRelocation(const std::vector<std::shared_ptr<RelocationData>> &relocData, relocation_trampoline_entry_t *tramp_data, uint32_t tramp_length, uint32_t trampolineID) {
    std::map<std::string, OSDynLoad_Module> moduleHandleCache;
    for (auto const &cur : relocData) {
        uint32_t functionAddress        = 0;
        const std::string &functionName = cur->getName();

        if (functionName == "MEMAllocFromDefaultHeap") {
            OSDynLoad_Module rplHandle;
            OSDynLoad_Acquire("homebrew_memorymapping", &rplHandle);
            OSDynLoad_FindExport(rplHandle, 1, "MEMAllocFromMappedMemory", (void **) &functionAddress);
        } else if (functionName == "MEMAllocFromDefaultHeapEx") {
            OSDynLoad_Module rplHandle;
            OSDynLoad_Acquire("homebrew_memorymapping", &rplHandle);
            OSDynLoad_FindExport(rplHandle, 1, "MEMAllocFromMappedMemoryEx", (void **) &functionAddress);
        } else if (functionName == "MEMFreeToDefaultHeap") {
            OSDynLoad_Module rplHandle;
            OSDynLoad_Acquire("homebrew_memorymapping", &rplHandle);
            OSDynLoad_FindExport(rplHandle, 1, "MEMFreeToMappedMemory", (void **) &functionAddress);
        }

        if (functionAddress == 0) {
            std::string rplName        = cur->getImportRPLInformation()->getName();
            int32_t isData             = cur->getImportRPLInformation()->isData();
            OSDynLoad_Module rplHandle = nullptr;
            if (moduleHandleCache.count(rplName) > 0) {
                rplHandle = moduleHandleCache[rplName];
            } else {
                OSDynLoad_Acquire(rplName.c_str(), &rplHandle);
                moduleHandleCache[rplName] = rplHandle;
            }
            OSDynLoad_FindExport(rplHandle, isData, functionName.c_str(), (void **) &functionAddress);
        }

        if (functionAddress == 0) {
            DEBUG_FUNCTION_LINE_ERR("Failed to find export for %s", functionName.c_str());
            return false;
        } else {
            //DEBUG_FUNCTION_LINE("Found export for %s %s", rplName.c_str(), functionName.c_str());
        }
        if (!ElfUtils::elfLinkOne(cur->getType(), cur->getOffset(), cur->getAddend(), (uint32_t) cur->getDestination(), functionAddress, tramp_data, tramp_length, RELOC_TYPE_IMPORT, trampolineID)) {
            DEBUG_FUNCTION_LINE_ERR("Relocation failed");
            return false;
        }
    }

    DCFlushRange(tramp_data, tramp_length * sizeof(relocation_trampoline_entry_t));
    ICInvalidateRange(tramp_data, tramp_length * sizeof(relocation_trampoline_entry_t));
    return true;
}


void PluginManagement::doRelocations(const std::vector<std::shared_ptr<PluginContainer>> &plugins, relocation_trampoline_entry_t *trampData, uint32_t tramp_size) {
    for (auto &pluginContainer : plugins) {
        DEBUG_FUNCTION_LINE_VERBOSE("Doing relocations for plugin: %s", pluginContainer->getMetaInformation()->getName().c_str());

        if (!PluginManagement::doRelocation(pluginContainer->getPluginInformation()->getRelocationDataList(), trampData, tramp_size, pluginContainer->getPluginInformation()->getTrampolineId())) {
            DEBUG_FUNCTION_LINE_ERR("Relocation failed");
        }
    }
}

void PluginManagement::memsetBSS(const std::vector<std::shared_ptr<PluginContainer>> &plugins) {
    for (auto &pluginContainer : plugins) {
        auto sbssSection = pluginContainer->getPluginInformation()->getSectionInfo(".sbss");
        if (sbssSection) {
            DEBUG_FUNCTION_LINE_VERBOSE("memset .sbss %08X (%d)", sbssSection.value()->getAddress(), sbssSection.value()->getSize());
            memset((void *) sbssSection.value()->getAddress(), 0, sbssSection.value()->getSize());
        }
        auto bssSection = pluginContainer->getPluginInformation()->getSectionInfo(".bss");
        if (bssSection) {
            DEBUG_FUNCTION_LINE_VERBOSE("memset .bss %08X (%d)", bssSection.value()->getAddress(), bssSection.value()->getSize());
            memset((void *) bssSection.value()->getAddress(), 0, bssSection.value()->getSize());
        }
    }
}

void PluginManagement::RestorePatches(plugin_information_t *pluginInformation, BOOL pluginOnly) {
    for (int32_t plugin_index = pluginInformation->number_used_plugins - 1; plugin_index >= 0; plugin_index--) {
        FunctionPatcherRestoreFunctions(pluginInformation->plugin_data[plugin_index].info.functions, pluginInformation->plugin_data[plugin_index].info.number_used_functions);
    }
    if (!pluginOnly) {
        FunctionPatcherRestoreFunctions(method_hooks_hooks_static, method_hooks_size_hooks_static);
    }
}

void PluginManagement::unloadPlugins(plugin_information_t *pluginInformation, MEMHeapHandle pluginHeap, BOOL freePluginData) {

    RestorePatches(pluginInformation, true);
    for (int32_t plugin_index = 0; plugin_index < pluginInformation->number_used_plugins; plugin_index++) {
        plugin_information_single_t *plugin = &(pluginInformation->plugin_data[plugin_index]);
        if (freePluginData) {
            if (plugin->data.buffer != nullptr) {
                if (plugin->data.memoryType == eMemTypeMEM2) {
                    DEBUG_FUNCTION_LINE_VERBOSE("Free plugin data buffer for %s [%08X]", plugin->meta.name, plugin->data.buffer);
                    free(plugin->data.buffer);
                } else if (plugin->data.memoryType == eMemTypeExpHeap) {
                    DEBUG_FUNCTION_LINE_VERBOSE("Free plugin data buffer for %s [%08X on heap %08X]", plugin->meta.name, plugin->data.buffer, plugin->data.heapHandle);
                    MEMFreeToExpHeap((MEMHeapHandle) plugin->data.heapHandle, plugin->data.buffer);
                } else {
                    DEBUG_FUNCTION_LINE_ERR("########################");
                    DEBUG_FUNCTION_LINE_ERR("Failed to free memory from plugin");
                    DEBUG_FUNCTION_LINE_ERR("########################");
                }
                plugin->data.buffer       = nullptr;
                plugin->data.bufferLength = 0;
            } else {
                DEBUG_FUNCTION_LINE_ERR("Plugin has no copy of elf saved in memory, can't free it");
            }
        }
        if (plugin->info.allocatedTextMemoryAddress != nullptr) {
            MEMFreeToExpHeap((MEMHeapHandle) pluginHeap, plugin->info.allocatedTextMemoryAddress);
            DEBUG_FUNCTION_LINE_VERBOSE("Deleted allocated .text section for plugin %s [%08X]", plugin->meta.name, plugin->info.allocatedTextMemoryAddress);
        }
        if (plugin->info.allocatedDataMemoryAddress != nullptr) {
            MEMFreeToExpHeap((MEMHeapHandle) pluginHeap, plugin->info.allocatedDataMemoryAddress);
            DEBUG_FUNCTION_LINE_VERBOSE("Deleted allocated .data section for plugin %s [%08X]", plugin->meta.name, plugin->info.allocatedDataMemoryAddress);
        }

        for (uint32_t i = 0; i < gTrampolineDataSize; i++) {
            auto trampoline = &(gTrampolineData[i]);
            if (trampoline->id == plugin->info.trampolineId) {
                trampoline->id     = 0;
                trampoline->status = RELOC_TRAMP_FREE;
            }
        }
    }

    memset((void *) pluginInformation, 0, sizeof(plugin_information_t));
}

void PluginManagement::callInitHooks(plugin_information_t *pluginInformation) {
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_STORAGE);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_PLUGIN);
    DEBUG_FUNCTION_LINE_VERBOSE("Done calling init hooks");
}

void module_callback(OSDynLoad_Module module,
                     void *userContext,
                     OSDynLoad_NotifyReason reason,
                     OSDynLoad_NotifyData *infos) {
    if (reason == OS_DYNLOAD_NOTIFY_LOADED) {
        auto *pluginInformation = (plugin_information_t *) userContext;
        for (int32_t plugin_index = 0; plugin_index < pluginInformation->number_used_plugins; plugin_index++) {
            FunctionPatcherPatchFunction(pluginInformation->plugin_data[plugin_index].info.functions, pluginInformation->plugin_data[plugin_index].info.number_used_functions);
        }
    }
}

void PluginManagement::PatchFunctionsAndCallHooks(plugin_information_t *pluginInformation) {
    DEBUG_FUNCTION_LINE_VERBOSE("Patching functions");
    FunctionPatcherPatchFunction(method_hooks_hooks_static, method_hooks_size_hooks_static);

    DCFlushRange((void *) 0x00800000, 0x00800000);
    ICInvalidateRange((void *) 0x00800000, 0x00800000);

    for (int32_t plugin_index = 0; plugin_index < pluginInformation->number_used_plugins; plugin_index++) {
        CallHookEx(pluginInformation, WUPS_LOADER_HOOK_APPLICATION_STARTS, plugin_index);
        FunctionPatcherPatchFunction(pluginInformation->plugin_data[plugin_index].info.functions, pluginInformation->plugin_data[plugin_index].info.number_used_functions);
        CallHookEx(pluginInformation, WUPS_LOADER_HOOK_FUNCTIONS_PATCHED, plugin_index);
    }
    OSDynLoad_AddNotifyCallback(module_callback, pluginInformation);
}

std::vector<std::shared_ptr<PluginContainer>>
PluginManagement::loadPlugins(const std::vector<std::shared_ptr<PluginData>> &pluginList, MEMHeapHandle heapHandle, relocation_trampoline_entry_t *trampoline_data, uint32_t trampoline_data_length) {
    std::vector<std::shared_ptr<PluginContainer>> plugins;

    for (auto &pluginData : pluginList) {
        DEBUG_FUNCTION_LINE_VERBOSE("Load meta information");
        auto metaInfo = PluginMetaInformationFactory::loadPlugin(pluginData);
        if (metaInfo) {
            auto container = std::make_shared<PluginContainer>();
            container->setMetaInformation(metaInfo.value());
            container->setPluginData(pluginData);
            plugins.push_back(container);
        } else {
            DEBUG_FUNCTION_LINE("Failed to get meta information");
        }
    }
    uint32_t trampolineID = 0;
    for (auto &pluginContainer : plugins) {
        auto info = PluginInformationFactory::load(pluginContainer->getPluginData(), heapHandle, trampoline_data, trampoline_data_length, trampolineID++);
        if (!info) {
            DEBUG_FUNCTION_LINE("Failed to load Plugin %s", pluginContainer->getMetaInformation()->getName().c_str());
            continue;
        }
        pluginContainer->setPluginInformation(info.value());
    }
    return plugins;
}
