#include <whb/log.h>
#include <whb/log_udp.h>
#include <coreinit/debug.h>
#include <coreinit/memexpheap.h>
#include <coreinit/cache.h>
#include "plugin/PluginDataFactory.h"
#include "plugin/PluginContainerPersistence.h"
#include "plugin/PluginInformationFactory.h"
#include "plugin/PluginMetaInformationFactory.h"
#include "utils/utils.h"

#include "common/module_defines.h"
#include "PluginManagement.h"
#include "globals.h"
#include <whb/sdcard.h>
#include <utils/exports.h>
#include <wums.h>
#include <plugin/PluginDataPersistence.h>

WUMS_MODULE_EXPORT_NAME("homebrew_wupsbackend");

std::vector<PluginContainer> loadPlugins(const std::vector<PluginData> &pluginList, MEMHeapHandle heapHandle);

#define gModuleData ((module_information_t *) (0x00880000))

WUMS_INITIALIZE(){
}

WUMS_APPLICATION_STARTS() {
    WHBLogUdpInit();
    uint32_t upid = OSGetUPID();
    if (upid != 2 && upid != 15) {
        return;
    }
    bool initNeeded = false;
    if (pluginDataHeap == NULL) {
        DCFlushRange((void *) 0x00880000, sizeof(module_information_t));
        uint32_t endAddress = 0;
        DEBUG_FUNCTION_LINE("Using %d modules", gModuleData->number_used_modules);
        for (int i = 0; i < gModuleData->number_used_modules; i++) {
            DEBUG_FUNCTION_LINE("%08x", gModuleData->module_data[i].endAddress);
            uint32_t curEndAddr = gModuleData->module_data[i].endAddress;
            if (curEndAddr > endAddress) {
                endAddress = curEndAddr;
            }
        }

        memset((void *) &gLinkOnReload, 0, sizeof(gLinkOnReload));

        // If this address is 0, make sure the header common match the one
        // in the SetupPayload repo. (I know that's a bad idea)
        endAddress = (endAddress + 0x100) & 0xFFFFFF00;
        DEBUG_FUNCTION_LINE("endAddress: %08X", endAddress);

        DEBUG_FUNCTION_LINE("Create heap");
        pluginDataHeap = MEMCreateExpHeapEx((void *) (endAddress), 0x00FFF000 - endAddress, 0);

        if (pluginDataHeap != NULL) {
            if (gPluginInformation == NULL) {
                gPluginInformation = (plugin_information_t *) MEMAllocFromExpHeapEx(pluginDataHeap, sizeof(plugin_information_t), 4);
                if (gPluginInformation == NULL) {
                    DEBUG_FUNCTION_LINE("Failed to allocate global plugin information");
                    return;
                }
                memset((void *) gPluginInformation, 0, sizeof(plugin_information_t));

            }
            DEBUG_FUNCTION_LINE("MEMGetAllocatableSizeForExpHeapEx %d kb", MEMGetAllocatableSizeForExpHeapEx(pluginDataHeap, 4) / 1024);
            std::vector<PluginData> pluginList = PluginDataFactory::loadDir("fs:/vol/external01/wiiu/plugins/", pluginDataHeap);
            DEBUG_FUNCTION_LINE("Loaded %d plugin data", pluginList.size());

            std::vector<PluginContainer> plugins = PluginManagement::loadPlugins(pluginList, pluginDataHeap, gPluginInformation->trampolines, DYN_LINK_TRAMPOLIN_LIST_LENGTH);

            for (auto &pluginContainer : plugins) {
                for (const auto &kv : pluginContainer.getPluginInformation().getSectionInfoList()) {
                    DEBUG_FUNCTION_LINE("%s = %s %08X %d", kv.first.c_str(), kv.second.getName().c_str(), kv.second.getAddress(), kv.second.getSize());
                }
                if (!PluginContainerPersistence::savePlugin(gPluginInformation, pluginContainer)) {
                    DEBUG_FUNCTION_LINE("Failed to save plugin");
                }
            }
            initNeeded = true;
        }
    }
    if (gLinkOnReload.loadOnReload) {
        DEBUG_FUNCTION_LINE("We would now swap the plugins");
        std::vector<PluginData> pluginDataList;
        for (int32_t i = 0; i < gLinkOnReload.number_used_plugins; i++) {
            auto pluginData = PluginDataPersistence::load(&gLinkOnReload.plugin_data[i]);
            pluginDataList.push_back(pluginData);
        }

        for (int32_t plugin_index = 0; plugin_index < gPluginInformation->number_used_plugins; plugin_index++) {
            plugin_information_single_t *plugin = &(gPluginInformation->plugin_data[plugin_index]);
            BOOL doDelete = true;
            DEBUG_FUNCTION_LINE("Check if we can delete %08X", plugin->data.buffer);
            for (auto &pluginData: pluginDataList) {
                if (pluginData.buffer == plugin->data.buffer) {
                    DEBUG_FUNCTION_LINE("We can keep buffer %08X", plugin->data.buffer);
                    doDelete = false;
                    break;
                }
            }
            if (doDelete) {
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
                    plugin->data.buffer = NULL;
                    plugin->data.bufferLength = 0;
                } else {
                    DEBUG_FUNCTION_LINE("Plugin has no copy of elf save in memory, can't free it");
                }
            }
        }

        DEBUG_FUNCTION_LINE("unloadPlugins");
        PluginManagement::unloadPlugins(gPluginInformation, pluginDataHeap, false);

        std::vector<PluginContainer> plugins = PluginManagement::loadPlugins(pluginDataList, pluginDataHeap, gPluginInformation->trampolines, DYN_LINK_TRAMPOLIN_LIST_LENGTH);

        for (auto &pluginContainer : plugins) {
            DEBUG_FUNCTION_LINE("Saving %s from %s", pluginContainer.getMetaInformation().getName().c_str(), pluginContainer.getMetaInformation().getAuthor().c_str());
            if (!PluginContainerPersistence::savePlugin(gPluginInformation, pluginContainer)) {
                DEBUG_FUNCTION_LINE("Failed to save plugin");
            }
        }
        gLinkOnReload.loadOnReload = false;
        initNeeded = true;
    }

    if (pluginDataHeap != NULL) {
        std::vector<PluginContainer> plugins = PluginContainerPersistence::loadPlugins(gPluginInformation);
        PluginManagement::doRelocations(plugins, gPluginInformation->trampolines, DYN_LINK_TRAMPOLIN_LIST_LENGTH);
        // PluginManagement::memsetBSS(plugins);

        DCFlushRange((void *) 0x00800000, 0x00800000);
        ICInvalidateRange((void *) 0x00800000, 0x00800000);

        if (initNeeded) {
            PluginManagement::callInitHooks(gPluginInformation);
            initNeeded = false;
        }

        PluginManagement::PatchFunctionsAndCallHooks(gPluginInformation);
    }

    return;
}
