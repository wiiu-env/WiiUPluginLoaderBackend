#include <plugin/PluginContainer.h>
#include <utils/ElfUtils.h>
#include <coreinit/cache.h>
#include <patcher/function_patcher.h>
#include "patcher/hooks_patcher_static.h"
#include "patcher/hooks_patcher.h"
#include "PluginManagement.h"
#include "hooks.h"


bool PluginManagement::doRelocation(const std::vector<RelocationData> &relocData, relocation_trampolin_entry_t * tramp_data, uint32_t tramp_length, uint32_t trampolinID) {
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


void PluginManagement::doRelocations(std::vector<PluginContainer> plugins, relocation_trampolin_entry_t * trampData, uint32_t tramp_size) {
    for(auto & pluginContainer : plugins) {
        DEBUG_FUNCTION_LINE("Doing relocations for plugin: %s", pluginContainer.getMetaInformation().getName().c_str());

        if(!PluginManagement::doRelocation(pluginContainer.getPluginInformation().getRelocationDataList(), trampData, tramp_size, pluginContainer.getPluginInformation().getTrampolinId())) {
            DEBUG_FUNCTION_LINE("Relocation failed");
        }
    }
}

void PluginManagement::memsetBSS(std::vector<PluginContainer> plugins) {
    for(auto & pluginContainer : plugins) {
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
}

void PluginManagement::callInitHooks(plugin_information_t *pluginInformation) {
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_WUT_MALLOC);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_WUT_NEWLIB);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_WUT_STDCPP);

    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_VID_MEM);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_KERNEL);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_OVERLAY);
    CallHook(pluginInformation, WUPS_LOADER_HOOK_INIT_PLUGIN);
    DEBUG_FUNCTION_LINE("Done calling init hooks");
}

void PluginManagement::PatchFunctionsAndCallHooks(plugin_information_t* gPluginInformation) {
    DEBUG_FUNCTION_LINE("Patching functions");
    PatchInvidualMethodHooks(method_hooks_hooks_static, method_hooks_size_hooks_static, method_calls_hooks_static);
    PatchInvidualMethodHooks(method_hooks_hooks, method_hooks_size_hooks, method_calls_hooks);

    for(int32_t plugin_index=0; plugin_index<gPluginInformation->number_used_plugins; plugin_index++) {
        CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB,plugin_index);
        CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_APPLICATION_START,plugin_index);
        new_PatchInvidualMethodHooks(&(gPluginInformation->plugin_data[plugin_index].info));
        CallHookEx(gPluginInformation, WUPS_LOADER_HOOK_FUNCTIONS_PATCHED,plugin_index);
    }
}
