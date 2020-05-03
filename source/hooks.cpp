#include "hooks.h"
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

static const char **hook_names = (const char *[]) {
        "WUPS_LOADER_HOOK_INIT_OVERLAY",
        "WUPS_LOADER_HOOK_INIT_KERNEL",
        "WUPS_LOADER_HOOK_INIT_VID_MEM",
        "WUPS_LOADER_HOOK_INIT_WUT_MALLOC",
        "WUPS_LOADER_HOOK_FINI_WUT_MALLOC",
        "WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB",
        "WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB",
        "WUPS_LOADER_HOOK_INIT_WUT_NEWLIB",
        "WUPS_LOADER_HOOK_FINI_WUT_NEWLIB",
        "WUPS_LOADER_HOOK_INIT_WUT_STDCPP",
        "WUPS_LOADER_HOOK_FINI_WUT_STDCPP",
        "WUPS_LOADER_HOOK_INIT_PLUGIN",
        "WUPS_LOADER_HOOK_DEINIT_PLUGIN",
        "WUPS_LOADER_HOOK_APPLICATION_START",
        "WUPS_LOADER_HOOK_FUNCTIONS_PATCHED",
        "WUPS_LOADER_HOOK_RELEASE_FOREGROUND",
        "WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND",
        "WUPS_LOADER_HOOK_APPLICATION_END",
        "WUPS_LOADER_HOOK_CONFIRM_RELEASE_FOREGROUND",
        "WUPS_LOADER_HOOK_SAVES_DONE_READY_TO_RELEASE",
        "WUPS_LOADER_HOOK_VSYNC",
        "WUPS_LOADER_HOOK_GET_CONFIG",
        "WUPS_LOADER_HOOK_VID_DRC_DRAW",
        "WUPS_LOADER_HOOK_VID_TV_DRAW",
        "WUPS_LOADER_HOOK_APPLET_START"};

void CallHookEx(plugin_information_t *pluginInformation, wups_loader_hook_type_t hook_type, int32_t plugin_index_needed) {
    for (int32_t plugin_index = 0; plugin_index < pluginInformation->number_used_plugins; plugin_index++) {
        plugin_information_single_t *plugin_data = &pluginInformation->plugin_data[plugin_index];
        if (plugin_index_needed != -1 && plugin_index_needed != plugin_index) {
            continue;
        }

        //DEBUG_FUNCTION_LINE("Checking hook functions for %s.\n",plugin_data->plugin_name);
        //DEBUG_FUNCTION_LINE("Found hooks: %d\n",plugin_data->number_used_hooks);
        for (uint32_t j = 0; j < plugin_data->info.number_used_hooks; j++) {
            replacement_data_hook_t *hook_data = &plugin_data->info.hooks[j];
            if (hook_data->type == hook_type) {
                DEBUG_FUNCTION_LINE("Calling hook of type %s for plugin %s [%d]", hook_names[hook_data->type], plugin_data->meta.name, hook_type);
                void *func_ptr = hook_data->func_pointer;
                if (func_ptr != NULL) {
                    //DEBUG_FUNCTION_LINE("function pointer is %08x\n",func_ptr);
                    if (hook_type == WUPS_LOADER_HOOK_INIT_PLUGIN) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_DEINIT_PLUGIN) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_APPLICATION_START) {
                        wups_loader_app_started_args_t args;
                        ((void (*)(wups_loader_app_started_args_t)) ((uint32_t *) func_ptr))(args);
                    } else if (hook_type == WUPS_LOADER_HOOK_FUNCTIONS_PATCHED) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_APPLICATION_END) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_INIT_WUT_MALLOC) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_FINI_WUT_MALLOC) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_INIT_WUT_NEWLIB) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_FINI_WUT_NEWLIB) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_INIT_WUT_STDCPP) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else if (hook_type == WUPS_LOADER_HOOK_FINI_WUT_STDCPP) {
                        ((void (*)(void)) ((uint32_t *) func_ptr))();
                    } else {
                        DEBUG_FUNCTION_LINE("######################################");
                        DEBUG_FUNCTION_LINE("Hook is not implemented %s [%d]", hook_names[hook_type], hook_type);
                        DEBUG_FUNCTION_LINE("######################################");
                    }
                } else {
                    DEBUG_FUNCTION_LINE("Failed to call hook. It was not defined\n");
                }
            }
        }
    }
}
