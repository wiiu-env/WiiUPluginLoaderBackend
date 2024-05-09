#include "PluginManagement.h"
#include "coreinit/interrupts.h"
#include "coreinit/scheduler.h"
#include "globals.h"
#include "hooks.h"
#include "patcher/hooks_patcher_static.h"
#include "plugin/PluginDataFactory.h"
#include "utils/utils.h"
#include <coreinit/debug.h>
#include <notifications/notifications.h>
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

    NotificationModuleStatus res;
    if ((res = NotificationModule_InitLibrary()) != NOTIFICATION_MODULE_RESULT_SUCCESS) {
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
    uint32_t upid = OSGetUPID();
    if (upid != 2 && upid != 15) {
        return;
    }
    CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_APPLICATION_REQUESTS_EXIT);
}

WUMS_APPLICATION_ENDS() {
    uint32_t upid = OSGetUPID();
    if (upid != 2 && upid != 15) {
        return;
    }

    CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_APPLICATION_ENDS);
    CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_FINI_WUT_SOCKETS);
    CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB);

    for (const auto &pair : gUsedRPLs) {
        OSDynLoad_Release(pair.second);
    }
    gUsedRPLs.clear();

    deinitLogging();
}

void CheckCleanupCallbackUsage(const std::vector<PluginContainer> &plugins);

WUMS_APPLICATION_STARTS() {
    uint32_t upid = OSGetUPID();
    if (upid != 2 && upid != 15) {
        return;
    }

    OSReport("Running WiiUPluginLoaderBackend " VERSION_FULL "\n");
    gStoredTVBuffer = {};

    gUsedRPLs.clear();

    // If an allocated rpl was not released properly (e.g. if something else calls OSDynload_Acquire without releasing it) memory get leaked.
    // Let's clean this up!
    for (const auto &addr : gAllocatedAddresses) {
        DEBUG_FUNCTION_LINE_WARN("Memory allocated by OSDynload was not freed properly, let's clean it up! (%08X)", addr);
        free((void *) addr);
    }
    gAllocatedAddresses.clear();

    initLogging();
    bool initNeeded = false;

    std::lock_guard<std::mutex> lock(gLoadedDataMutex);

    if (gTrampData.empty()) {
        gTrampData = std::vector<relocation_trampoline_entry_t>(TRAMP_DATA_SIZE);
        for (auto &cur : gTrampData) {
            cur.status = RELOC_TRAMP_FREE;
        }
    }

    if (gLoadedPlugins.empty()) {
        auto pluginPath = getPluginPath();

        DEBUG_FUNCTION_LINE("Load plugins from %s", pluginPath.c_str());

        auto pluginData = PluginDataFactory::loadDir(pluginPath);
        gLoadedPlugins  = PluginManagement::loadPlugins(pluginData, gTrampData);

        initNeeded = true;
    }

    if (!gLoadOnNextLaunch.empty()) {
        auto *currentThread        = OSGetCurrentThread();
        auto saved_reent           = currentThread->reserved[4];
        auto saved_cleanupCallback = currentThread->cleanupCallback;

        currentThread->reserved[4] = 0;

        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_DEINIT_PLUGIN);

        CheckCleanupCallbackUsage(gLoadedPlugins);

        if (currentThread->cleanupCallback != saved_cleanupCallback) {
            DEBUG_FUNCTION_LINE_WARN("WUPS_LOADER_HOOK_DEINIT_PLUGIN overwrote the ThreadCleanupCallback, we need to restore it!\n");
            OSSetThreadCleanupCallback(OSGetCurrentThread(), saved_cleanupCallback);
        }

        currentThread->reserved[4] = saved_reent;

        DEBUG_FUNCTION_LINE("Restore function patches of currently loaded plugins.");
        PluginManagement::RestoreFunctionPatches(gLoadedPlugins);

        for (auto &plugin : gLoadedPlugins) {
            WUPSStorageError err = plugin.CloseStorage();
            if (err != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to close storage for plugin: %s", plugin.getMetaInformation().getName().c_str());
            }
        }

        DEBUG_FUNCTION_LINE("Unload existing plugins.");
        gLoadedPlugins.clear();
        for (auto &cur : gTrampData) {
            cur.status = RELOC_TRAMP_FREE;
        }

        DEBUG_FUNCTION_LINE("Load new plugins");
        gLoadedPlugins = PluginManagement::loadPlugins(gLoadOnNextLaunch, gTrampData);
        initNeeded     = true;
    }

    DEBUG_FUNCTION_LINE("Clear plugin data lists.");
    gLoadOnNextLaunch.clear();
    gLoadedData.clear();

    if (!gLoadedPlugins.empty()) {
        if (!PluginManagement::doRelocations(gLoadedPlugins, gTrampData, gUsedRPLs)) {
            DEBUG_FUNCTION_LINE_ERR("Relocations failed");
            OSFatal("WiiUPluginLoaderBackend: Relocations failed.\n See crash logs for more information.");
        }
        // PluginManagement::memsetBSS(plugins);

        if (initNeeded) {
            CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WUT_MALLOC);
            CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WUT_NEWLIB);
            CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WUT_STDCPP);
        }
        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB);
        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WUT_SOCKETS);

        if (initNeeded) {
            CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_INIT_WRAPPER);
        }

        if (initNeeded) {
            for (auto &plugin : gLoadedPlugins) {
                WUPSStorageError err = plugin.OpenStorage();
                if (err != WUPS_STORAGE_ERROR_SUCCESS) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to open storage for plugin: %s. (%s)", plugin.getMetaInformation().getName().c_str(), WUPSStorageAPI_GetStatusStr(err));
                }
            }
            PluginManagement::callInitHooks(gLoadedPlugins);
        }

        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_APPLICATION_STARTS);
    }
}

void CheckCleanupCallbackUsage(const std::vector<PluginContainer> &plugins) {
    auto *curThread = OSGetCurrentThread();
    for (const auto &cur : plugins) {
        auto textSection = cur.getPluginInformation().getSectionInfo(".text");
        if (!textSection) {
            continue;
        }
        uint32_t startAddress = textSection->getAddress();
        uint32_t endAddress   = textSection->getAddress() + textSection->getSize();
        auto *pluginName      = cur.getMetaInformation().getName().c_str();
        {
            __OSLockScheduler(curThread);
            int state   = OSDisableInterrupts();
            OSThread *t = *((OSThread **) 0x100567F8);
            while (t) {
                auto address = reinterpret_cast<uint32_t>(t->cleanupCallback);
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
