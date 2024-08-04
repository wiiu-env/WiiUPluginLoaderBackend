#pragma once

#include "plugin/PluginContainer.h"
#include "plugin/PluginLoadWrapper.h"
#include <coreinit/dynload.h>
#include <functional>
#include <map>
#include <set>
#include <wums/defines/relocation_defines.h>

class PluginManagement {
public:
    static std::vector<PluginContainer> loadPlugins(
            const std::vector<PluginLoadWrapper> &pluginDataList,
            std::vector<relocation_trampoline_entry_t> &trampolineData);

    static void callInitHooks(const std::vector<PluginContainer> &plugins, const std::function<bool(const PluginContainer &)> &pred);

    static bool doRelocations(const std::vector<PluginContainer> &plugins,
                              std::vector<relocation_trampoline_entry_t> &trampData,
                              std::map<std::string, OSDynLoad_Module> &usedRPls);

    static bool doRelocation(const std::vector<RelocationData> &relocData,
                             std::vector<relocation_trampoline_entry_t> &trampData,
                             uint32_t trampolineID,
                             std::map<std::string, OSDynLoad_Module> &usedRPls);

    static bool DoFunctionPatches(std::vector<PluginContainer> &plugins);

    static bool RestoreFunctionPatches(std::vector<PluginContainer> &plugins);
};