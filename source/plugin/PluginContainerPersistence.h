#pragma once

#include "../common/plugin_defines.h"
#include "PluginContainer.h"

class PluginContainerPersistence {
public:
    static bool savePlugin(plugin_information_t *pluginInformation, PluginContainer &plugin);

    static std::vector<PluginContainer> loadPlugins(plugin_information_t *pluginInformation);
};
