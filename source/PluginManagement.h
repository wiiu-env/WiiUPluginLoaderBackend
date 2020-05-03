#pragma once


#include <common/plugin_defines.h>
#include <vector>

class PluginManagement {
public:

    static void Init(std::vector<PluginContainer> pInformation);

    static void doRelocations(std::vector<PluginContainer> plugins, relocation_trampolin_entry_t *trampData, uint32_t tramp_size);
    static void memsetBSS(std::vector<PluginContainer> plugins);

    static void callInitHooks(plugin_information_t *pluginInformation);

    static void PatchFunctionsAndCallHooks(plugin_information_t* gPluginInformation);

    static bool doRelocation(const std::vector<RelocationData> &relocData, relocation_trampolin_entry_t *tramp_data, uint32_t tramp_length, uint32_t trampolinID);
};