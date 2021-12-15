#pragma once

#include <common/plugin_defines.h>
#include <vector>

class PluginManagement {
public:

    static void doRelocations(const std::vector<std::shared_ptr<PluginContainer>> &plugins, relocation_trampolin_entry_t *trampData, uint32_t tramp_size);

    static void memsetBSS(const std::vector<std::shared_ptr<PluginContainer>> &plugins);

    static void callInitHooks(plugin_information_t *pluginInformation);

    static void PatchFunctionsAndCallHooks(plugin_information_t *gPluginInformation);

    static bool doRelocation(const std::vector<std::shared_ptr<RelocationData>> &relocData, relocation_trampolin_entry_t *tramp_data, uint32_t tramp_length, uint32_t trampolinID);

    static void unloadPlugins(plugin_information_t *pluginInformation, MEMHeapHandle pluginHeap, BOOL freePluginData);

    static std::vector<std::shared_ptr<PluginContainer>> loadPlugins(const std::vector<std::shared_ptr<PluginData>> &pluginList, MEMHeapHandle pHeader, relocation_trampolin_entry_t *trampolin_data, uint32_t trampolin_data_length);

    static void RestorePatches(plugin_information_t *pluginInformation, BOOL pluginOnly);
};