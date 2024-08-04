#include "PluginManagement.h"
#include "coreinit/interrupts.h"
#include "coreinit/scheduler.h"
#include "globals.h"
#include "hooks.h"
#include "patcher/hooks_patcher_static.h"
#include "plugin/PluginDataFactory.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "version.h"

#include <coreinit/debug.h>
#include <notifications/notifications.h>
#include <ranges>
#include <wums.h>

WUMS_MODULE_EXPORT_NAME("homebrew_wupsbackend");
WUMS_USE_WUT_DEVOPTAB();
WUMS_DEPENDS_ON(homebrew_functionpatcher);
WUMS_DEPENDS_ON(homebrew_memorymapping);
WUMS_DEPENDS_ON(homebrew_notifications);

WUMS_INITIALIZE() {
    initLogging();

    if (FunctionPatcher_InitLibrary() != FUNCTION_PATCHER_RESULT_SUCCESS) {
        OSFatal("homebrew_wupsbackend: FunctionPatcher_InitLibrary failed");
    }

    if (const NotificationModuleStatus res = NotificationModule_InitLibrary(); res != NOTIFICATION_MODULE_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init NotificationModule: %s (%d)", NotificationModule_GetStatusStr(res), res);
        gNotificationModuleLoaded = false;
    } else {
        gNotificationModuleLoaded = true;
    }

    DEBUG_FUNCTION_LINE("Patching functions");
    for (uint32_t i = 0; i < method_hooks_static_size; i++) {
        if (FunctionPatcher_AddFunctionPatch(&method_hooks_static[i], nullptr, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
            OSFatal("homebrew_wupsbackend: Failed to AddPatch function");
        }
    }

    deinitLogging();
}

WUMS_APPLICATION_REQUESTS_EXIT() {
    const uint32_t upid = OSGetUPID();
    if (upid != 2 && upid != 15) {
        return;
    }
    CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_APPLICATION_REQUESTS_EXIT);
}

WUMS_APPLICATION_ENDS() {
    const uint32_t upid = OSGetUPID();
    if (upid != 2 && upid != 15) {
        return;
    }

    CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_APPLICATION_ENDS);
    CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_FINI_WUT_SOCKETS);
    CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB);

    for (const auto &val : gUsedRPLs | std::views::values) {
        OSDynLoad_Release(val);
    }
    gUsedRPLs.clear();

    deinitLogging();
}

void CheckCleanupCallbackUsage(const std::vector<PluginContainer> &plugins);
void CleanupPlugins(std::vector<PluginContainer> &&pluginsToDeinit);


WUMS_APPLICATION_STARTS() {
    const uint32_t upid = OSGetUPID();
    if (upid != 2 && upid != 15) {
        return;
    }

    OSReport("Running WiiUPluginLoaderBackend " MODULE_VERSION_FULL "\n");
    gStoredTVBuffer = {};

    gUsedRPLs.clear();

    // If an allocated rpl was not released properly (e.g. if something else calls OSDynload_Acquire without releasing it) memory get leaked.
    // Let's clean this up!
    for (const auto &addr : gAllocatedAddresses) {
        DEBUG_FUNCTION_LINE_WARN("Memory allocated by OSDynload was not freed properly, let's clean it up! (%08X)", addr);
        free(addr);
    }
    gAllocatedAddresses.clear();

    initLogging();

    std::lock_guard lock(gLoadedDataMutex);

    if (gTrampData.empty()) {
        gTrampData = std::vector<relocation_trampoline_entry_t>(TRAMP_DATA_SIZE);
        for (auto &cur : gTrampData) {
            cur.status = RELOC_TRAMP_FREE;
        }
    }

    std::vector<PluginContainer> newLoadedPlugins;

    if (gLoadedPlugins.empty()) {
        const auto pluginPath = getPluginPath();

        DEBUG_FUNCTION_LINE("Load plugins from %s", pluginPath.c_str());

        const auto pluginData = PluginDataFactory::loadDir(pluginPath);
        newLoadedPlugins      = PluginManagement::loadPlugins(pluginData, gTrampData);
    }

    if (!gLoadOnNextLaunch.empty()) {
        std::vector<PluginContainer> pluginsToKeep;
        std::set<std::shared_ptr<PluginData>, PluginDataSharedPtrComparator> toBeLoaded;

        // Check which plugins are already loaded and which needs to be
        for (const auto &pluginData : gLoadOnNextLaunch) {
            // Check if the plugin data is already loaded
            if (auto it = std::ranges::find_if(gLoadedPlugins,
                                               [&pluginData](const PluginContainer &container) {
                                                   return container.getPluginDataCopy()->getHandle() == pluginData->getHandle();
                                               });
                it != gLoadedPlugins.end()) {
                pluginsToKeep.push_back(std::move(*it));
                gLoadedPlugins.erase(it);
            } else {
                // Load it if it's not already loaded
                toBeLoaded.insert(pluginData);
            }
        }

        std::vector<PluginContainer> pluginsToDeinit = std::move(gLoadedPlugins);
        gLoadedPlugins                               = std::move(pluginsToKeep);

        DEBUG_FUNCTION_LINE("Deinit unused plugins");
        CleanupPlugins(std::move(pluginsToDeinit));

        DEBUG_FUNCTION_LINE("Load new plugins");
        newLoadedPlugins = PluginManagement::loadPlugins(toBeLoaded, gTrampData);
    }

    DEBUG_FUNCTION_LINE("Clear plugin data lists.");
    gLoadOnNextLaunch.clear();
    gLoadedData.clear();

    if (!gLoadedPlugins.empty() || !newLoadedPlugins.empty()) {
        for (auto &pluginContainer : newLoadedPlugins) {
            pluginContainer.setInitDone(false);
        }

        // Move all new plugin containers into gLoadedPlugins
        append_move_all_values(gLoadedPlugins, newLoadedPlugins);

        if (!PluginManagement::doRelocations(gLoadedPlugins, gTrampData, gUsedRPLs)) {
            DEBUG_FUNCTION_LINE_ERR("Relocations failed");
            OSFatal("WiiUPluginLoaderBackend: Relocations failed.\n See crash logs for more information.");
        }
        // PluginManagement::memsetBSS(plugins);

        const auto &needsInitsCheck = [](const PluginContainer &container) { return !container.isInitDone(); };
        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WUT_MALLOC, needsInitsCheck);
        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WUT_NEWLIB, needsInitsCheck);
        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WUT_STDCPP, needsInitsCheck);

        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB);
        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WUT_SOCKETS);

        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WRAPPER, needsInitsCheck);

        for (auto &plugin : gLoadedPlugins) {
            if (plugin.isInitDone()) { continue; }
            if (const WUPSStorageError err = plugin.OpenStorage(); err != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to open storage for plugin: %s. (%s)", plugin.getMetaInformation().getName().c_str(), WUPSStorageAPI_GetStatusStr(err));
            }
        }
        PluginManagement::callInitHooks(gLoadedPlugins, needsInitsCheck);

        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_APPLICATION_STARTS);
        for (auto &pluginContainer : gLoadedPlugins) {
            pluginContainer.setInitDone(true);
        }
    }
}

void CleanupPlugins(std::vector<PluginContainer> &&pluginsToDeinit) {
    auto *currentThread              = OSGetCurrentThread();
    const auto saved_reent           = currentThread->reserved[4];
    const auto saved_cleanupCallback = currentThread->cleanupCallback;

    currentThread->reserved[4] = 0;

    CallHook(pluginsToDeinit, WUPS_LOADER_HOOK_DEINIT_PLUGIN);

    CheckCleanupCallbackUsage(pluginsToDeinit);

    if (currentThread->cleanupCallback != saved_cleanupCallback) {
        DEBUG_FUNCTION_LINE_WARN("WUPS_LOADER_HOOK_DEINIT_PLUGIN overwrote the ThreadCleanupCallback, we need to restore it!\n");
        OSSetThreadCleanupCallback(OSGetCurrentThread(), saved_cleanupCallback);
    }

    currentThread->reserved[4] = saved_reent;

    DEBUG_FUNCTION_LINE("Restore function patches of plugins.");
    PluginManagement::RestoreFunctionPatches(pluginsToDeinit);

    for (auto &plugin : pluginsToDeinit) {
        if (const WUPSStorageError err = plugin.CloseStorage(); err != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to close storage for plugin: %s", plugin.getMetaInformation().getName().c_str());
        }
    }

    for (const auto &pluginContainer : pluginsToDeinit) {
        for (auto &cur : gTrampData) {
            if (!pluginContainer.isPluginLinkedAndLoaded() || cur.id != pluginContainer.getPluginLinkInformation()->getTrampolineId()) {
                continue;
            }
            cur.status = RELOC_TRAMP_FREE;
        }
    }
}
void CheckCleanupCallbackUsage(const std::vector<PluginContainer> &plugins) {
    auto *curThread = OSGetCurrentThread();
    for (const auto &cur : plugins) {
        if (!cur.isLinkedAndLoaded()) {
            continue;
        }

        const auto textSection = cur.getPluginLinkInformation().getSectionInfo(".text");
        if (!textSection) {
            continue;
        }
        const uint32_t startAddress = textSection->getAddress();
        const uint32_t endAddress   = textSection->getAddress() + textSection->getSize();
        auto *pluginName            = cur.getMetaInformation().getName().c_str();
        {
            __OSLockScheduler(curThread);
            const int state = OSDisableInterrupts();
            OSThread *t     = *reinterpret_cast<OSThread **>(0x100567F8);
            while (t) {
                const auto address = reinterpret_cast<uint32_t>(t->cleanupCallback);
                if (address != 0 && address >= startAddress && address <= endAddress) {
                    OSReport("[WARN] PluginBackend: Thread 0x%08X is using a function from plugin %s for the threadCleanupCallback\n", t, pluginName);
                }
                t = t->activeLink.next;
            }
            OSRestoreInterrupts(state);
            __OSUnlockScheduler(curThread);
        }
    }
}
