#include "PluginManagement.h"
#include "hooks.h"
#include "patcher/hooks_patcher_static.h"
#include "plugin/PluginContainer.h"
#include "plugin/PluginInformationFactory.h"
#include "plugin/PluginMetaInformationFactory.h"
#include "utils/ElfUtils.h"
#include "utils/utils.h"
#include <coreinit/cache.h>
#include <coreinit/dynload.h>
#include <forward_list>
#include <memory.h>
#include <memory>
#include <ranges>

bool PluginManagement::doRelocation(const std::vector<std::unique_ptr<RelocationData>> &relocData,
                                    relocation_trampoline_entry_t *tramp_data,
                                    uint32_t tramp_length,
                                    uint32_t trampolineID,
                                    std::map<std::string, OSDynLoad_Module> &usedRPls) {
    for (auto const &cur : relocData) {
        uint32_t functionAddress        = 0;
        const std::string &functionName = cur->getName();

        if (functionName == "MEMAllocFromDefaultHeap") {
            OSDynLoad_Module rplHandle;
            OSDynLoad_Acquire("homebrew_memorymapping", &rplHandle);
            OSDynLoad_FindExport(rplHandle, OS_DYNLOAD_EXPORT_DATA, "MEMAllocFromMappedMemory", (void **) &functionAddress);
        } else if (functionName == "MEMAllocFromDefaultHeapEx") {
            OSDynLoad_Module rplHandle;
            OSDynLoad_Acquire("homebrew_memorymapping", &rplHandle);
            OSDynLoad_FindExport(rplHandle, OS_DYNLOAD_EXPORT_DATA, "MEMAllocFromMappedMemoryEx", (void **) &functionAddress);
        } else if (functionName == "MEMFreeToDefaultHeap") {
            OSDynLoad_Module rplHandle;
            OSDynLoad_Acquire("homebrew_memorymapping", &rplHandle);
            OSDynLoad_FindExport(rplHandle, OS_DYNLOAD_EXPORT_DATA, "MEMFreeToMappedMemory", (void **) &functionAddress);
        }

        if (functionAddress == 0) {
            auto rplName               = cur->getImportRPLInformation()->getRPLName();
            int32_t isData             = cur->getImportRPLInformation()->isData();
            OSDynLoad_Module rplHandle = nullptr;

            if (!usedRPls.contains(rplName)) {
                DEBUG_FUNCTION_LINE_VERBOSE("Acquire %s", rplName.c_str());
                // Always acquire to increase refcount and make sure it won't get unloaded while we're using it.
                OSDynLoad_Error err = OSDynLoad_Acquire(rplName.c_str(), &rplHandle);
                if (err != OS_DYNLOAD_OK) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to acquire %s", rplName.c_str());
                    return false;
                }
                // Keep track RPLs we are using.
                // They will be released on exit
                usedRPls[rplName] = rplHandle;
            } else {
                rplHandle = usedRPls[rplName];
            }

            OSDynLoad_FindExport(rplHandle, (OSDynLoad_ExportType) isData, functionName.c_str(), (void **) &functionAddress);
        }

        if (functionAddress == 0) {
            DEBUG_FUNCTION_LINE_ERR("Failed to find export for %s", functionName.c_str());
            return false;
        } else {
            //DEBUG_FUNCTION_LINE("Found export for %s %s", rplName.c_str(), functionName.c_str());
        }

        if (!ElfUtils::elfLinkOne(cur->getType(), cur->getOffset(), cur->getAddend(), (uint32_t) cur->getDestination(), functionAddress, tramp_data, tramp_length, RELOC_TYPE_IMPORT, trampolineID)) {
            DEBUG_FUNCTION_LINE_ERR("elfLinkOne failed");
            return false;
        }
    }

    // Unloading RPLs which you want to use is stupid.
    // Never uncomment this again.
    /* for (auto &cur : moduleHandleCache) {
        // Release handle if they are not from DynLoadPatchModule
        if (((uint32_t) cur.second & 0xFFFF0000) != 0x13370000) {
            OSDynLoad_Release(cur.second);
        }
    } */

    DCFlushRange(tramp_data, tramp_length * sizeof(relocation_trampoline_entry_t));
    ICInvalidateRange(tramp_data, tramp_length * sizeof(relocation_trampoline_entry_t));
    OSMemoryBarrier();
    return true;
}

bool PluginManagement::doRelocations(const std::vector<std::unique_ptr<PluginContainer>> &plugins,
                                     relocation_trampoline_entry_t *trampData, uint32_t tramp_size,
                                     std::map<std::string, OSDynLoad_Module> &usedRPls) {
    for (uint32_t i = 0; i < tramp_size; i++) {
        if (trampData[i].status == RELOC_TRAMP_IMPORT_DONE) {
            trampData[i].status = RELOC_TRAMP_FREE;
        }
    }

    OSDynLoadAllocFn prevDynLoadAlloc = nullptr;
    OSDynLoadFreeFn prevDynLoadFree   = nullptr;

    OSDynLoad_GetAllocator(&prevDynLoadAlloc, &prevDynLoadFree);
    OSDynLoad_SetAllocator(CustomDynLoadAlloc, CustomDynLoadFree);

    for (auto &pluginContainer : plugins) {
        DEBUG_FUNCTION_LINE_VERBOSE("Doing relocations for plugin: %s", pluginContainer->getMetaInformation()->getName().c_str());
        if (!PluginManagement::doRelocation(pluginContainer->getPluginInformation()->getRelocationDataList(),
                                            trampData,
                                            tramp_size,
                                            pluginContainer->getPluginInformation()->getTrampolineId(),
                                            usedRPls)) {
            return false;
        }
    }

    OSDynLoad_SetAllocator(prevDynLoadAlloc, prevDynLoadFree);

    return true;
}

bool PluginManagement::RestoreFunctionPatches(const std::vector<std::unique_ptr<PluginContainer>> &plugins) {
    for (const auto &cur : std::ranges::reverse_view(plugins)) {
        for (const auto &curFunction : std::ranges::reverse_view(cur->getPluginInformation()->getFunctionDataList())) {
            if (!curFunction->RemovePatch()) {
                return false;
            }
        }
    }
    return true;
}

bool PluginManagement::DoFunctionPatches(const std::vector<std::unique_ptr<PluginContainer>> &plugins) {
    for (const auto &cur : plugins) {
        for (const auto &curFunction : cur->getPluginInformation()->getFunctionDataList()) {
            if (!curFunction->AddPatch()) {
                return false;
            }
        }
    }
    return true;
}

void PluginManagement::callInitHooks(const std::vector<std::unique_ptr<PluginContainer>> &plugins) {
    CallHook(plugins, WUPS_LOADER_HOOK_INIT_STORAGE);
    CallHook(plugins, WUPS_LOADER_HOOK_INIT_PLUGIN);
    DEBUG_FUNCTION_LINE_VERBOSE("Done calling init hooks");
}

std::vector<std::unique_ptr<PluginContainer>>
PluginManagement::loadPlugins(const std::forward_list<std::shared_ptr<PluginData>> &pluginList, relocation_trampoline_entry_t *trampoline_data, uint32_t trampoline_data_length) {
    std::vector<std::unique_ptr<PluginContainer>> plugins;

    uint32_t trampolineID = 0;
    for (auto &pluginData : pluginList) {
        auto metaInfo = PluginMetaInformationFactory::loadPlugin(pluginData);
        if (metaInfo) {
            auto info = PluginInformationFactory::load(pluginData, trampoline_data, trampoline_data_length, trampolineID++);
            if (!info) {
                DEBUG_FUNCTION_LINE_ERR("Failed to load Plugin %s", metaInfo.value()->getName().c_str());
                continue;
            }
            auto container = make_unique_nothrow<PluginContainer>(std::move(metaInfo.value()), std::move(info.value()), pluginData);
            if (!container) {
                DEBUG_FUNCTION_LINE_ERR("Failed to create PluginContainer, skipping  %s", metaInfo.value()->getName().c_str());
                continue;
            }
            plugins.push_back(std::move(container));
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to get meta information");
        }
    }

    if (!PluginManagement::DoFunctionPatches(plugins)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to patch functions");
        OSFatal("Failed to patch functions");
    }

    return plugins;
}