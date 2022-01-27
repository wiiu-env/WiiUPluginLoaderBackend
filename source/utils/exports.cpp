#include <coreinit/cache.h>
#include "../plugin/PluginMetaInformationFactory.h"
#include "../plugin/PluginContainer.h"
#include "../plugin/PluginContainerPersistence.h"
#include "../plugin/PluginDataFactory.h"
#include "../PluginManagement.h"
#include "../globals.h"
#include "../plugin/PluginDataPersistence.h"
#include "exports.h"
#include <wums.h>

void fillPluginInformation(plugin_information *out, const std::shared_ptr<PluginMetaInformation> &metaInformation) {
    out->plugin_information_version = PLUGIN_INFORMATION_VERSION;
    strncpy(out->author, metaInformation->getAuthor().c_str(), sizeof(out->author) - 1);
    strncpy(out->buildTimestamp, metaInformation->getBuildTimestamp().c_str(), sizeof(out->buildTimestamp) - 1);
    strncpy(out->description, metaInformation->getDescription().c_str(), sizeof(out->description) - 1);
    strncpy(out->name, metaInformation->getName().c_str(), sizeof(out->name) - 1);
    strncpy(out->license, metaInformation->getLicense().c_str(), sizeof(out->license) - 1);
    strncpy(out->version, metaInformation->getVersion().c_str(), sizeof(out->version) - 1);
    strncpy(out->storageId, metaInformation->getStorageId().c_str(), sizeof(out->storageId) - 1);
    out->size = metaInformation->getSize();
}

extern "C" PluginBackendApiErrorType WUPSLoadAndLinkByDataHandle(const plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size) {
    gLinkOnReload.number_used_plugins = 0;
    if (plugin_data_handle_list != nullptr && plugin_data_handle_list_size != 0) {
        for (uint32_t i = 0; i < plugin_data_handle_list_size; i++) {
            plugin_data_handle handle = plugin_data_handle_list[i];
            auto *pluginData = (PluginData *) handle;
            PluginDataPersistence::save(&gLinkOnReload.plugin_data[gLinkOnReload.number_used_plugins], pluginData);

            gLinkOnReload.number_used_plugins++;
        }
        if (gLinkOnReload.number_used_plugins > 0) {
            gLinkOnReload.loadOnReload = true;
        }
        DCFlushRange(&gLinkOnReload, sizeof(gLinkOnReload));
    } else {
        DEBUG_FUNCTION_LINE("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSDeletePluginContainer(const plugin_container_handle *handle_list, uint32_t handle_list_size) {
    if (handle_list != nullptr && handle_list_size != 0) {
        for (uint32_t i = 0; i < handle_list_size; i++) {
            auto handle = handle_list[i];
            auto *pluginContainer = (PluginContainer *) handle;
            delete pluginContainer;
        }
    }
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSDeletePluginData(const plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size) {
    if (plugin_data_handle_list != nullptr && plugin_data_handle_list_size != 0) {
        for (uint32_t i = 0; i < plugin_data_handle_list_size; i++) {
            auto handle = plugin_data_handle_list[i];
            auto *pluginData = (PluginData *) handle;
            delete pluginData;
        }
    }
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSLoadPluginAsData(GetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, plugin_data_handle *out) {
    std::optional<std::shared_ptr<PluginData>> pluginData;
    if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_PATH && path != nullptr) {
        pluginData = PluginDataFactory::load(path, gPluginDataHeap);
    } else if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_BUFFER && buffer != nullptr && size > 0) {
        std::vector<uint8_t> data(size);
        memcpy(&data[0], buffer, size);
        pluginData = PluginDataFactory::load(data, gPluginDataHeap);
    } else {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    if (!pluginData) {
        DEBUG_FUNCTION_LINE("PLUGIN_BACKEND_API_ERROR_FAILED_ALLOC");
        return PLUGIN_BACKEND_API_ERROR_FAILED_ALLOC;
    }

    if (out == nullptr) {
        DEBUG_FUNCTION_LINE("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    } else {
        auto *pluginDataHandle = new PluginData(*pluginData.value());
        *out = (uint32_t) pluginDataHandle;
    }

    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSLoadPluginAsDataByPath(plugin_data_handle *output, const char *path) {
    return WUPSLoadPluginAsData(PLUGIN_INFORMATION_INPUT_TYPE_PATH, path, nullptr, 0, output);
}

extern "C" PluginBackendApiErrorType WUPSLoadPluginAsDataByBuffer(plugin_data_handle *output, char *buffer, size_t size) {
    return WUPSLoadPluginAsData(PLUGIN_INFORMATION_INPUT_TYPE_BUFFER, nullptr, buffer, size, output);
}

extern "C" PluginBackendApiErrorType WUPSGetPluginMetaInformation(GetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, plugin_information *output) {
    std::optional<std::shared_ptr<PluginMetaInformation>> pluginInfo;
    if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_PATH && path != nullptr) {
        std::string pathStr(path);
        pluginInfo = PluginMetaInformationFactory::loadPlugin(pathStr);
    } else if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_BUFFER && buffer != nullptr && size > 0) {
        pluginInfo = PluginMetaInformationFactory::loadPlugin(buffer, size);
    } else {
        DEBUG_FUNCTION_LINE("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    if (!pluginInfo) {
        DEBUG_FUNCTION_LINE("PLUGIN_BACKEND_API_ERROR_FILE_NOT_FOUND");
        return PLUGIN_BACKEND_API_ERROR_FILE_NOT_FOUND;
    }

    if (output == nullptr) {
        DEBUG_FUNCTION_LINE("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    } else {
        fillPluginInformation(output, pluginInfo.value());
    }
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSGetPluginMetaInformationByPath(plugin_information *output, const char *path) {
    return WUPSGetPluginMetaInformation(PLUGIN_INFORMATION_INPUT_TYPE_PATH, path, nullptr, 0, output);
}

extern "C" PluginBackendApiErrorType WUPSGetPluginMetaInformationByBuffer(plugin_information *output, char *buffer, size_t size) {
    return WUPSGetPluginMetaInformation(PLUGIN_INFORMATION_INPUT_TYPE_BUFFER, nullptr, buffer, size, output);
}

extern "C" PluginBackendApiErrorType WUPSGetPluginDataForContainerHandles(const plugin_container_handle *plugin_container_handle_list, plugin_data_handle *plugin_data_list, uint32_t buffer_size) {
    PluginBackendApiErrorType res = PLUGIN_BACKEND_API_ERROR_NONE;
    if (plugin_container_handle_list != nullptr && buffer_size != 0) {
        for (uint32_t i = 0; i < buffer_size; i++) {
            auto handle = plugin_container_handle_list[i];
            auto *container = (PluginContainer *) handle;
            auto *pluginData = new PluginData(*container->getPluginData());
            plugin_data_list[i] = (uint32_t) pluginData;
        }
    } else {
        DEBUG_FUNCTION_LINE("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
        res = PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    return res;
}

extern "C" PluginBackendApiErrorType WUPSGetMetaInformation(const plugin_container_handle *plugin_container_handle_list, plugin_information *plugin_information_list, uint32_t buffer_size) {
    PluginBackendApiErrorType res = PLUGIN_BACKEND_API_ERROR_NONE;
    if (plugin_container_handle_list != nullptr && buffer_size != 0) {
        for (uint32_t i = 0; i < buffer_size; i++) {
            auto handle = plugin_container_handle_list[i];
            auto *container = (PluginContainer *) handle;

            plugin_information_list[i].plugin_information_version = PLUGIN_INFORMATION_VERSION;
            strncpy(plugin_information_list[i].storageId, container->metaInformation->getStorageId().c_str(), sizeof(plugin_information_list[i].storageId) - 1);
            strncpy(plugin_information_list[i].author, container->metaInformation->getAuthor().c_str(), sizeof(plugin_information_list[i].author) - 1);
            strncpy(plugin_information_list[i].buildTimestamp, container->metaInformation->getBuildTimestamp().c_str(), sizeof(plugin_information_list[i].buildTimestamp) - 1);
            strncpy(plugin_information_list[i].description, container->metaInformation->getDescription().c_str(), sizeof(plugin_information_list[i].description) - 1);
            strncpy(plugin_information_list[i].name, container->metaInformation->getName().c_str(), sizeof(plugin_information_list[i].name) - 1);
            strncpy(plugin_information_list[i].license, container->metaInformation->getLicense().c_str(), sizeof(plugin_information_list[i].license) - 1);
            strncpy(plugin_information_list[i].version, container->metaInformation->getVersion().c_str(), sizeof(plugin_information_list[i].version) - 1);
            plugin_information_list[i].size = container->metaInformation->getSize();
        }
    } else {
        DEBUG_FUNCTION_LINE("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
        res = PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    return res;
}

extern "C" PluginBackendApiErrorType WUPSGetLoadedPlugins(plugin_container_handle *io_handles, uint32_t buffer_size, uint32_t *outSize, uint32_t *plugin_information_version) {
    DEBUG_FUNCTION_LINE();
    if (plugin_information_version == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    *plugin_information_version = PLUGIN_INFORMATION_VERSION;
    auto plugins = PluginContainerPersistence::loadPlugins(gPluginInformation);
    uint32_t counter = 0;
    for (auto &plugin: plugins) {
        if (counter < buffer_size) {
            auto *container = new PluginContainer(*plugin);
            io_handles[counter] = (uint32_t) container;
            counter++;
        } else {
            break;
        }
    }
    if (outSize != nullptr) {
        *outSize = counter;
    }
    return PLUGIN_BACKEND_API_ERROR_NONE;
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