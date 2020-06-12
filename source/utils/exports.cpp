#include <coreinit/cache.h>
#include <plugin/PluginMetaInformationFactory.h>
#include <plugin/PluginContainer.h>
#include <plugin/PluginContainerPersistence.h>
#include <plugin/PluginDataFactory.h>
#include <PluginManagement.h>
#include <globals.h>
#include <plugin/PluginDataPersistence.h>
#include "exports.h"
#include <wums.h>

void fillPluginInformation(plugin_information *out, PluginMetaInformation *metaInformation) {
    strncpy(out->author, metaInformation->getAuthor().c_str(), 255);
    strncpy(out->buildTimestamp, metaInformation->getBuildTimestamp().c_str(), 255);
    strncpy(out->description, metaInformation->getDescription().c_str(), 255);
    strncpy(out->name, metaInformation->getName().c_str(), 255);
    strncpy(out->license, metaInformation->getLicense().c_str(), 255);
    strncpy(out->version, metaInformation->getVersion().c_str(), 255);
    out->size = metaInformation->getSize();
}

int32_t WUPSLoadAndLinkByDataHandle(const plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size) {
    gLinkOnReload.number_used_plugins = 0;
    if (plugin_data_handle_list != NULL && plugin_data_handle_list_size != 0) {
        for (uint32_t i = 0; i < plugin_data_handle_list_size; i++) {
            auto handle = plugin_data_handle_list[i];
            PluginData *pluginData = (PluginData *) handle;
            DEBUG_FUNCTION_LINE("Saving plugin data %08X", pluginData);
            PluginDataPersistence::save(&gLinkOnReload.plugin_data[gLinkOnReload.number_used_plugins], *pluginData);

            gLinkOnReload.number_used_plugins++;
        }
        if (gLinkOnReload.number_used_plugins > 0) {
            gLinkOnReload.loadOnReload = true;
        }
        DCFlushRange(&gLinkOnReload, sizeof(gLinkOnReload));
    } else {
        return ERROR_INVALID_ARG;
    }
    return ERROR_NONE;
}

int32_t WUPSDeletePluginContainer(const plugin_container_handle *handle_list, uint32_t handle_list_size) {
    if (handle_list != NULL && handle_list_size != 0) {
        for (uint32_t i = 0; i < handle_list_size; i++) {
            auto handle = handle_list[i];
            PluginContainer *pluginContainer = (PluginContainer *) handle;
            DEBUG_FUNCTION_LINE("Delete plugin container: %08X", pluginContainer);
            delete pluginContainer;
        }
    }
    return ERROR_NONE;
}

int32_t WUPSDeletePluginData(const plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size) {
    if (plugin_data_handle_list != NULL && plugin_data_handle_list_size != 0) {
        for (uint32_t i = 0; i < plugin_data_handle_list_size; i++) {
            auto handle = plugin_data_handle_list[i];
            PluginData *pluginData = (PluginData *) handle;
            DEBUG_FUNCTION_LINE("delete plugin data %08X", pluginData);
            delete pluginData;
        }
    }
    return ERROR_NONE;
}

int32_t WUPSLoadPluginAsData(GetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, plugin_data_handle *out) {
    std::optional<PluginData> pluginData;
    if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_PATH) {
        pluginData = PluginDataFactory::load(path, pluginDataHeap);
    } else if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_BUFFER) {
        std::vector<uint8_t> data(size);
        memcpy(&data[0], buffer, size);
        pluginData = PluginDataFactory::load(data, pluginDataHeap);
    } else {
        return ERROR_INVALID_ARG;
    }

    if (!pluginData) {
        DEBUG_FUNCTION_LINE("Failed to alloc plugin data");
        return ERROR_FAILED_ALLOC;
    }

    if (out == NULL) {
        DEBUG_FUNCTION_LINE("out was NULL");
        return ERROR_INVALID_ARG;
    } else {
        PluginData *pluginDataHeap = new PluginData(pluginData.value());
        DEBUG_FUNCTION_LINE("Saving handle %08X", pluginDataHeap);
        *out = (uint32_t) pluginDataHeap;
    }

    return ERROR_NONE;
}

int32_t WUPSLoadPluginAsDataByPath(plugin_data_handle *output, const char *path) {
    return WUPSLoadPluginAsData(PLUGIN_INFORMATION_INPUT_TYPE_PATH, path, NULL, 0, output);
}

int32_t WUPSLoadPluginAsDataByBuffer(plugin_data_handle *output, char *buffer, size_t size) {
    return WUPSLoadPluginAsData(PLUGIN_INFORMATION_INPUT_TYPE_BUFFER, NULL, buffer, size, output);
}

int32_t WUPSGetPluginMetaInformation(GetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, plugin_information *output) {
    std::optional<PluginMetaInformation> pluginInfo;
    if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_PATH && path != NULL) {
        DEBUG_FUNCTION_LINE("PLUGIN_INFORMATION_INPUT_TYPE_PATH %s", path);
        pluginInfo = PluginMetaInformationFactory::loadPlugin(path);
    } else if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_BUFFER && buffer != NULL && size > 0) {
        DEBUG_FUNCTION_LINE("PLUGIN_INFORMATION_INPUT_TYPE_BUFFER %08X %d", buffer, size);
        pluginInfo = PluginMetaInformationFactory::loadPlugin(buffer, size);
    } else {
        return ERROR_INVALID_ARG;
    }

    if (!pluginInfo) {
        DEBUG_FUNCTION_LINE("Failed to load plugin");
        return ERROR_FILE_NOT_FOUND;
    }

    DEBUG_FUNCTION_LINE("loaded plugin");

    if (output == NULL) {
        return ERROR_INVALID_ARG;
    } else {
        fillPluginInformation(output, &pluginInfo.value());
    }
    return ERROR_NONE;
}

int32_t WUPSGetPluginMetaInformationByPath(plugin_information *output, const char *path) {
    return WUPSGetPluginMetaInformation(PLUGIN_INFORMATION_INPUT_TYPE_PATH, path, NULL, 0, output);
}

int32_t WUPSGetPluginMetaInformationByBuffer(plugin_information *output, char *buffer, size_t size) {
    return WUPSGetPluginMetaInformation(PLUGIN_INFORMATION_INPUT_TYPE_BUFFER, NULL, buffer, size, output);
}

int32_t WUPSGetPluginDataForContainerHandles(const plugin_container_handle *plugin_container_handle_list, plugin_data_handle *plugin_data_list, uint32_t buffer_size) {
    int res = ERROR_NONE;
    if (plugin_container_handle_list != NULL && buffer_size != 0) {
        for (uint32_t i = 0; i < buffer_size; i++) {
            auto handle = plugin_container_handle_list[i];
            PluginContainer *container = (PluginContainer *) handle;
            PluginData *pluginData = new PluginData(container->getPluginData());
            DEBUG_FUNCTION_LINE("Created pluginData [%08X]", pluginData);
            plugin_data_list[i] = (uint32_t) pluginData;
        }
    } else {
        res = ERROR_INVALID_ARG;
    }
    return res;
}

int32_t WUPSGetMetaInformation(plugin_container_handle *plugin_container_handle_list, plugin_information *plugin_information_list, uint32_t buffer_size) {
    int res = ERROR_NONE;
    if (plugin_container_handle_list != NULL && buffer_size != 0) {
        for (uint32_t i = 0; i < buffer_size; i++) {
            auto handle = plugin_container_handle_list[i];
            PluginContainer *container = (PluginContainer *) handle;

            strncpy(plugin_information_list[i].author, container->metaInformation.getAuthor().c_str(), 255);
            strncpy(plugin_information_list[i].buildTimestamp, container->metaInformation.getBuildTimestamp().c_str(), 255);
            strncpy(plugin_information_list[i].description, container->metaInformation.getDescription().c_str(), 255);
            strncpy(plugin_information_list[i].name, container->metaInformation.getName().c_str(), 255);
            strncpy(plugin_information_list[i].license, container->metaInformation.getLicense().c_str(), 255);
            strncpy(plugin_information_list[i].version, container->metaInformation.getVersion().c_str(), 255);
            plugin_information_list[i].size = container->metaInformation.getSize();
        }
    } else {
        res = ERROR_INVALID_ARG;
    }
    return res;
}

int32_t WUPSGetLoadedPlugins(plugin_container_handle *io_handles, uint32_t buffer_size, uint32_t *outSize) {
    auto plugins = PluginContainerPersistence::loadPlugins(gPluginInformation);
    uint32_t counter = 0;
    for (auto &plugin: plugins) {
        if (counter < buffer_size) {
            PluginContainer *container = new PluginContainer(plugin);
            DEBUG_FUNCTION_LINE("Created container [%08X]", container);
            io_handles[counter] = (uint32_t) container;
            counter++;
        } else {
            break;
        }
    }
    if (outSize != NULL) {
        *outSize = counter;
    }
    return 0;
}


WUMS_EXPORT_FUNCTION(WUPSLoadPluginAsDataByPath);
WUMS_EXPORT_FUNCTION(WUPSLoadPluginAsDataByBuffer);
WUMS_EXPORT_FUNCTION(WUPSLoadPluginAsData);
WUMS_EXPORT_FUNCTION(WUPSLoadAndLinkByDataHandle);
WUMS_EXPORT_FUNCTION(WUPSDeletePluginContainer);
WUMS_EXPORT_FUNCTION(WUPSDeletePluginData);
WUMS_EXPORT_FUNCTION(WUPSGetPluginMetaInformation);
WUMS_EXPORT_FUNCTION(WUPSGetPluginMetaInformationByPath);
WUMS_EXPORT_FUNCTION(WUPSGetPluginMetaInformationByBuffer);
WUMS_EXPORT_FUNCTION(WUPSGetMetaInformation);
WUMS_EXPORT_FUNCTION(WUPSGetLoadedPlugins);
WUMS_EXPORT_FUNCTION(WUPSGetPluginDataForContainerHandles);