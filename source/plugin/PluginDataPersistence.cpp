#include <common/plugin_defines.h>
#include "PluginDataPersistence.h"
#include "PluginData.h"

bool PluginDataPersistence::save(plugin_data_t *pluginDataStruct, PluginData &plugin) {
    if (pluginDataStruct == NULL) {
        return false;
    }
    pluginDataStruct->buffer = (char *) plugin.buffer;
    pluginDataStruct->bufferLength = plugin.length;
    pluginDataStruct->memoryType = plugin.memoryType;
    pluginDataStruct->heapHandle = (int) plugin.heapHandle;
    return true;
}

PluginData PluginDataPersistence::load(plugin_data_t *pluginDataStruct) {
    PluginData pluginData;

    pluginData.buffer = pluginDataStruct->buffer;
    pluginData.length = pluginDataStruct->bufferLength;
    pluginData.memoryType = (eMemoryTypes) pluginDataStruct->memoryType;
    pluginData.heapHandle = (MEMHeapHandle) pluginDataStruct->heapHandle;
    pluginData.loadReader();
    return pluginData;
}