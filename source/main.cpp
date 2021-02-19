#include <whb/log_udp.h>
#include <wums.h>
#include <coreinit/debug.h>
#include <coreinit/cache.h>
#include "plugin/PluginContainer.h"
#include "globals.h"
#include "plugin/PluginDataFactory.h"
#include "plugin/PluginDataPersistence.h"
#include "plugin/PluginContainerPersistence.h"
#include "PluginManagement.h"
#include "hooks.h"
#include "patcher/hooks_patcher_static.h"

WUMS_MODULE_EXPORT_NAME("homebrew_wupsbackend");

WUMS_INITIALIZE(args) {
    WHBLogUdpInit();

    gModuleData = args.module_information;
    if (gModuleData == nullptr) {
        OSFatal("WUPS-Backend: Failed to get gModuleData pointer.");
    }
    if (gModuleData->version != MODULE_INFORMATION_VERSION) {
        OSFatal("WUPS-Backend: The module information struct version does not match.");
    }
    WHBLogPrintf("Init successful");
}

WUMS_APPLICATION_ENDS() {
    DEBUG_FUNCTION_LINE("Reset alreadyPatched flags for dynamic functions");
    auto pluginInformation = gPluginInformation;
    for (int32_t plugin_index = pluginInformation->number_used_plugins - 1; plugin_index >= 0; plugin_index--) {
        FunctionPatcherRestoreDynamicFunctions(pluginInformation->plugin_data[plugin_index].info.functions, pluginInformation->plugin_data[plugin_index].info.number_used_functions);
    }
    FunctionPatcherRestoreDynamicFunctions(method_hooks_hooks_static, method_hooks_size_hooks_static);
    DEBUG_FUNCTION_LINE("Call hooks");
    CallHook(gPluginInformation, WUPS_LOADER_HOOK_APPLICATION_END);
    CallHook(gPluginInformation, WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB);
}

WUMS_APPLICATION_STARTS() {
    WHBLogUdpInit();
    uint32_t upid = OSGetUPID();
    if (upid != 2 && upid != 15) {
        return;
    }
    bool initNeeded = false;
    if (pluginDataHeap == nullptr) {
        DEBUG_FUNCTION_LINE_VERBOSE("gModuleData = %08X", gModuleData);
        DCFlushRange((void *) gModuleData, sizeof(module_information_t));
        uint32_t endAddress = 0;
        DEBUG_FUNCTION_LINE_VERBOSE("Using %d modules", gModuleData->number_used_modules);
        for (int i = 0; i < gModuleData->number_used_modules; i++) {
            uint32_t curEndAddr = gModuleData->module_data[i].endAddress;
            if (curEndAddr > endAddress) {
                endAddress = curEndAddr;
            }
        }

        memset((void *) &gLinkOnReload, 0, sizeof(gLinkOnReload));
        // If this address is 0, make sure the header common match the one
        // in the SetupPayload repo. (I know that's a bad idea)
        endAddress = (endAddress + 0x100) & 0xFFFFFF00;

        gPluginDataHeapSize = 0x00FFF000 - endAddress;

        DEBUG_FUNCTION_LINE("Create heap to store plugins");
        pluginDataHeap = MEMCreateExpHeapEx((void *) (endAddress), gPluginDataHeapSize, 0);

        if (pluginDataHeap != nullptr) {
            if (gPluginInformation == nullptr) {
                DEBUG_FUNCTION_LINE_VERBOSE("Allocate gPluginInformation on heap %08X (size: %d bytes)", pluginDataHeap, sizeof(plugin_information_t));
                gPluginInformation = (plugin_information_t *) MEMAllocFromExpHeapEx(pluginDataHeap, sizeof(plugin_information_t), 4);
                if (gPluginInformation == nullptr) {
                    DEBUG_FUNCTION_LINE("Failed to allocate global plugin information");
                    return;
                }
                memset((void *) gPluginInformation, 0, sizeof(plugin_information_t));
            }
            DEBUG_FUNCTION_LINE("Available memory for storing plugins: %d kb", MEMGetAllocatableSizeForExpHeapEx(pluginDataHeap, 4) / 1024);
            std::vector<PluginData> pluginList = PluginDataFactory::loadDir("fs:/vol/external01/wiiu/plugins/", pluginDataHeap);
            DEBUG_FUNCTION_LINE("Loaded data for %d plugins.", pluginList.size());

            std::vector<PluginContainer> plugins = PluginManagement::loadPlugins(pluginList, pluginDataHeap, gPluginInformation->trampolines, DYN_LINK_TRAMPOLIN_LIST_LENGTH);

            for (auto &pluginContainer : plugins) {
                for (const auto &kv : pluginContainer.getPluginInformation().getSectionInfoList()) {
                    DEBUG_FUNCTION_LINE_VERBOSE("%s = %s %08X %d", kv.first.c_str(), kv.second.getName().c_str(), kv.second.getAddress(), kv.second.getSize());
                }
                if (!PluginContainerPersistence::savePlugin(gPluginInformation, pluginContainer)) {
                    DEBUG_FUNCTION_LINE("Failed to save plugin");
                }
            }
            initNeeded = true;
        }
    }
    if (gLinkOnReload.loadOnReload) {
        DEBUG_FUNCTION_LINE("Reload with new plugin list.");
        std::vector<PluginData> pluginDataList;
        for (int32_t i = 0; i < gLinkOnReload.number_used_plugins; i++) {
            auto pluginData = PluginDataPersistence::load(&gLinkOnReload.plugin_data[i]);
            pluginDataList.push_back(pluginData);
        }

        for (int32_t plugin_index = 0; plugin_index < gPluginInformation->number_used_plugins; plugin_index++) {
            plugin_information_single_t *plugin = &(gPluginInformation->plugin_data[plugin_index]);
            BOOL doDelete = true;
            for (auto &pluginData: pluginDataList) {
                if (pluginData.buffer == plugin->data.buffer) {
                    doDelete = false;
                    break;
                }
            }
            if (doDelete) {
                DEBUG_FUNCTION_LINE("We need to delete the plugin data for plugin %s", plugin->meta.name);
                if (plugin->data.buffer != nullptr) {
                    if (plugin->data.memoryType == eMemTypeMEM2) {
                        DEBUG_FUNCTION_LINE_VERBOSE("Free plugin data buffer for %s [%08X]", plugin->meta.name, plugin->data.buffer);
                        free(plugin->data.buffer);
                    } else if (plugin->data.memoryType == eMemTypeExpHeap) {
                        DEBUG_FUNCTION_LINE_VERBOSE("Free plugin data buffer for %s [%08X on heap %08X]", plugin->meta.name, plugin->data.buffer, plugin->data.heapHandle);
                        MEMFreeToExpHeap((MEMHeapHandle) plugin->data.heapHandle, plugin->data.buffer);
                    } else {
                        DEBUG_FUNCTION_LINE("########################");
                        DEBUG_FUNCTION_LINE("Failed to free memory from plugin");
                        DEBUG_FUNCTION_LINE("########################");
                    }
                    plugin->data.buffer = nullptr;
                    plugin->data.bufferLength = 0;
                } else {
                    DEBUG_FUNCTION_LINE("Plugin %s has no copy of elf saved in memory, can't free it", plugin->meta.name);
                }
            }
        }

        PluginManagement::unloadPlugins(gPluginInformation, pluginDataHeap, false);

        std::vector<PluginContainer> plugins = PluginManagement::loadPlugins(pluginDataList, pluginDataHeap, gPluginInformation->trampolines, DYN_LINK_TRAMPOLIN_LIST_LENGTH);

        for (auto &pluginContainer : plugins) {
            DEBUG_FUNCTION_LINE("Stored information for plugin %s ; %s", pluginContainer.getMetaInformation().getName().c_str(), pluginContainer.getMetaInformation().getAuthor().c_str());
            if (!PluginContainerPersistence::savePlugin(gPluginInformation, pluginContainer)) {
                DEBUG_FUNCTION_LINE("Failed to save plugin");
            }
        }
        gLinkOnReload.loadOnReload = false;
        initNeeded = true;
    }

    if (pluginDataHeap != nullptr) {
        std::vector<PluginContainer> plugins = PluginContainerPersistence::loadPlugins(gPluginInformation);
        PluginManagement::doRelocations(plugins, gPluginInformation->trampolines, DYN_LINK_TRAMPOLIN_LIST_LENGTH);
        // PluginManagement::memsetBSS(plugins);

        DCFlushRange((void *) pluginDataHeap, gPluginDataHeapSize);
        ICInvalidateRange((void *) pluginDataHeap, gPluginDataHeapSize);

        if (initNeeded) {
            PluginManagement::callInitHooks(gPluginInformation);
        }

        PluginManagement::PatchFunctionsAndCallHooks(gPluginInformation);
    }
}
