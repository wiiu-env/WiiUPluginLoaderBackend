#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ipc.h"

extern void RestoreEverything();
extern void afterLoadAndLink();

int ipc_ioctl(ipcmessage *message) {
    int res = 0;

    switch(message->command) {
    case IOCTL_OPEN_PLUGIN_LOADER: {
        DEBUG_FUNCTION_LINE("IOCTL_OPEN_PLUGIN_LOADER\n");
        if(message->ioctl.length_in != 8 || message->ioctl.length_io < 4) {
            DEBUG_FUNCTION_LINE("IPC_ERROR_INVALID_SIZE\n");
            res = IPC_ERROR_INVALID_SIZE;
        } else {
            uint32_t startAddress = message->ioctl.buffer_in[0];
            uint32_t endAddress = message->ioctl.buffer_in[1];


            PluginLoader * pluginLoader = new PluginLoader((void*)startAddress, (void*)endAddress);
            if(pluginLoader == NULL) {
                DEBUG_FUNCTION_LINE("Creating plugin loader for %08X %08X failed \n",startAddress, endAddress);
                res = IPC_ERROR_FAILED_ALLOC;
            } else {
                DEBUG_FUNCTION_LINE("Creating plugin loader %08X %08X: %08X\n",startAddress, endAddress, pluginLoader);
                message->ioctl.buffer_io[0] = (uint32_t) pluginLoader;
            }
        }
        break;
    }
    case IOCTL_CLOSE_PLUGIN_LOADER: {
        DEBUG_FUNCTION_LINE("IOCTL_CLOSE_PLUGIN_LOADER\n");
        if(message->ioctl.length_in != 4) {
            DEBUG_FUNCTION_LINE("IPC_ERROR_INVALID_SIZE\n");
            res = IPC_ERROR_INVALID_SIZE;
        } else {
            void * ptr = (void *) message->ioctl.buffer_in[0];

            DEBUG_FUNCTION_LINE("Closing %08X\n", ptr);

            if(ptr != NULL/* && (PluginLoader* pluginLoader = dynamic_cast<PluginLoader*>(ptr))*/) {
                PluginLoader* pluginLoader = (PluginLoader * )ptr;
                delete pluginLoader;
            } else {
                res = IPC_ERROR_INVALID_ARG;
            }
        }

        break;
    }
    case IOCTL_PLUGIN_LOADER_GET_INFORMATION_FOR_PATH: {
        DEBUG_FUNCTION_LINE("IOCTL_PLUGIN_LOADER_GET_INFORMATION_FOR_PATH\n");
        if(message->ioctl.length_in != 8 || message->ioctl.length_io < 4) {
            DEBUG_FUNCTION_LINE("IPC_ERROR_INVALID_SIZE\n");
            res = IPC_ERROR_INVALID_SIZE;
        } else {
            void * ptr = (void *) message->ioctl.buffer_in[0];
            char * path = (char *) message->ioctl.buffer_in[1];
            uint32_t * filledCount = (uint32_t *)message->ioctl.buffer_io;
            plugin_information_handle * io_handles  = (plugin_information_handle *) &(message->ioctl.buffer_io[1]);
            uint32_t lengthIo = message->ioctl.length_io - 4;

            *filledCount = 0;

            DEBUG_FUNCTION_LINE("ptr %08X ptr_value %08X path %08X %s, filledcount_ptr %08X io_handles%08X lengthio%d\n", ptr,*(uint32_t*)ptr, path,path, filledCount,io_handles,lengthIo);

            if(ptr != NULL && path != NULL/* && (PluginLoader* pluginLoader = dynamic_cast<PluginLoader*>(ptr))*/) {
                PluginLoader* pluginLoader = (PluginLoader * )ptr;
                std::vector<PluginInformation *> pluginList =  pluginLoader->getPluginInformation(path);
                uint32_t pluginInfoSpace = lengthIo / sizeof(plugin_information_handle);

                DEBUG_FUNCTION_LINE("pluginInfoSpace %d\n", pluginInfoSpace);

                uint32_t cur = 0;

                for (std::vector<PluginInformation *>::iterator it = pluginList.begin() ; it != pluginList.end(); ++it) {

                    PluginInformation * curPlugin = *it;
                    DEBUG_FUNCTION_LINE("cur %d filledCount %d curPlugin %08X\n", cur, *filledCount,curPlugin);
                    if(cur >= pluginInfoSpace) {
                        DEBUG_FUNCTION_LINE("deleted plugins because they don't fit in buffer..\n");
                        // delete all that don't fit into the target list.
                        delete curPlugin;
                        continue;
                    }
                    io_handles[cur] = (plugin_information_handle) curPlugin;
                    DEBUG_FUNCTION_LINE("%08X = %08X\n", &(io_handles[cur]), io_handles[cur]);
                    cur++;
                    (*filledCount)++;
                }

                DCFlushRange(message->ioctl.buffer_io,message->ioctl.length_io);
                ICInvalidateRange(message->ioctl.buffer_io,message->ioctl.length_io);
            } else {
                res = IPC_ERROR_INVALID_ARG;
            }
        }
        break;
    }

    case IOCTL_PLUGIN_LOADER_GET_INFORMATION_LOADED: {
        if(message->ioctl.length_in != 4 || message->ioctl.length_io < 4) {
            res = IPC_ERROR_INVALID_SIZE;
        } else {
            void * ptr = (void *) message->ioctl.buffer_in[0];
            uint32_t * filledCount = (uint32_t *)message->ioctl.buffer_io;
            plugin_information_handle * io_handles  = (plugin_information_handle *)  &(message->ioctl.buffer_io[1]);
            uint32_t lengthIo = message->ioctl.length_io - 4;

            *filledCount = 0;

            if(ptr != NULL) {
                PluginLoader* pluginLoader = (PluginLoader * )ptr;
                std::vector<PluginInformation *> pluginList =  pluginLoader->getPluginsLoadedInMemory();
                uint32_t pluginInfoSpace = lengthIo / sizeof(plugin_information_handle);

                uint32_t cur = 0;

                if(pluginInfoSpace > 0) {
                    for (std::vector<PluginInformation *>::iterator it = pluginList.begin() ; it != pluginList.end(); ++it) {
                        PluginInformation * curPlugin = *it;
                        if(cur >= pluginInfoSpace) {
                            // delete all that don't fit into the target list.
                            delete curPlugin;
                            continue;
                        }
                        io_handles[cur] = (plugin_information_handle) curPlugin;
                        cur++;
                        (*filledCount)++;
                    }
                }
                DCFlushRange(message->ioctl.buffer_io,message->ioctl.length_io);
                ICInvalidateRange(message->ioctl.buffer_io,message->ioctl.length_io);
            } else {
                res = IPC_ERROR_INVALID_ARG;
            }
        }
        break;
    }

    case IOCTL_PLUGIN_LOADER_DELETE_INFORMATION: {
        DEBUG_FUNCTION_LINE("IOCTL_PLUGIN_LOADER_DELETE_INFORMATION\n");
        if(message->ioctl.length_in != 4) {
            res = IPC_ERROR_INVALID_SIZE;
        } else {
            plugin_information_handle * plugin_handle = (plugin_information_handle *) message->ioctl.buffer_in[0];

            if(plugin_handle != NULL/* && (PluginLoader* pluginLoader = dynamic_cast<PluginLoader*>(ptr))*/) {
                PluginInformation* curInformation = (PluginInformation *)plugin_handle;
                DEBUG_FUNCTION_LINE("Deleting %08X\n", curInformation);
                delete curInformation;
            } else {
                res = IPC_ERROR_INVALID_ARG;
            }
        }
        break;
    }

    case IOCTL_PLUGIN_LOADER_GET_INFORMATION_DETAILS: {
        DEBUG_FUNCTION_LINE("IOCTL_PLUGIN_LOADER_GET_INFORMATION_DETAILS\n");
        if(message->ioctl.length_in != 8) {
            DEBUG_FUNCTION_LINE("IPC_ERROR_INVALID_SIZE\n");
            res = IPC_ERROR_INVALID_SIZE;
        } else {
            plugin_information_handle * plugin_information_handle_list = (plugin_information_handle *) message->ioctl.buffer_in[0];
            uint32_t plugin_information_handle_list_size = (uint32_t) message->ioctl.buffer_in[1];

            DEBUG_FUNCTION_LINE("plugin_information_handle_list %08X plugin_information_handle_list_size %d\n",plugin_information_handle_list,plugin_information_handle_list_size);

            plugin_information * plugin_information_list = (plugin_information *) message->ioctl.buffer_io;

            if(message->ioctl.length_io < plugin_information_handle_list_size * sizeof(plugin_information)) {
                res = IPC_ERROR_INVALID_SIZE;
            } else {
                if(plugin_information_handle_list != NULL && plugin_information_handle_list != NULL) {
                    for(uint32_t i = 0; i  < plugin_information_handle_list_size; i++ ) {
                        PluginInformation* curHandle = (PluginInformation * )plugin_information_handle_list[i];
                        plugin_information_list[i].handle = plugin_information_handle_list[i];
                        strncpy(plugin_information_list[i].path, curHandle->getPath().c_str(),255);
                        strncpy(plugin_information_list[i].name, curHandle->getName().c_str(),255);
                        strncpy(plugin_information_list[i].author, curHandle->getAuthor().c_str(),255);
                    }
                } else {
                    res = IPC_ERROR_INVALID_ARG;
                }
            }
            DCFlushRange(message->ioctl.buffer_io,message->ioctl.length_io);
            ICInvalidateRange(message->ioctl.buffer_io,message->ioctl.length_io);
        }
        break;
    }

    case IOCTL_PLUGIN_LOADER_LINK_VIA_INFORMATION: {
        DEBUG_FUNCTION_LINE("IOCTL_PLUGIN_LOADER_LINK_VIA_INFORMATION\n");
        if(message->ioctl.length_in != 12 || message->ioctl.length_io < 4) {
            res = IPC_ERROR_INVALID_SIZE;
        } else {
            void * ptr = (void *) message->ioctl.buffer_in[0];
            plugin_information_handle * plugin_handle_list = (plugin_information_handle *) message->ioctl.buffer_in[1];
            uint32_t plugin_handle_list_size = (uint32_t) message->ioctl.buffer_in[2];

            DEBUG_FUNCTION_LINE("ptr %08X plugin_handle_list %08X size %d\n",ptr,plugin_handle_list,plugin_handle_list_size);

            uint32_t * linkedCount = (uint32_t *)message->ioctl.buffer_io;
            *linkedCount = 0;

            if(ptr != NULL/* && (PluginLoader* pluginLoader = dynamic_cast<PluginLoader*>(ptr))*/) {
                PluginLoader* pluginLoader = (PluginLoader * )ptr;
                std::vector<PluginInformation*> willBeLoaded;
                for(uint32_t i = 0; i  < plugin_handle_list_size; i++ ) {

                    plugin_information_handle curHandle = plugin_handle_list[i];
                    DEBUG_FUNCTION_LINE("Adding handle %08X\n",curHandle);
                    if(curHandle != 0/* && (PluginInformation* curInformation = dynamic_cast<PluginInformation*>(curHandle))*/) {
                        PluginInformation* curInformation = (PluginInformation *)curHandle;
                        willBeLoaded.push_back(curInformation);
                    }
                }

                DEBUG_FUNCTION_LINE("willBeLoaded size is %d\n",willBeLoaded.size());

                //if(willBeLoaded.size() > 0){
                    RestoreEverything();
                    pluginLoader->loadAndLinkPlugins(willBeLoaded);
                    afterLoadAndLink();
                //}

                *linkedCount = willBeLoaded.size();
            } else {
                res = IPC_ERROR_INVALID_ARG;
            }
        }
        break;
    }
    default:
        res = IPC_ERROR_INVALID_ARG;
        break;
    }

    return res;
}
