#include <plugin/PluginContainer.h>
#include <utils/ElfUtils.h>
#include <coreinit/cache.h>
#include <patcher/function_patcher.h>
#include <plugin/PluginMetaInformationFactory.h>
#include <plugin/PluginInformationFactory.h>
#include "patcher/hooks_patcher_static.h"
#include "patcher/hooks_patcher.h"
#include "PluginManagement.h"
#include "hooks.h"

bool PluginManagement::doRelocation(const std::vector<RelocationData> &relocData, relocation_trampolin_entry_t *tramp_data, uint32_t tramp_length, uint32_t trampolinID) {
    std::map<std::string, OSDynLoad_Module> moduleHandleCache;
    for (auto const &cur : relocData) {
        uint32_t functionAddress = 0;
        std::string functionName = cur.getName();

        if (functionName.compare("MEMAllocFromDefaultHeap") == 0) {
            OSDynLoad_Module rplHandle;
            OSDynLoad_Acquire("homebrew_memorymapping", &rplHandle);
            OSDynLoad_FindExport(rplHandle, 1, "MEMAllocFromMappedMemory", (void **) &functionAddress);
        } else if (functionName.compare("MEMAllocFromDefaultHeapEx") == 0) {
            OSDynLoad_Module rplHandle;
            OSDynLoad_Acquire("homebrew_memorymapping", &rplHandle);
            OSDynLoad_FindExport(rplHandle, 1, "MEMAllocFromMappedMemoryEx", (void **) &functionAddress);
        } else if (functionName.compare("MEMFreeToDefaultHeap") == 0) {
            OSDynLoad_Module rplHandle;
            OSDynLoad_Acquire("homebrew_memorymapping", &rplHandle);
            OSDynLoad_FindExport(rplHandle, 1, "MEMFreeToMappedMemory", (void **) &functionAddress);
        }

        if (functionAddress == 0) {
            std::string rplName = cur.getImportRPLInformation().getName();
            int32_t isData = cur.getImportRPLInformation().isData();
            OSDynLoad_Module rplHandle = 0;
            if (moduleHandleCache.count(rplName) > 0) {
                rplHandle = moduleHandleCache[rplName];
            } else {
                OSDynLoad_Acquire(rplName.c_str(), &rplHandle);
                moduleHandleCache[rplName] = rplHandle;
            }
            OSDynLoad_FindExport(rplHandle, isData, functionName.c_str(), (void **) &functionAddress);
        }

        if (functionAddress == 0) {
            DEBUG_FUNCTION_LINE("Failed to find export for %s", functionName.c_str());
            return false;
        } else {
            //DEBUG_FUNCTION_LINE("Found export for %s %s", rplName.c_str(), functionName.c_str());
        }
        if (!ElfUtils::elfLinkOne(cur.getType(), cur.getOffset(), cur.getAddend(), (uint32_t) cur.getDestination(), functionAddress, tramp_data, tramp_length, RELOC_TYPE_IMPORT, trampolinID)) {
            DEBUG_FUNCTION_LINE("Relocation failed\n");
            return false;
        }
    }

    DCFlushRange(tramp_data, tramp_length * sizeof(relocation_trampolin_entry_t));
    ICInvalidateRange(tramp_data, tramp_length * sizeof(relocation_trampolin_entry_t));
    return true;
}


void PluginManagement::doRelocations(const std::vector<PluginContainer> &plugins, relocation_trampolin_entry_t *trampData, uint32_t tramp_size) {
    for (auto &pluginContainer : plugins) {
        DEBUG_FUNCTION_LINE("Doing relocations for plugin: %s", pluginContainer.getMetaInformation().getName().c_str());

        if (!PluginManagement::doRelocation(pluginContainer.getPluginInformation().getRelocationDataList(), trampData, tramp_size, pluginContainer.getPluginInformation().getTrampolinId())) {
            DEBUG_FUNCTION_LINE("Relocation failed");
        }
    }
}

void PluginManagement::memsetBSS(const std::vector<PluginContainer> &plugins) {
    for (auto &pluginContainer : plugins) {
        auto sbssSection = pluginContainer.getPluginInformation().getSectionInfo(".sbss");
        if (sbssSection) {
            DEBUG_FUNCTION_LINE("memset .sbss %08X (%d)", sbssSection->getAddress(), sbssSection->getSize());
            memset((void *) sbssSection->getAddress(), 0, sbssSection->getSize());
        }
        auto bssSection = pluginContainer.getPluginInformation().getSectionInfo(".bss");
        if (bssSection) {
            DEBUG_FUNCTION_LINE("memset .bss %08X (%d)", bssSection->getAddress(), bssSection->getSize());
            memset((void *) bssSection->getAddress(), 0, bssSection->getSize());
        }
    }
}

void PluginManagement::callDeinitHooks(plugin_information_t *pluginInformation) {
    CallHook(pluginInformation, WUPS_LOADER_HOOK_RELEASE_FOREGROUND);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_APPLICATION_END);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_DEINIT_PLUGIN);

    CallHook(pluginInformation, WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_FINI_WUT_STDCPP);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_FINI_WUT_NEWLIB);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_FINI_WUT_MALLOC);

    DEBUG_FUNCTION_LINE("Done calling deinit hooks");
}


void PluginManagement::RestorePatches(plugin_information_t *pluginInformation, BOOL pluginOnly) {
    for (int32_t plugin_index = pluginInformation->number_used_plugins - 1; plugin_index >= 0; plugin_index--) {
        new_RestoreInvidualInstructions(&(pluginInformation->plugin_data[plugin_index].info));
    }
    if (!pluginOnly) {
        RestoreInvidualInstructions(method_hooks_hooks, method_hooks_size_hooks);
        RestoreInvidualInstructions(method_hooks_hooks_static, method_hooks_size_hooks_static);
    }
}

void PluginManagement::unloadPlugins(plugin_information_t *gPluginInformation, MEMHeapHandle pluginHeap, BOOL freePluginData) {
    RestorePatches(gPluginInformation, true);
    for (int32_t plugin_index = 0; plugin_index < gPluginInformation->number_used_plugins; plugin_index++) {
        plugin_information_single_t *plugin = &(gPluginInformation->plugin_data[plugin_index]);
        if (freePluginData) {
            if (plugin->data.buffer != nullptr) {
                if (plugin->data.memoryType == eMemTypeMEM2) {
                    DEBUG_FUNCTION_LINE("free %08X", plugin->data.buffer);
                    free(plugin->data.buffer);
                } else if (plugin->data.memoryType == eMemTypeExpHeap) {
                    DEBUG_FUNCTION_LINE("free %08X on EXP heap %08X", plugin->data.buffer, plugin->data.heapHandle);
                    MEMFreeToExpHeap((MEMHeapHandle) plugin->data.heapHandle, plugin->data.buffer);
                } else {
                    DEBUG_FUNCTION_LINE("########################");
                    DEBUG_FUNCTION_LINE("Failed to free memory from plugin");
                    DEBUG_FUNCTION_LINE("########################");
                }
                plugin->data.bufferLength = 0;
            } else {
                DEBUG_FUNCTION_LINE("Plugin has no copy of elf save in memory, can't free it");
            }
        }
        if (plugin->info.allocatedTextMemoryAddress != nullptr) {
            MEMFreeToExpHeap((MEMHeapHandle) pluginHeap, plugin->info.allocatedTextMemoryAddress);
            DEBUG_FUNCTION_LINE("Freed %08X", plugin->info.allocatedTextMemoryAddress);
        }
        if (plugin->info.allocatedDataMemoryAddress != nullptr) {
            MEMFreeToExpHeap((MEMHeapHandle) pluginHeap, plugin->info.allocatedDataMemoryAddress);
            DEBUG_FUNCTION_LINE("Freed %08X", plugin->info.allocatedDataMemoryAddress);
        }

        for (uint32_t i = 0; i < DYN_LINK_TRAMPOLIN_LIST_LENGTH; i++) {
            if (gPluginInformation->trampolines[i].id == plugin->info.trampolinId) {
                gPluginInformation->trampolines[i].id = 0;
                gPluginInformation->trampolines[i].status = RELOC_TRAMP_FREE;
            }
        }
    }

    memset((void *) gPluginInformation, 0, sizeof(plugin_information_t));
}


void PluginManagement::callInitHooks(plugin_information_t *pluginInformation) {
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_VID_MEM);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_KERNEL);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_OVERLAY);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_PLUGIN);
    DEBUG_FUNCTION_LINE("Done calling init hooks");
}

void PluginManagement::PatchFunctionsAndCallHooks(plugin_information_t *gPluginInformation) {
    DEBUG_FUNCTION_LINE("Patching functions");
    PatchInvidualMethodHooks(method_hooks_hooks_static, method_hooks_size_hooks_static, method_calls_hooks_static);
    PatchInvidualMethodHooks(method_hooks_hooks, method_hooks_size_hooks, method_calls_hooks);

    DCFlushRange((void *) 0x00800000, 0x00800000);
    ICInvalidateRange((void *) 0x00800000, 0x00800000);

    for (int32_t plugin_index = 0; plugin_index < gPluginInformation->number_used_plugins; plugin_index++) {
        CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_INIT_WUT_MALLOC, plugin_index);
        CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_INIT_WUT_NEWLIB, plugin_index);
        CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_INIT_WUT_STDCPP, plugin_index);
        CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB, plugin_index);
        CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_APPLICATION_START, plugin_index);
        new_PatchInvidualMethodHooks(&(gPluginInformation->plugin_data[plugin_index].info));
        CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_FUNCTIONS_PATCHED, plugin_index);
    }
}

std::vector<PluginContainer> PluginManagement::loadPlugins(const std::vector<PluginData> &pluginList, MEMHeapHandle heapHandle, relocation_trampolin_entry_t *trampolin_data, uint32_t trampolin_data_length) {
    std::vector<PluginContainer> plugins;

    for (auto &pluginData : pluginList) {
        DEBUG_FUNCTION_LINE("Load meta information");
        auto metaInfo = PluginMetaInformationFactory::loadPlugin(pluginData);
        if (metaInfo) {
            PluginContainer container;
            container.setMetaInformation(metaInfo.value());
            container.setPluginData(const_cast<PluginData &>(pluginData));
            plugins.push_back(container);
        } else {
            DEBUG_FUNCTION_LINE("Failed to get meta information");
        }
    }
    uint32_t trampolineID = 0;
    for (auto &pluginContainer : plugins) {
        std::optional<PluginInformation> info = PluginInformationFactory::load(pluginContainer.getPluginData(), heapHandle, trampolin_data, trampolin_data_length, trampolineID++);
        if (!info) {
            DEBUG_FUNCTION_LINE("Failed to load Plugin %s", pluginContainer.getMetaInformation().getName().c_str());
            continue;
        }
        pluginContainer.setPluginInformation(info.value());
    }
    return plugins;
}

