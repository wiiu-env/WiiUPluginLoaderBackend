#pragma once

#include "../common/plugin_defines.h"
#include "PluginData.h"
#include <memory>

class PluginDataPersistence {

public:
    static bool save(plugin_data_t *pluginDataStruct, const std::shared_ptr<PluginData> &plugin);

    static bool save(plugin_data_t *pluginDataStruct, PluginData *plugin);

    static std::shared_ptr<PluginData> load(plugin_data_t *pluginDataStruct);
};
