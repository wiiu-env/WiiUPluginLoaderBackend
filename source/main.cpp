#include "PluginManagement.h"
#include "globals.h"
#include "hooks.h"
#include "patcher/hooks_patcher_static.h"
#include "plugin/PluginDataFactory.h"
#include "utils/utils.h"
#include <coreinit/debug.h>
#include <wums.h>

WUMS_MODULE_EXPORT_NAME("homebrew_wupsbackend");

WUMS_USE_WUT_DEVOPTAB();

WUMS_INITIALIZE() {
    initLogging();
    DEBUG_FUNCTION_LINE("Patching functions");
    for (uint32_t i = 0; i < method_hooks_static_size; i++) {
        if (!FunctionPatcherPatchFunction(&method_hooks_static[i], nullptr)) {
            OSFatal("homebrew_wupsbackend: Failed to patch function");
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

    deinitLogging();
}


WUMS_APPLICATION_STARTS() {
    uint32_t upid = OSGetUPID();
    if (upid != 2 && upid != 15) {
        return;
    }
    initLogging();
    bool initNeeded = false;

    std::lock_guard<std::mutex> lock(gLoadedDataMutex);

    if (gTrampData == nullptr) {
        gTrampData = (relocation_trampoline_entry_t *) memalign(0x4, sizeof(relocation_trampoline_entry_t) * TRAMP_DATA_SIZE);
        if (gTrampData == nullptr) {
            DEBUG_FUNCTION_LINE_ERR("Failed to allocated the memory for the trampoline data");
            OSFatal("Failed to allocated the memory for the trampoline data");
        }
    }

    if (gLoadedPlugins.empty()) {
        auto pluginPath = getPluginPath();

        DEBUG_FUNCTION_LINE("Load plugins from %s", pluginPath.c_str());

        auto pluginData = PluginDataFactory::loadDir(pluginPath);

        gLoadedPlugins = PluginManagement::loadPlugins(pluginData, gTrampData, TRAMP_DATA_SIZE);

        initNeeded = true;
    }

    if (!gLoadOnNextLaunch.empty()) {
        DEBUG_FUNCTION_LINE("Restore function patches of currently loaded plugins.");
        PluginManagement::RestoreFunctionPatches(gLoadedPlugins);
        DEBUG_FUNCTION_LINE("Unload existing plugins.");
        gLoadedPlugins.clear();
        DEBUG_FUNCTION_LINE("Load new plugins");

        gLoadedPlugins = PluginManagement::loadPlugins(gLoadOnNextLaunch, gTrampData, TRAMP_DATA_SIZE);
        initNeeded     = true;
    }

    DEBUG_FUNCTION_LINE("Clear plugin data lists.");
    gLoadOnNextLaunch.clear();
    gLoadedData.clear();

    if (!gLoadedPlugins.empty()) {
        if (!PluginManagement::doRelocations(gLoadedPlugins, gTrampData, TRAMP_DATA_SIZE)) {
            DEBUG_FUNCTION_LINE_ERR("Relocations failed");
            OSFatal("Relocations failed");
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
            PluginManagement::callInitHooks(gLoadedPlugins);
        }

        CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_APPLICATION_STARTS);
    }
}