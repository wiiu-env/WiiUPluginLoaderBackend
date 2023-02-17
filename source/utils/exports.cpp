#include "exports.h"
#include "../PluginManagement.h"
#include "../globals.h"
#include "../plugin/PluginDataFactory.h"
#include "../plugin/PluginMetaInformationFactory.h"
#include "utils.h"
#include <wums.h>

static void fillPluginInformation(plugin_information *out, const std::unique_ptr<PluginMetaInformation> &metaInformation) {
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
    if (plugin_data_handle_list == nullptr || plugin_data_handle_list_size == 0) {

        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    std::lock_guard<std::mutex> lock(gLoadedDataMutex);
    for (uint32_t i = 0; i < plugin_data_handle_list_size; i++) {
        auto handle = plugin_data_handle_list[i];
        bool found  = false;

        for (auto &pluginData : gLoadedData) {
            if (pluginData->getHandle() == handle) {
                gLoadOnNextLaunch.push_front(pluginData);
                found = true;
                break;
            }
        }
        if (!found) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get plugin data for handle %08X. Skipping it.", handle);
        }
    }

    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSDeletePluginData(const plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size) {
    if (plugin_data_handle_list != nullptr && plugin_data_handle_list_size != 0) {
        for (uint32_t i = 0; i < plugin_data_handle_list_size; i++) {
            auto handle = plugin_data_handle_list[i];

            if (!remove_locked_first_if(gLoadedDataMutex, gLoadedData, [handle](auto &cur) { return cur->getHandle() == handle; })) {
                DEBUG_FUNCTION_LINE_ERR("Failed to delete plugin data by handle %08X", handle);
            }
        }
    }
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSLoadPluginAsData(GetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, plugin_data_handle *out) {
    if (out == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    std::optional<std::unique_ptr<PluginData>> pluginData;
    if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_PATH && path != nullptr) {
        pluginData = PluginDataFactory::load(path);
    } else if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_BUFFER && buffer != nullptr && size > 0) {
        std::vector<uint8_t> data(size);
        memcpy(&data[0], buffer, size);
        pluginData = PluginDataFactory::load(data);
    } else {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    if (!pluginData) {
        DEBUG_FUNCTION_LINE_ERR("PLUGIN_BACKEND_API_ERROR_FAILED_ALLOC");
        return PLUGIN_BACKEND_API_ERROR_FAILED_ALLOC;
    }

    else {
        *out = pluginData.value()->getHandle();
        gLoadedData.push_front(std::move(pluginData.value()));
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
    std::optional<std::unique_ptr<PluginMetaInformation>> pluginInfo;
    if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_PATH && path != nullptr) {
        std::string pathStr(path);
        pluginInfo = PluginMetaInformationFactory::loadPlugin(pathStr);
    } else if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_BUFFER && buffer != nullptr && size > 0) {
        pluginInfo = PluginMetaInformationFactory::loadPlugin(buffer, size);
    } else {
        DEBUG_FUNCTION_LINE_ERR("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    if (!pluginInfo) {
        DEBUG_FUNCTION_LINE_ERR("PLUGIN_BACKEND_API_ERROR_FILE_NOT_FOUND");
        return PLUGIN_BACKEND_API_ERROR_FILE_NOT_FOUND;
    }

    if (output == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
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
    if (plugin_container_handle_list == nullptr || buffer_size == 0) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    std::lock_guard<std::mutex> lock(gLoadedDataMutex);
    for (uint32_t i = 0; i < buffer_size; /* only increase on success*/) {
        auto handle = plugin_container_handle_list[i];
        bool found  = false;
        for (auto &curContainer : gLoadedPlugins) {
            if (curContainer->getHandle() == handle) {
                auto &pluginData    = curContainer->getPluginData();
                plugin_data_list[i] = (uint32_t) pluginData->getHandle();
                gLoadedData.push_front(pluginData);
                found = true;
                i++;
                break;
            }
        }
        if (!found) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get container for handle %08X", handle);
        }
    }

    return res;
}

extern "C" PluginBackendApiErrorType WUPSGetMetaInformation(const plugin_container_handle *plugin_container_handle_list, plugin_information *plugin_information_list, uint32_t buffer_size) {
    PluginBackendApiErrorType res = PLUGIN_BACKEND_API_ERROR_NONE;
    if (plugin_container_handle_list != nullptr && buffer_size != 0) {
        for (uint32_t i = 0; i < buffer_size; i++) {
            auto handle = plugin_container_handle_list[i];
            bool found  = false;
            for (auto &curContainer : gLoadedPlugins) {
                if (curContainer->getHandle() == handle) {
                    auto &metaInfo = curContainer->getMetaInformation();

                    plugin_information_list[i].plugin_information_version = PLUGIN_INFORMATION_VERSION;
                    strncpy(plugin_information_list[i].storageId, metaInfo->getStorageId().c_str(), sizeof(plugin_information_list[i].storageId) - 1);
                    strncpy(plugin_information_list[i].author, metaInfo->getAuthor().c_str(), sizeof(plugin_information_list[i].author) - 1);
                    strncpy(plugin_information_list[i].buildTimestamp, metaInfo->getBuildTimestamp().c_str(), sizeof(plugin_information_list[i].buildTimestamp) - 1);
                    strncpy(plugin_information_list[i].description, metaInfo->getDescription().c_str(), sizeof(plugin_information_list[i].description) - 1);
                    strncpy(plugin_information_list[i].name, metaInfo->getName().c_str(), sizeof(plugin_information_list[i].name) - 1);
                    strncpy(plugin_information_list[i].license, metaInfo->getLicense().c_str(), sizeof(plugin_information_list[i].license) - 1);
                    strncpy(plugin_information_list[i].version, metaInfo->getVersion().c_str(), sizeof(plugin_information_list[i].version) - 1);
                    plugin_information_list[i].size = metaInfo->getSize();
                    found                           = true;

                    break;
                }
            }
            if (!found) {
                DEBUG_FUNCTION_LINE_ERR("FAILED TO FIND CONTAINER FOR HANDLE %08X", handle);
            }
        }
    } else {
        DEBUG_FUNCTION_LINE_ERR("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
        res = PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    return res;
}

extern "C" PluginBackendApiErrorType WUPSGetLoadedPlugins(plugin_container_handle *io_handles, uint32_t buffer_size, uint32_t *outSize, uint32_t *plugin_information_version) {
    if (plugin_information_version == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    *plugin_information_version = PLUGIN_INFORMATION_VERSION;
    auto &plugins               = gLoadedPlugins;
    uint32_t counter            = 0;
    for (auto &plugin : plugins) {
        if (counter < buffer_size) {
            io_handles[counter] = plugin->getHandle();
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
WUMS_EXPORT_FUNCTION(WUPSDeletePluginData);
WUMS_EXPORT_FUNCTION(WUPSGetPluginMetaInformation);
WUMS_EXPORT_FUNCTION(WUPSGetPluginMetaInformationByPath);
WUMS_EXPORT_FUNCTION(WUPSGetPluginMetaInformationByBuffer);
WUMS_EXPORT_FUNCTION(WUPSGetMetaInformation);
WUMS_EXPORT_FUNCTION(WUPSGetLoadedPlugins);
WUMS_EXPORT_FUNCTION(WUPSGetPluginDataForContainerHandles);

// API 2.0
extern "C" PluginBackendApiErrorType WUPSGetAPIVersion(WUPSBackendAPIVersion *outVersion) {
    if (outVersion == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    *outVersion = 2;
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSGetNumberOfLoadedPlugins(uint32_t *outCount) {
    if (outCount == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    *outCount = gLoadedPlugins.size();
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSGetSectionInformationForPlugin(const plugin_container_handle handle, plugin_section_info *plugin_section_list, uint32_t buffer_size, uint32_t *out_count) {
    PluginBackendApiErrorType res = PLUGIN_BACKEND_API_ERROR_NONE;
    if (out_count != nullptr) {
        *out_count = 0;
    }
    if (handle != 0 && plugin_section_list != nullptr && buffer_size != 0) {
        bool found = false;
        for (auto &curContainer : gLoadedPlugins) {
            if (curContainer->getHandle() == handle) {
                found                 = true;
                auto &sectionInfoList = curContainer->getPluginInformation()->getSectionInfoList();

                uint32_t offset = 0;
                for (auto const &[key, sectionInfo] : sectionInfoList) {
                    if (offset >= buffer_size) {
                        break;
                    }
                    plugin_section_list[offset].plugin_section_info_version = PLUGIN_SECTION_INFORMATION_VERSION;
                    strncpy(plugin_section_list[offset].name, sectionInfo->getName().c_str(), sizeof(plugin_section_list[offset].name) - 1);
                    plugin_section_list[offset].address = (void *) sectionInfo->getAddress();
                    plugin_section_list[offset].size    = sectionInfo->getSize();
                    offset++;
                }
                if (out_count != nullptr) {
                    *out_count = offset;
                }
                break;
            }
        }
        if (!found) {
            res = PLUGIN_BACKEND_API_INVALID_HANDLE;
        }
    } else {
        res = PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    return res;
}

extern "C" PluginBackendApiErrorType WUPSWillReloadPluginsOnNextLaunch(bool *out) {
    if (out == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    std::lock_guard<std::mutex> lock(gLoadedDataMutex);
    *out = !gLoadOnNextLaunch.empty();
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSGetSectionMemoryAddresses(plugin_container_handle handle, void **textAddress, void **dataAddress) {
    if (handle == 0 || textAddress == nullptr || dataAddress == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    for (auto &curContainer : gLoadedPlugins) {
        if (curContainer->getHandle() == handle) {
            *textAddress = curContainer->getPluginInformation()->getTextMemoryAddress();
            *dataAddress = curContainer->getPluginInformation()->getDataMemoryAddress();
            return PLUGIN_BACKEND_API_ERROR_NONE;
        }
    }
    return PLUGIN_BACKEND_API_INVALID_HANDLE;
}

WUMS_EXPORT_FUNCTION(WUPSGetAPIVersion);
WUMS_EXPORT_FUNCTION(WUPSGetNumberOfLoadedPlugins);
WUMS_EXPORT_FUNCTION(WUPSGetSectionInformationForPlugin);
WUMS_EXPORT_FUNCTION(WUPSWillReloadPluginsOnNextLaunch);
WUMS_EXPORT_FUNCTION(WUPSGetSectionMemoryAddresses);