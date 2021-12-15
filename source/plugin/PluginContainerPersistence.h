#pragma once

#include "../common/plugin_defines.h"
#include "PluginContainer.h"

class PluginContainerPersistence {
public:
    static bool savePlugin(plugin_information_t *pluginInformation, const std::shared_ptr<PluginContainer> &plugin, MEMHeapHandle heapHandle);

    static std::vector<std::shared_ptr<PluginContainer>> loadPlugins(plugin_information_t *pluginInformation);
};
