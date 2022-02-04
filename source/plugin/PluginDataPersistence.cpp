#include "PluginDataPersistence.h"
#include "../common/plugin_defines.h"
#include <memory>

bool PluginDataPersistence::save(plugin_data_t *pluginDataStruct, const std::shared_ptr<PluginData> &plugin) {
    if (pluginDataStruct == nullptr) {
        return false;
    }
    pluginDataStruct->buffer       = (char *) plugin->buffer;
    pluginDataStruct->bufferLength = plugin->length;
    pluginDataStruct->memoryType   = plugin->memoryType;
    pluginDataStruct->heapHandle   = (int) plugin->heapHandle;
    return true;
}

bool PluginDataPersistence::save(plugin_data_t *pluginDataStruct, PluginData *plugin) {
    if (pluginDataStruct == nullptr) {
        return false;
    }
    pluginDataStruct->buffer       = (char *) plugin->buffer;
    pluginDataStruct->bufferLength = plugin->length;
    pluginDataStruct->memoryType   = plugin->memoryType;
    pluginDataStruct->heapHandle   = (int) plugin->heapHandle;
    return true;
}

std::shared_ptr<PluginData> PluginDataPersistence::load(plugin_data_t *pluginDataStruct) {
    auto pluginData = std::make_shared<PluginData>();

    pluginData->buffer     = pluginDataStruct->buffer;
    pluginData->length     = pluginDataStruct->bufferLength;
    pluginData->memoryType = (eMemoryTypes) pluginDataStruct->memoryType;
    pluginData->heapHandle = (MEMHeapHandle) pluginDataStruct->heapHandle;
    return pluginData;
}