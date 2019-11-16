#ifndef __IPC_UTILS_H_
#define __IPC_UTILS_H_



#include "plugin/PluginLoader.h"
#include "plugin/PluginInformation.h"


#ifdef __cplusplus
extern "C" {
#endif

#define IPC_ERROR_INVALID_NONE      0
#define IPC_ERROR_INVALID_SIZE      0xFFFFFFFF
#define IPC_ERROR_INVALID_ARG       0xFFFFFFFE
#define IPC_ERROR_FAILED_ALLOC      0xFFFFFFFD

#define IOCTL_OPEN_PLUGIN_LOADER                                0x01
#define IOCTL_CLOSE_PLUGIN_LOADER                               0x02
#define IOCTL_PLUGIN_LOADER_GET_INFORMATION_FOR_PATH            0x03
#define IOCTL_PLUGIN_LOADER_GET_INFORMATION_LOADED              0x04
#define IOCTL_PLUGIN_LOADER_GET_INFORMATION_DETAILS             0x05
#define IOCTL_PLUGIN_LOADER_DELETE_INFORMATION                  0x06
#define IOCTL_PLUGIN_LOADER_LINK_VIA_INFORMATION                0x07
#define IOCTL_PLUGIN_LOADER_LINK_VIA_INFORMATION_ON_RESTART     0x08
#define IOCTL_PLUGIN_INFORMATION_GET_INFORMATION_FOR_FILEPATH   0x09

/* IPC message */
typedef struct ipcmessage {
    uint32_t command;
    union {
        struct {
            uint32_t *buffer_in;
            uint32_t  length_in;
            uint32_t *buffer_io;
            uint32_t  length_io;
        } ioctl;
    };
} __attribute__((packed)) ipcmessage;

typedef uint32_t plugin_information_handle;

/* plugin_information message */
typedef struct plugin_information {
    plugin_information_handle handle;
    char path[256];
    char name[256];
    char author[256];
} plugin_information;

int ipc_ioctl(ipcmessage *message);

#ifdef __cplusplus
}
#endif

#endif
