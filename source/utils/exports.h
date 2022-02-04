#pragma once

#include <wums.h>
#include <wups_backend/import_defines.h>

#ifdef __cplusplus
extern "C" {
#endif

PluginBackendApiErrorType WUPSLoadAndLinkByDataHandle(const plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size);

PluginBackendApiErrorType WUPSDeletePluginContainer(const plugin_container_handle *handle_list, uint32_t handle_list_size);

PluginBackendApiErrorType WUPSDeletePluginData(const plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size);

PluginBackendApiErrorType WUPSLoadPluginAsData(GetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, plugin_data_handle *out);

PluginBackendApiErrorType WUPSLoadPluginAsDataByPath(plugin_data_handle *output, const char *path);

PluginBackendApiErrorType WUPSLoadPluginAsDataByBuffer(plugin_data_handle *output, char *buffer, size_t size);

PluginBackendApiErrorType WUPSGetPluginMetaInformation(GetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, plugin_information *output);

PluginBackendApiErrorType WUPSGetPluginMetaInformationByPath(plugin_information *output, const char *path);

PluginBackendApiErrorType WUPSGetPluginMetaInformationByBuffer(plugin_information *output, char *buffer, size_t size);

PluginBackendApiErrorType WUPSGetPluginDataForContainerHandles(const plugin_container_handle *plugin_container_handle_list, plugin_data_handle *plugin_data_list, uint32_t buffer_size);

PluginBackendApiErrorType WUPSGetMetaInformation(const plugin_container_handle *plugin_container_handle_list, plugin_information *plugin_information_list, uint32_t buffer_size);

PluginBackendApiErrorType WUPSGetLoadedPlugins(plugin_container_handle *io_handles, uint32_t buffer_size, uint32_t *outSize, uint32_t *plugin_information_version);

#ifdef __cplusplus
}
#endif