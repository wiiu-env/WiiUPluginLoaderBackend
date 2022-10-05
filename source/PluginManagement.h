#pragma once

#include "plugin/PluginContainer.h"
#include <coreinit/dynload.h>
#include <forward_list>
#include <map>
#include <memory>
#include <wums/defines/relocation_defines.h>

class PluginManagement {
public:
    static std::vector<std::unique_ptr<PluginContainer>> loadPlugins(const std::forward_list<std::shared_ptr<PluginData>> &pluginList, relocation_trampoline_entry_t *trampoline_data, uint32_t trampoline_data_length);

    static void callInitHooks(const std::vector<std::unique_ptr<PluginContainer>> &plugins);

    static bool doRelocations(const std::vector<std::unique_ptr<PluginContainer>> &plugins,
                              relocation_trampoline_entry_t *trampData,
                              uint32_t tramp_size,
                              std::map<std::string, OSDynLoad_Module> &usedRPls);

    static bool doRelocation(const std::vector<std::unique_ptr<RelocationData>> &relocData,
                             relocation_trampoline_entry_t *tramp_data,
                             uint32_t tramp_length,
                             uint32_t trampolineID,
                             std::map<std::string, OSDynLoad_Module> &usedRPls);

    static bool DoFunctionPatches(const std::vector<std::unique_ptr<PluginContainer>> &plugins);

    static bool RestoreFunctionPatches(const std::vector<std::unique_ptr<PluginContainer>> &plugins);
};