#pragma once

#include "plugin/PluginContainer.h"
#include <coreinit/dynload.h>
#include <map>
#include <memory>
#include <set>
#include <wums/defines/relocation_defines.h>

class PluginManagement {
public:
    static std::vector<std::unique_ptr<PluginContainer>> loadPlugins(
            const std::set<std::shared_ptr<PluginData>> &pluginDataList,
            std::vector<relocation_trampoline_entry_t> &trampolineData);

    static void callInitHooks(const std::vector<std::unique_ptr<PluginContainer>> &plugins);

    static bool doRelocations(const std::vector<std::unique_ptr<PluginContainer>> &plugins,
                              std::vector<relocation_trampoline_entry_t> &trampData,
                              std::map<std::string, OSDynLoad_Module> &usedRPls);

    static bool doRelocation(const std::vector<std::unique_ptr<RelocationData>> &relocData,
                             std::vector<relocation_trampoline_entry_t> &trampData,
                             uint32_t trampolineID,
                             std::map<std::string, OSDynLoad_Module> &usedRPls);

    static bool DoFunctionPatches(const std::vector<std::unique_ptr<PluginContainer>> &plugins);

    static bool RestoreFunctionPatches(const std::vector<std::unique_ptr<PluginContainer>> &plugins);
};