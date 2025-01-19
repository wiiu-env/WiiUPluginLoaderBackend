#pragma once

#include <wums/defines/relocation_defines.h>

#include <coreinit/dynload.h>

#include <functional>
#include <map>
#include <span>
#include <string>

class RelocationData;
class PluginLoadWrapper;
class PluginContainer;

class PluginManagement {
public:
    static std::vector<PluginContainer> loadPlugins(
            const std::vector<PluginLoadWrapper> &pluginDataList);

    static void callInitHooks(const std::vector<PluginContainer> &plugins, const std::function<bool(const PluginContainer &)> &pred);

    static bool doRelocations(const std::vector<PluginContainer> &plugins,
                              std::map<std::string, OSDynLoad_Module> &usedRPls);

    static bool doRelocation(const std::vector<RelocationData> &relocData,
                             std::span<relocation_trampoline_entry_t> trampData,
                             std::map<std::string, OSDynLoad_Module> &usedRPls);

    static bool DoFunctionPatches(std::vector<PluginContainer> &plugins);

    static bool RestoreFunctionPatches(std::vector<PluginContainer> &plugins);
};