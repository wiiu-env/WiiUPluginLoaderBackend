#pragma once

#include "PluginData.h"

class PluginDataPersistence {

public:
    static bool save(plugin_data_t *pluginDataStruct, PluginData &plugin);

    static PluginData load(plugin_data_t *pluginDataStruct);
};
