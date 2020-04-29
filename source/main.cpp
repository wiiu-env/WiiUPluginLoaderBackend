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
#include "patcher/hooks_patcher_static.h"
#include "patcher/hooks_patcher.h"
#include "utils/ElfUtils.h"
#include "common/relocation_defines.h"
#include "common/plugin_defines.h"
#include "common/plugin_defines.h"
#include "common/module_defines.h"
#include "hooks.h"
#include <whb/sdcard.h>

MEMHeapHandle pluginDataHeap __attribute__((section(".data"))) = 0;
plugin_information_t * gPluginInformation __attribute__((section(".data"))) = NULL;

int test();

#define gModuleData ((module_information_t *) (0x00880000))

int main(int argc, char **argv) {
    test();
}

bool doRelocation(const std::vector<RelocationData> &relocData, relocation_trampolin_entry_t * tramp_data, uint32_t tramp_length, uint32_t trampolinID) {
    std::map<std::string,OSDynLoad_Module> moduleHandleCache;
    for (auto const& cur : relocData) {
        std::string functionName = cur.getName();
        std::string rplName = cur.getImportRPLInformation().getName();
        int32_t isData = cur.getImportRPLInformation().isData();
        OSDynLoad_Module rplHandle = 0;
        if(moduleHandleCache.count(rplName) > 0) {
            rplHandle = moduleHandleCache[rplName];
        } else {
            OSDynLoad_Acquire(rplName.c_str(), &rplHandle);
            moduleHandleCache[rplName] = rplHandle;
        }

        uint32_t functionAddress = 0;
        OSDynLoad_FindExport(rplHandle, isData, functionName.c_str(), (void**) &functionAddress);
        if(functionAddress == 0) {
            DEBUG_FUNCTION_LINE("Failed to find export for %s", functionName.c_str());
            return false;
        } else {
            //DEBUG_FUNCTION_LINE("Found export for %s %s", rplName.c_str(), functionName.c_str());
        }
        if(!ElfUtils::elfLinkOne(cur.getType(), cur.getOffset(), cur.getAddend(), (uint32_t) cur.getDestination(), functionAddress, tramp_data, tramp_length, RELOC_TYPE_IMPORT,trampolinID)) {
            DEBUG_FUNCTION_LINE("Relocation failed\n");
            return false;
        }
    }

    DCFlushRange(tramp_data, tramp_length * sizeof(relocation_trampolin_entry_t));
    ICInvalidateRange(tramp_data, tramp_length * sizeof(relocation_trampolin_entry_t));
    return true;
}

int test() {
    WHBLogUdpInit();
    log_init();
    bool pluginFreshLoaded = false;
    if(pluginDataHeap == NULL) {
        kernelInitialize();
        DEBUG_FUNCTION_LINE("Kernel init done");
        DCFlushRange((void*)0x00880000, sizeof(module_information_t));
        uint32_t endAddress = 0;
        DEBUG_FUNCTION_LINE("Using %d modules",gModuleData->number_used_modules);
        for(int i = 0; i < gModuleData->number_used_modules; i++) {
            DEBUG_FUNCTION_LINE("%08x",gModuleData->module_data[i].endAddress);
            uint32_t curEndAddr = gModuleData->module_data[i].endAddress;
            if(curEndAddr > endAddress) {
                endAddress = curEndAddr;
            }
        }
        // If this address is 0, make sure the header common match the one
        // in the SetupPayload repo. (I know thats a bad idea)
        endAddress = (endAddress + 0x100) & 0xFFFFFF00;
        DEBUG_FUNCTION_LINE("endAddress: %08X", endAddress);

        DEBUG_FUNCTION_LINE("Create heap");
        pluginDataHeap = MEMCreateExpHeapEx((void*) (endAddress), 0x00FFF000 - endAddress, 0);

        if(pluginDataHeap != NULL) {
            if(gPluginInformation == NULL) {
                gPluginInformation = (plugin_information_t*) MEMAllocFromExpHeapEx(pluginDataHeap, sizeof(plugin_information_t), 4);
                if(gPluginInformation == NULL) {
                    DEBUG_FUNCTION_LINE("Failed to allocate global plugin information");
                    return 0;
                }
            }
            DEBUG_FUNCTION_LINE("MEMGetAllocatableSizeForExpHeapEx %d kb", MEMGetAllocatableSizeForExpHeapEx(pluginDataHeap, 4)/1024);
            std::vector<PluginData> pluginList = PluginDataFactory::loadDir("fs:/vol/external01/wiiu/plugins/", pluginDataHeap);
            std::vector<PluginContainer> plugins;
            DEBUG_FUNCTION_LINE("Loaded %d plugin data", pluginList.size());
            for(auto & pluginData : pluginList) {
                DEBUG_FUNCTION_LINE("Load meta information");
                auto metaInfo = PluginMetaInformationFactory::loadPlugin(pluginData);
                if(metaInfo) {
                    PluginContainer container;
                    container.setMetaInformation(metaInfo.value());
                    container.setPluginData(pluginData);
                    plugins.push_back(container);
                }
            }
            uint8_t trampolinID = 0;
            for(auto & pluginContainer : plugins) {
                std::optional<PluginInformation> data = PluginInformationFactory::load(pluginContainer.getPluginData(), pluginDataHeap, gPluginInformation->trampolines, DYN_LINK_TRAMPOLIN_LIST_LENGTH, trampolinID);

                if(!data) {
                    DEBUG_FUNCTION_LINE("Failed to load Plugin %s", pluginContainer.getMetaInformation().getName().c_str());
                    break;
                }
                pluginContainer.setPluginInformation(data.value());

                for (const auto& kv : data->getSectionInfoList()) {
                    DEBUG_FUNCTION_LINE("%s = %s %08X %d", kv.first.c_str(), kv.second.getName().c_str(), kv.second.getAddress(), kv.second.getSize());
                }

                trampolinID++;

                PluginContainerPersistence::savePlugin(gPluginInformation, pluginContainer);
            }
            pluginFreshLoaded = true;
        }
    }

    if(pluginDataHeap != NULL) {
        std::vector<PluginContainer> plugins = PluginContainerPersistence::loadPlugins(gPluginInformation);
        for(auto & pluginContainer : plugins) {
            DEBUG_FUNCTION_LINE("Doing relocations for plugin: %s", pluginContainer.getMetaInformation().getName().c_str());

            if(!doRelocation(pluginContainer.getPluginInformation().getRelocationDataList(), gPluginInformation->trampolines, DYN_LINK_TRAMPOLIN_LIST_LENGTH, pluginContainer.getPluginInformation().getTrampolinId())) {
                DEBUG_FUNCTION_LINE("Relocation failed");
            }

            auto sbssSection = pluginContainer.getPluginInformation().getSectionInfo(".sbss");
            if(sbssSection){
                DEBUG_FUNCTION_LINE("memset .sbss %08X (%d)", sbssSection->getAddress(), sbssSection->getSize());
                memset((void*)sbssSection->getAddress(), 0, sbssSection->getSize());
            }
            auto bssSection = pluginContainer.getPluginInformation().getSectionInfo(".bss");
            if(bssSection) {
                DEBUG_FUNCTION_LINE("memset .bss %08X (%d)", bssSection->getAddress(), bssSection->getSize());
                memset((void*)bssSection->getAddress(), 0, bssSection->getSize());
            }

        }

        DCFlushRange((void*)0x00800000, 0x00800000);
        ICInvalidateRange((void*)0x00800000, 0x00800000);

        if(pluginFreshLoaded) {
            CallHook(gPluginInformation, WUPS_LOADER_HOOK_INIT_WUT_MALLOC);
            CallHook(gPluginInformation, WUPS_LOADER_HOOK_INIT_WUT_NEWLIB);
            CallHook(gPluginInformation, WUPS_LOADER_HOOK_INIT_WUT_STDCPP);

            CallHook(gPluginInformation, WUPS_LOADER_HOOK_INIT_VID_MEM);
            CallHook(gPluginInformation, WUPS_LOADER_HOOK_INIT_KERNEL);
            CallHook(gPluginInformation, WUPS_LOADER_HOOK_INIT_OVERLAY);
            CallHook(gPluginInformation, WUPS_LOADER_HOOK_INIT_PLUGIN);
            pluginFreshLoaded = false;
        }

        PatchInvidualMethodHooks(method_hooks_hooks_static, method_hooks_size_hooks_static, method_calls_hooks_static);
        PatchInvidualMethodHooks(method_hooks_hooks, method_hooks_size_hooks, method_calls_hooks);

        for(int32_t plugin_index=0; plugin_index<gPluginInformation->number_used_plugins; plugin_index++) {
            CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB,plugin_index);
            CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_APPLICATION_START,plugin_index);
            //new_PatchInvidualMethodHooks(&gbl_replacement_data.plugin_data[plugin_index]);
            CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_FUNCTIONS_PATCHED,plugin_index);
        }
    }

    return 0;
}


