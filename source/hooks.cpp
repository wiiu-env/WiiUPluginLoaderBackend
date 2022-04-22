#include "hooks.h"
#include "utils/StorageUtils.h"
#include "utils/logger.h"

void CallHook(plugin_information_t *pluginInformation, wups_loader_hook_type_t hook_type) {
    CallHookEx(pluginInformation, hook_type, -1);
}

bool HasHookCallHook(plugin_information_t *pluginInformation, wups_loader_hook_type_t hook_type) {
    for (int32_t plugin_index = 0; plugin_index < pluginInformation->number_used_plugins; plugin_index++) {
        plugin_information_single_t *plugin_data = &pluginInformation->plugin_data[plugin_index];

        for (uint32_t j = 0; j < plugin_data->info.number_used_hooks; j++) {
            replacement_data_hook_t *hook_data = &plugin_data->info.hooks[j];
            if (hook_data->type == hook_type) {
                return true;
            }
        }
    }
    return false;
}

static const char **hook_names = (const char *[]){
        "WUPS_LOADER_HOOK_INIT_WUT_MALLOC",
        "WUPS_LOADER_HOOK_FINI_WUT_MALLOC",
        "WUPS_LOADER_HOOK_INIT_WUT_NEWLIB",
        "WUPS_LOADER_HOOK_FINI_WUT_NEWLIB",
        "WUPS_LOADER_HOOK_INIT_WUT_STDCPP",
        "WUPS_LOADER_HOOK_FINI_WUT_STDCPP",
        "WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB",
        "WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB",
        "WUPS_LOADER_HOOK_INIT_WUT_SOCKETS",
        "WUPS_LOADER_HOOK_FINI_WUT_SOCKETS",

        "WUPS_LOADER_HOOK_INIT_WRAPPER",
        "WUPS_LOADER_HOOK_FINI_WRAPPER",

        "WUPS_LOADER_HOOK_GET_CONFIG",
        "WUPS_LOADER_HOOK_CONFIG_CLOSED",

        "WUPS_LOADER_HOOK_INIT_STORAGE",

        "WUPS_LOADER_HOOK_INIT_PLUGIN",
        "WUPS_LOADER_HOOK_DEINIT_PLUGIN",
        "WUPS_LOADER_HOOK_APPLICATION_STARTS",
        "WUPS_LOADER_HOOK_FUNCTIONS_PATCHED",
        "WUPS_LOADER_HOOK_RELEASE_FOREGROUND",
        "WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND",
        "WUPS_LOADER_HOOK_APPLICATION_REQUESTS_EXIT",
        "WUPS_LOADER_HOOK_APPLICATION_ENDS"};

void CallHookEx(plugin_information_t *pluginInformation, wups_loader_hook_type_t hook_type, int32_t plugin_index_needed) {
    DEBUG_FUNCTION_LINE_VERBOSE("Calling hook of type %s [%d]", hook_names[hook_type], hook_type);
    for (int32_t plugin_index = 0; plugin_index < pluginInformation->number_used_plugins; plugin_index++) {
        plugin_information_single_t *plugin_data = &pluginInformation->plugin_data[plugin_index];
        if (plugin_index_needed != -1 && plugin_index_needed != plugin_index) {
            continue;
        }

        //DEBUG_FUNCTION_LINE("Checking hook functions for %s.",plugin_data->plugin_name);
        //DEBUG_FUNCTION_LINE("Found hooks: %d",plugin_data->number_used_hooks);
        for (uint32_t j = 0; j < plugin_data->info.number_used_hooks; j++) {
            replacement_data_hook_t *hook_data = &plugin_data->info.hooks[j];
            if (hook_data->type == hook_type) {
                DEBUG_FUNCTION_LINE_VERBOSE("Calling hook of type %s for plugin %s [%d]", hook_names[hook_data->type], plugin_data->meta.name, hook_type);
                void *func_ptr = hook_data->func_pointer;
                if (func_ptr != nullptr) {
                    //DEBUG_FUNCTION_LINE("function pointer is %08x",func_ptr);
                    if (hook_type == WUPS_LOADER_HOOK_INIT_PLUGIN ||
                        hook_type == WUPS_LOADER_HOOK_DEINIT_PLUGIN ||
                        hook_type == WUPS_LOADER_HOOK_APPLICATION_STARTS ||
                        hook_type == WUPS_LOADER_HOOK_FUNCTIONS_PATCHED ||
                        hook_type == WUPS_LOADER_HOOK_APPLICATION_ENDS ||
                        hook_type == WUPS_LOADER_HOOK_APPLICATION_REQUESTS_EXIT ||
                        hook_type == WUPS_LOADER_HOOK_INIT_WUT_MALLOC ||
                        hook_type == WUPS_LOADER_HOOK_FINI_WUT_MALLOC ||
                        hook_type == WUPS_LOADER_HOOK_INIT_WUT_NEWLIB ||
                        hook_type == WUPS_LOADER_HOOK_FINI_WUT_NEWLIB ||
                        hook_type == WUPS_LOADER_HOOK_INIT_WUT_STDCPP ||
                        hook_type == WUPS_LOADER_HOOK_FINI_WUT_STDCPP ||
                        hook_type == WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB ||
                        hook_type == WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB ||
                        hook_type == WUPS_LOADER_HOOK_INIT_WUT_SOCKETS ||
                        hook_type == WUPS_LOADER_HOOK_FINI_WUT_SOCKETS ||
                        hook_type == WUPS_LOADER_HOOK_INIT_WRAPPER ||
                        hook_type == WUPS_LOADER_HOOK_FINI_WRAPPER ||
                        hook_type == WUPS_LOADER_HOOK_GET_CONFIG ||
                        hook_type == WUPS_LOADER_HOOK_CONFIG_CLOSED ||
                        hook_type == WUPS_LOADER_HOOK_RELEASE_FOREGROUND ||
                        hook_type == WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND) {
                        // clang-format off
                        ((void(*)())((uint32_t *) func_ptr))();
                        // clang-format on
                    } else if (hook_type == WUPS_LOADER_HOOK_INIT_STORAGE) {
                        wups_loader_init_storage_args_t args;
                        args.open_storage_ptr  = &StorageUtils::OpenStorage;
                        args.close_storage_ptr = &StorageUtils::CloseStorage;
                        args.plugin_id         = plugin_data->meta.storageId;
                        // clang-format off
                        ((void(*)(wups_loader_init_storage_args_t))((uint32_t *) func_ptr))(args);
                        // clang-format on
                    } else {
                        DEBUG_FUNCTION_LINE_ERR("######################################");
                        DEBUG_FUNCTION_LINE_ERR("Hook is not implemented %s [%d]", hook_names[hook_type], hook_type);
                        DEBUG_FUNCTION_LINE_ERR("######################################");
                    }
                } else {
                    DEBUG_FUNCTION_LINE_ERR("Failed to call hook. It was not defined");
                }
            }
        }
    }
}
