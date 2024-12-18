#include "PluginManagement.h"
#include "NotificationsUtils.h"
#include "hooks.h"
#include "plugin/PluginContainer.h"
#include "plugin/PluginLinkInformationFactory.h"
#include "plugin/PluginLoadWrapper.h"
#include "plugin/PluginMetaInformationFactory.h"
#include "utils/ElfUtils.h"
#include "utils/StringTools.h"
#include "utils/utils.h"
#include <coreinit/cache.h>
#include <coreinit/dynload.h>
#include <memory.h>
#include <ranges>

static uint32_t sTrampolineID = 0;

std::vector<PluginContainer>
PluginManagement::loadPlugins(const std::vector<PluginLoadWrapper> &pluginDataList, std::vector<relocation_trampoline_entry_t> &trampolineData) {
    std::vector<PluginContainer> plugins;

    for (const auto &pluginDataWrapper : pluginDataList) {
        PluginParseErrors error = PLUGIN_PARSE_ERROR_UNKNOWN;
        auto metaInfo           = PluginMetaInformationFactory::loadPlugin(*pluginDataWrapper.getPluginData(), error);
        if (metaInfo && error == PLUGIN_PARSE_ERROR_NONE) {
            if (pluginDataWrapper.isLoadAndLink()) {
                DEBUG_FUNCTION_LINE_INFO("LOAD (ACTIVE)   %s", metaInfo->getName().c_str());

                auto linkInfo = PluginLinkInformationFactory::load(*pluginDataWrapper.getPluginData(), trampolineData, sTrampolineID++);
                if (!linkInfo) {
                    auto errMsg = string_format("Failed to load plugin: %s", pluginDataWrapper.getPluginData()->getSource().c_str());
                    DEBUG_FUNCTION_LINE_ERR("%s", errMsg.c_str());
                    DisplayErrorNotificationMessage(errMsg, 15.0f);
                    continue;
                }
                plugins.emplace_back(std::move(*metaInfo), std::move(*linkInfo), pluginDataWrapper.getPluginData());
            } else {
                DEBUG_FUNCTION_LINE_INFO("LOAD (INACTIVE) %s", metaInfo->getName().c_str());
                plugins.emplace_back(std::move(*metaInfo), PluginLinkInformation::CreateStub(), pluginDataWrapper.getPluginData());
            }
        } else {
            auto errMsg = string_format("Failed to load plugin: %s", *pluginDataWrapper.getPluginData()->getSource().c_str());
            if (error == PLUGIN_PARSE_ERROR_INCOMPATIBLE_VERSION) {
                errMsg += ". Incompatible version.";
            }
            DEBUG_FUNCTION_LINE_ERR("%s", errMsg.c_str());
            DisplayErrorNotificationMessage(errMsg, 15.0f);
        }
    }

    if (!PluginManagement::DoFunctionPatches(plugins)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to patch functions");
        OSFatal("WiiUPluginLoaderBackend: Failed to patch functions");
    }

    return plugins;
}

bool PluginManagement::doRelocation(const std::vector<RelocationData> &relocData,
                                    std::vector<relocation_trampoline_entry_t> &trampData,
                                    const uint32_t trampolineID,
                                    std::map<std::string, OSDynLoad_Module> &usedRPls) {
    for (auto const &cur : relocData) {
        uint32_t functionAddress = 0;
        auto &functionName       = cur.getName();

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
            auto rplName               = cur.getImportRPLInformation().getRPLName();
            int32_t isData             = cur.getImportRPLInformation().isData();
            OSDynLoad_Module rplHandle = nullptr;

            if (!usedRPls.contains(rplName)) {
                DEBUG_FUNCTION_LINE_VERBOSE("Acquire %s", rplName.c_str());
                // Always acquire to increase refcount and make sure it won't get unloaded while we're using it.
                if (const OSDynLoad_Error err = OSDynLoad_Acquire(rplName.c_str(), &rplHandle); err != OS_DYNLOAD_OK) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to acquire %s", rplName.c_str());
                    return false;
                }
                // Keep track RPLs we are using.
                // They will be released on exit
                usedRPls[rplName] = rplHandle;
            } else {
                rplHandle = usedRPls[rplName];
            }

            OSDynLoad_FindExport(rplHandle, static_cast<OSDynLoad_ExportType>(isData), functionName.c_str(), reinterpret_cast<void **>(&functionAddress));
        }

        if (functionAddress == 0) {
            DEBUG_FUNCTION_LINE_ERR("Failed to find export for %s", functionName.c_str());
            return false;
        } else {
            //DEBUG_FUNCTION_LINE("Found export for %s %s", rplName.c_str(), functionName.c_str());
        }

        if (!ElfUtils::elfLinkOne(cur.getType(), cur.getOffset(), cur.getAddend(), reinterpret_cast<uint32_t>(cur.getDestination()), functionAddress, trampData, RELOC_TYPE_IMPORT, trampolineID)) {
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

    DCFlushRange((void *) trampData.data(), trampData.size() * sizeof(relocation_trampoline_entry_t));
    ICInvalidateRange((void *) trampData.data(), trampData.size() * sizeof(relocation_trampoline_entry_t));
    OSMemoryBarrier();
    return true;
}

bool PluginManagement::doRelocations(const std::vector<PluginContainer> &plugins,
                                     std::vector<relocation_trampoline_entry_t> &trampData,
                                     std::map<std::string, OSDynLoad_Module> &usedRPls) {
    for (auto &cur : trampData) {
        if (cur.status == RELOC_TRAMP_IMPORT_DONE) {
            cur.status = RELOC_TRAMP_FREE;
        }
    }

    OSDynLoadAllocFn prevDynLoadAlloc = nullptr;
    OSDynLoadFreeFn prevDynLoadFree   = nullptr;

    OSDynLoad_GetAllocator(&prevDynLoadAlloc, &prevDynLoadFree);
    OSDynLoad_SetAllocator(CustomDynLoadAlloc, CustomDynLoadFree);

    for (const auto &pluginContainer : plugins) {
        if (!pluginContainer.isLinkedAndLoaded()) {
            continue;
        }
        DEBUG_FUNCTION_LINE_VERBOSE("Doing relocations for plugin: %s", pluginContainer.getMetaInformation().getName().c_str());
        if (!PluginManagement::doRelocation(pluginContainer.getPluginLinkInformation().getRelocationDataList(),
                                            trampData,
                                            pluginContainer.getPluginLinkInformation().getTrampolineId(),
                                            usedRPls)) {
            return false;
        }
    }

    OSDynLoad_SetAllocator(prevDynLoadAlloc, prevDynLoadFree);

    return true;
}

bool PluginManagement::RestoreFunctionPatches(std::vector<PluginContainer> &plugins) {
    for (auto &cur : std::ranges::reverse_view(plugins)) {
        for (auto &curFunction : std::ranges::reverse_view(cur.getPluginLinkInformation().getFunctionDataList())) {
            if (!curFunction.RemovePatch()) {
                DEBUG_FUNCTION_LINE_ERR("Failed to remove function patch for: plugin %s", cur.getMetaInformation().getName().c_str());
                return false;
            }
        }
    }
    return true;
}

bool PluginManagement::DoFunctionPatches(std::vector<PluginContainer> &plugins) {
    for (auto &cur : plugins) {
        for (auto &curFunction : cur.getPluginLinkInformation().getFunctionDataList()) {
            if (!curFunction.AddPatch()) {
                DEBUG_FUNCTION_LINE_ERR("Failed to add function patch for: plugin %s", cur.getMetaInformation().getName().c_str());
                return false;
            }
        }
    }
    return true;
}

void PluginManagement::callInitHooks(const std::vector<PluginContainer> &plugins, const std::function<bool(const PluginContainer &)> &pred) {
    CallHook(plugins, WUPS_LOADER_HOOK_INIT_BUTTON_COMBO, pred);
    CallHook(plugins, WUPS_LOADER_HOOK_INIT_CONFIG, pred);
    CallHook(plugins, WUPS_LOADER_HOOK_INIT_STORAGE_DEPRECATED, pred);
    CallHook(plugins, WUPS_LOADER_HOOK_INIT_STORAGE, pred);
    CallHook(plugins, WUPS_LOADER_HOOK_INIT_PLUGIN, pred);
    DEBUG_FUNCTION_LINE_VERBOSE("Done calling init hooks");
}