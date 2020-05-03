#include <wups.h>
#include <whb/log.h>
#include <whb/log_udp.h>
#include <exception>
#include <sysapp/launch.h>
#include <coreinit/memexpheap.h>
#include <coreinit/dynload.h>
#include <coreinit/cache.h>
#include "plugin/PluginDataFactory.h"
#include "plugin/PluginContainerPersistence.h"
#include "plugin/PluginInformationFactory.h"
#include "plugin/PluginMetaInformationFactory.h"
#include "plugin/FunctionData.h"
#include "plugin/PluginContainer.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "kernel/kernel_utils.h"

#include "utils/ElfUtils.h"
#include "common/relocation_defines.h"
#include "common/plugin_defines.h"
#include "common/plugin_defines.h"
#include "common/module_defines.h"
#include "hooks.h"
#include "PluginManagement.h"
#include <whb/sdcard.h>

MEMHeapHandle pluginDataHeap __attribute__((section(".data"))) = 0;
plugin_information_t *gPluginInformation __attribute__((section(".data"))) = NULL;

int test();

#define gModuleData ((module_information_t *) (0x00880000))

int main(int argc, char **argv) {
    test();
}

int test() {
    WHBLogUdpInit();
    log_init();
    bool initNeeded = false;
    if (pluginDataHeap == NULL) {
        kernelInitialize();
        DEBUG_FUNCTION_LINE("Kernel init done");
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
                    return 0;
                }
                memset((void *) gPluginInformation, 0, sizeof(plugin_information_t));

            }
            DEBUG_FUNCTION_LINE("MEMGetAllocatableSizeForExpHeapEx %d kb", MEMGetAllocatableSizeForExpHeapEx(pluginDataHeap, 4) / 1024);
            std::vector<PluginData> pluginList = PluginDataFactory::loadDir("fs:/vol/external01/wiiu/plugins/", pluginDataHeap);
            std::vector<PluginContainer> plugins;
            DEBUG_FUNCTION_LINE("Loaded %d plugin data", pluginList.size());

            for (auto &pluginData : pluginList) {
                DEBUG_FUNCTION_LINE("Load meta information");
                auto metaInfo = PluginMetaInformationFactory::loadPlugin(pluginData);
                if (metaInfo) {
                    PluginContainer container;
                    container.setMetaInformation(metaInfo.value());
                    container.setPluginData(pluginData);
                    plugins.push_back(container);
                } else {
                    DEBUG_FUNCTION_LINE("Failed to get meta information");
                }
            }
            for (auto &pluginContainer : plugins) {
                uint32_t trampolineId = pluginContainer.getPluginInformation().getTrampolinId();
                std::optional<PluginInformation> data = PluginInformationFactory::load(pluginContainer.getPluginData(), pluginDataHeap, gPluginInformation->trampolines, DYN_LINK_TRAMPOLIN_LIST_LENGTH, trampolineId);

                if (!data) {
                    DEBUG_FUNCTION_LINE("Failed to load Plugin %s", pluginContainer.getMetaInformation().getName().c_str());
                    break;
                }
                pluginContainer.setPluginInformation(data.value());

                for (const auto &kv : data->getSectionInfoList()) {
                    DEBUG_FUNCTION_LINE("%s = %s %08X %d", kv.first.c_str(), kv.second.getName().c_str(), kv.second.getAddress(), kv.second.getSize());
                }
                if (!PluginContainerPersistence::savePlugin(gPluginInformation, pluginContainer)) {
                    DEBUG_FUNCTION_LINE("Failed to save plugin");
                }
            }
            initNeeded = true;
        }
    }

    if (pluginDataHeap != NULL) {
        std::vector<PluginContainer> plugins = PluginContainerPersistence::loadPlugins(gPluginInformation);
        PluginManagement::doRelocations(plugins, gPluginInformation->trampolines, DYN_LINK_TRAMPOLIN_LIST_LENGTH);
        PluginManagement::memsetBSS(plugins);

        DCFlushRange((void *) 0x00800000, 0x00800000);
        ICInvalidateRange((void *) 0x00800000, 0x00800000);

        if (initNeeded) {
            PluginManagement::callInitHooks(gPluginInformation);
            initNeeded = false;
        }

        PluginManagement::PatchFunctionsAndCallHooks(gPluginInformation);
    }

    return 0;
}
