#include "globals.h"
#include "logger.h"
#include "plugin/PluginContainer.h"
#include "plugin/PluginData.h"
#include "plugin/PluginDataFactory.h"
#include "plugin/PluginMetaInformation.h"
#include "plugin/PluginMetaInformationFactory.h"
#include "utils.h"

#include <ranges>
#include <wums.h>
#include <wups_backend/import_defines.h>

static void fillPluginInformation(wups_backend_plugin_information *out, const PluginMetaInformation &metaInformation) {
    out->plugin_information_version = WUPS_BACKEND_PLUGIN_INFORMATION_VERSION;
    strncpy(out->author, metaInformation.getAuthor().c_str(), sizeof(out->author) - 1);
    strncpy(out->buildTimestamp, metaInformation.getBuildTimestamp().c_str(), sizeof(out->buildTimestamp) - 1);
    strncpy(out->description, metaInformation.getDescription().c_str(), sizeof(out->description) - 1);
    strncpy(out->name, metaInformation.getName().c_str(), sizeof(out->name) - 1);
    strncpy(out->license, metaInformation.getLicense().c_str(), sizeof(out->license) - 1);
    strncpy(out->version, metaInformation.getVersion().c_str(), sizeof(out->version) - 1);
    strncpy(out->storageId, metaInformation.getStorageId().c_str(), sizeof(out->storageId) - 1);
    out->size = metaInformation.getSize();
}

extern "C" PluginBackendApiErrorType WUPSLoadAndLinkByDataHandle(const wups_backend_plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size) {
    if (plugin_data_handle_list == nullptr || plugin_data_handle_list_size == 0) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    std::lock_guard lock(gLoadedDataMutex);
    for (uint32_t i = 0; i < plugin_data_handle_list_size; i++) {
        const auto handle = plugin_data_handle_list[i];
        bool found        = false;

        for (const auto &pluginData : gLoadedData) {
            if (pluginData->getHandle() == handle) {
                gLoadOnNextLaunch.insert(pluginData);
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

extern "C" PluginBackendApiErrorType WUPSDeletePluginData(const wups_backend_plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size) {
    if (plugin_data_handle_list != nullptr && plugin_data_handle_list_size != 0) {
        std::lock_guard lock(gLoadedDataMutex);
        for (auto &handle : std::span(plugin_data_handle_list, plugin_data_handle_list_size)) {
            if (!remove_first_if(gLoadedData, [&handle](auto &cur) { return cur->getHandle() == handle; })) {
                DEBUG_FUNCTION_LINE_ERR("Failed to delete plugin data by handle %08X", handle);
            }
        }
    }
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSLoadPluginAsData(WUPSBackendGetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, wups_backend_plugin_data_handle *out) {
    if (out == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    std::unique_ptr<PluginData> pluginData;
    if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_PATH && path != nullptr) {
        pluginData = PluginDataFactory::load(path);
    } else if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_BUFFER && buffer != nullptr && size > 0) {
        pluginData = make_unique_nothrow<PluginData>(std::span(reinterpret_cast<uint8_t *>(buffer), size), "<UNKNOWN>");
    } else {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    if (!pluginData) {
        DEBUG_FUNCTION_LINE_ERR("PLUGIN_BACKEND_API_ERROR_FAILED_ALLOC");
        return PLUGIN_BACKEND_API_ERROR_FAILED_ALLOC;
    } else {
        *out = pluginData->getHandle();
        {
            std::lock_guard lockLoadedData(gLoadedDataMutex);
            gLoadedData.insert(std::move(pluginData));
        }
    }

    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSLoadPluginAsDataByPath(wups_backend_plugin_data_handle *output, const char *path) {
    return WUPSLoadPluginAsData(PLUGIN_INFORMATION_INPUT_TYPE_PATH, path, nullptr, 0, output);
}

extern "C" PluginBackendApiErrorType WUPSLoadPluginAsDataByBuffer(wups_backend_plugin_data_handle *output, char *buffer, size_t size) {
    return WUPSLoadPluginAsData(PLUGIN_INFORMATION_INPUT_TYPE_BUFFER, nullptr, buffer, size, output);
}

extern "C" PluginBackendApiErrorType WUPSGetPluginMetaInformationEx(WUPSBackendGetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, wups_backend_plugin_information *output, PluginBackendPluginParseError *errOut) {
    if (output == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    std::optional<PluginMetaInformation> pluginInfo;
    PluginParseErrors error = PLUGIN_PARSE_ERROR_UNKNOWN;
    if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_PATH && path != nullptr) {
        pluginInfo = PluginMetaInformationFactory::loadPlugin(path, error);
    } else if (inputType == PLUGIN_INFORMATION_INPUT_TYPE_BUFFER && buffer != nullptr && size > 0) {
        pluginInfo = PluginMetaInformationFactory::loadPlugin(std::span<uint8_t const>(reinterpret_cast<uint8_t *>(buffer), size), error);
    } else {
        if (errOut) {
            *errOut = PLUGIN_BACKEND_PLUGIN_PARSE_ERROR_UNKNOWN;
        }

        DEBUG_FUNCTION_LINE_ERR("PLUGIN_BACKEND_API_ERROR_INVALID_ARG");
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    if (errOut) {
        switch (error) {
            case PLUGIN_PARSE_ERROR_NONE:
                *errOut = PLUGIN_BACKEND_PLUGIN_PARSE_ERROR_NONE;
                break;
            case PLUGIN_PARSE_ERROR_INCOMPATIBLE_VERSION:
                *errOut = PLUGIN_BACKEND_PLUGIN_PARSE_ERROR_INCOMPATIBLE_VERSION;
                break;
            case PLUGIN_PARSE_ERROR_UNKNOWN:
            case PLUGIN_PARSE_ERROR_BUFFER_EMPTY:
            case PLUGIN_PARSE_ERROR_ELFIO_PARSE_FAILED:
            case PLUGIN_PARSE_ERROR_IO_ERROR:
            case PLUGIN_PARSE_ERROR_NO_PLUGIN:
                *errOut = PLUGIN_BACKEND_PLUGIN_PARSE_ERROR_UNKNOWN;
                break;
        }
    }

    if (!pluginInfo) {
        DEBUG_FUNCTION_LINE_ERR("PLUGIN_BACKEND_API_ERROR_FILE_NOT_FOUND");
        return PLUGIN_BACKEND_API_ERROR_FILE_NOT_FOUND;
    }
    fillPluginInformation(output, *pluginInfo);

    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSGetPluginMetaInformation(WUPSBackendGetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, wups_backend_plugin_information *output) {
    return WUPSGetPluginMetaInformationEx(inputType, path, buffer, size, output, nullptr);
}

extern "C" PluginBackendApiErrorType WUPSGetPluginMetaInformationByPath(wups_backend_plugin_information *output, const char *path) {
    return WUPSGetPluginMetaInformationEx(PLUGIN_INFORMATION_INPUT_TYPE_PATH, path, nullptr, 0, output, nullptr);
}

extern "C" PluginBackendApiErrorType WUPSGetPluginMetaInformationByBuffer(wups_backend_plugin_information *output, char *buffer, size_t size) {
    return WUPSGetPluginMetaInformationEx(PLUGIN_INFORMATION_INPUT_TYPE_BUFFER, nullptr, buffer, size, output, nullptr);
}

extern "C" PluginBackendApiErrorType WUPSGetPluginDataForContainerHandles(const wups_backend_plugin_container_handle *plugin_container_handle_list, wups_backend_plugin_data_handle *plugin_data_list, uint32_t buffer_size) {
    if (plugin_container_handle_list == nullptr || buffer_size == 0) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }

    std::lock_guard lock(gLoadedDataMutex);
    for (uint32_t i = 0; i < buffer_size; i++) {
        const auto handle = plugin_container_handle_list[i];
        bool found        = false;
        for (const auto &curContainer : gLoadedPlugins) {
            if (curContainer.getHandle() == handle) {
                auto pluginData     = curContainer.getPluginDataCopy();
                plugin_data_list[i] = pluginData->getHandle();
                gLoadedData.insert(std::move(pluginData));
                found = true;
                break;
            }
        }
        if (!found) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get container for handle %08X", handle);
            return PLUGIN_BACKEND_API_INVALID_HANDLE;
        }
    }

    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSGetMetaInformation(const wups_backend_plugin_container_handle *plugin_container_handle_list, wups_backend_plugin_information *plugin_information_list, uint32_t buffer_size) {
    PluginBackendApiErrorType res = PLUGIN_BACKEND_API_ERROR_NONE;
    if (plugin_container_handle_list != nullptr && buffer_size != 0) {
        for (uint32_t i = 0; i < buffer_size; i++) {
            auto handle = plugin_container_handle_list[i];
            bool found  = false;
            for (const auto &curContainer : gLoadedPlugins) {
                if (curContainer.getHandle() == handle) {
                    const auto &metaInfo = curContainer.getMetaInformation();

                    plugin_information_list[i].plugin_information_version = WUPS_BACKEND_PLUGIN_INFORMATION_VERSION;
                    strncpy(plugin_information_list[i].storageId, metaInfo.getStorageId().c_str(), sizeof(plugin_information_list[i].storageId) - 1);
                    strncpy(plugin_information_list[i].author, metaInfo.getAuthor().c_str(), sizeof(plugin_information_list[i].author) - 1);
                    strncpy(plugin_information_list[i].buildTimestamp, metaInfo.getBuildTimestamp().c_str(), sizeof(plugin_information_list[i].buildTimestamp) - 1);
                    strncpy(plugin_information_list[i].description, metaInfo.getDescription().c_str(), sizeof(plugin_information_list[i].description) - 1);
                    strncpy(plugin_information_list[i].name, metaInfo.getName().c_str(), sizeof(plugin_information_list[i].name) - 1);
                    strncpy(plugin_information_list[i].license, metaInfo.getLicense().c_str(), sizeof(plugin_information_list[i].license) - 1);
                    strncpy(plugin_information_list[i].version, metaInfo.getVersion().c_str(), sizeof(plugin_information_list[i].version) - 1);
                    plugin_information_list[i].size = metaInfo.getSize();
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

extern "C" PluginBackendApiErrorType WUPSGetLoadedPlugins(wups_backend_plugin_container_handle *io_handles, uint32_t buffer_size, uint32_t *outSize, uint32_t *plugin_information_version) {
    if (plugin_information_version == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    *plugin_information_version = WUPS_BACKEND_PLUGIN_INFORMATION_VERSION;
    uint32_t counter            = 0;
    for (const auto &plugin : gLoadedPlugins) {
        if (counter < buffer_size) {
            io_handles[counter] = plugin.getHandle();
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
    *outVersion = 3;
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSGetNumberOfLoadedPlugins(uint32_t *outCount) {
    if (outCount == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    *outCount = gLoadedPlugins.size();
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSGetSectionInformationForPlugin(const wups_backend_plugin_container_handle handle, wups_backend_plugin_section_info *plugin_section_list, const uint32_t buffer_size, uint32_t *out_count) {
    PluginBackendApiErrorType res = PLUGIN_BACKEND_API_ERROR_NONE;
    if (out_count != nullptr) {
        *out_count = 0;
    }
    if (handle != 0 && plugin_section_list != nullptr && buffer_size != 0) {
        bool found = false;
        for (const auto &curContainer : gLoadedPlugins) {
            if (curContainer.getHandle() == handle) {
                found                       = true;
                const auto &sectionInfoList = curContainer.getPluginInformation().getSectionInfoList();

                uint32_t offset = 0;
                for (auto const &sectionInfo : sectionInfoList | std::views::values) {
                    if (offset >= buffer_size) {
                        break;
                    }
                    plugin_section_list[offset].plugin_section_info_version = WUPS_BACKEND_PLUGIN_SECTION_INFORMATION_VERSION;
                    strncpy(plugin_section_list[offset].name, sectionInfo.getName().c_str(), sizeof(plugin_section_list[offset].name) - 1);
                    plugin_section_list[offset].address = reinterpret_cast<void *>(sectionInfo.getAddress());
                    plugin_section_list[offset].size    = sectionInfo.getSize();
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
    std::lock_guard lock(gLoadedDataMutex);
    *out = !gLoadOnNextLaunch.empty();
    return PLUGIN_BACKEND_API_ERROR_NONE;
}

extern "C" PluginBackendApiErrorType WUPSGetSectionMemoryAddresses(const wups_backend_plugin_container_handle handle, void **textAddress, void **dataAddress) {
    if (handle == 0 || textAddress == nullptr || dataAddress == nullptr) {
        return PLUGIN_BACKEND_API_ERROR_INVALID_ARG;
    }
    for (const auto &curContainer : gLoadedPlugins) {
        if (curContainer.getHandle() == handle) {
            *textAddress = const_cast<void *>(curContainer.getPluginInformation().getTextMemory().data());
            *dataAddress = const_cast<void *>(curContainer.getPluginInformation().getDataMemory().data());
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

// API 3.0
extern "C" PluginBackendApiErrorType WUPSGetPluginMetaInformationByPathEx(wups_backend_plugin_information *output, const char *path, PluginBackendPluginParseError *err) {
    return WUPSGetPluginMetaInformationEx(PLUGIN_INFORMATION_INPUT_TYPE_PATH, path, nullptr, 0, output, err);
}

extern "C" PluginBackendApiErrorType WUPSGetPluginMetaInformationByBufferEx(wups_backend_plugin_information *output, char *buffer, const size_t size, PluginBackendPluginParseError *err) {
    return WUPSGetPluginMetaInformationEx(PLUGIN_INFORMATION_INPUT_TYPE_BUFFER, nullptr, buffer, size, output, err);
}

WUMS_EXPORT_FUNCTION(WUPSGetPluginMetaInformationByPathEx);
WUMS_EXPORT_FUNCTION(WUPSGetPluginMetaInformationByBufferEx);
