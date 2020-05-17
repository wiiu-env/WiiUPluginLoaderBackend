#pragma once

#include <wums.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_NONE      0
#define ERROR_INVALID_SIZE      0xFFFFFFFF
#define ERROR_INVALID_ARG       0xFFFFFFFE
#define ERROR_FAILED_ALLOC      0xFFFFFFFD
#define ERROR_FILE_NOT_FOUND    0xFFFFFFFC

typedef enum GetPluginInformationInputType {
    PLUGIN_INFORMATION_INPUT_TYPE_PATH = 0,
    PLUGIN_INFORMATION_INPUT_TYPE_BUFFER = 1,
} GetPluginInformationInputType;

typedef uint32_t plugin_container_handle;
typedef uint32_t plugin_data_handle;

/* plugin_information message */
typedef struct __attribute__((__packed__)) plugin_information {
    char name[256];
    char author[256];
    char buildTimestamp[256];
    char description[256];
    char license[256];
    char version[256];
    size_t size;
} plugin_information;

void fillPluginInformation(plugin_information *out, PluginMetaInformation *metaInformation);

int32_t WUPSLoadAndLinkByDataHandle(const plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size);

int32_t WUPSDeletePluginContainer(const plugin_container_handle *handle_list, uint32_t handle_list_size);

int32_t WUPSDeletePluginData(const plugin_data_handle *plugin_data_handle_list, uint32_t plugin_data_handle_list_size);

int32_t WUPSLoadPluginAsData(GetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, plugin_data_handle *out);

int32_t WUPSLoadPluginAsDataByPath(plugin_data_handle *output, const char *path);

int32_t WUPSLoadPluginAsDataByBuffer(plugin_data_handle *output, char *buffer, size_t size);

int32_t WUPSGetPluginMetaInformation(GetPluginInformationInputType inputType, const char *path, char *buffer, size_t size, plugin_information *output);

int32_t WUPSGetPluginMetaInformationByPath(plugin_information *output, const char *path);

int32_t WUPSGetPluginMetaInformationByBuffer(plugin_information *output, char *buffer, size_t size);

int32_t WUPSGetPluginDataForContainerHandles(const plugin_container_handle *plugin_container_handle_list, plugin_data_handle *plugin_data_list, uint32_t buffer_size);

int32_t WUPSGetMetaInformation(plugin_container_handle *plugin_container_handle_list, plugin_information *plugin_information_list, uint32_t buffer_size);

int32_t WUPSGetLoadedPlugins(plugin_container_handle *io_handles, uint32_t buffer_size, uint32_t *outSize);

#ifdef __cplusplus
}
#endif