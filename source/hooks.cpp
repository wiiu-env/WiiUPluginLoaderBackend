#include "hooks.h"
#include "plugin/PluginContainer.h"
#include "utils/StorageUtilsDeprecated.h"
#include "utils/logger.h"
#include "utils/storage/StorageUtils.h"
#include <wups/storage.h>

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

        "WUPS_LOADER_HOOK_GET_CONFIG_DEPRECATED",
        "WUPS_LOADER_HOOK_CONFIG_CLOSED_DEPRECATED",

        "WUPS_LOADER_HOOK_INIT_STORAGE_DEPRECATED",

        "WUPS_LOADER_HOOK_INIT_PLUGIN",
        "WUPS_LOADER_HOOK_DEINIT_PLUGIN",
        "WUPS_LOADER_HOOK_APPLICATION_STARTS",
        "WUPS_LOADER_HOOK_RELEASE_FOREGROUND",
        "WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND",
        "WUPS_LOADER_HOOK_APPLICATION_REQUESTS_EXIT",
        "WUPS_LOADER_HOOK_APPLICATION_ENDS",
        "WUPS_LOADER_HOOK_INIT_STORAGE",
        "WUPS_LOADER_HOOK_INIT_CONFIG"};

void CallHook(const std::vector<PluginContainer> &plugins, wups_loader_hook_type_t hook_type) {
    DEBUG_FUNCTION_LINE_VERBOSE("Calling hook of type %s [%d]", hook_names[hook_type], hook_type);
    for (const auto &plugin : plugins) {
        CallHook(plugin, hook_type);
    }
}

void CallHook(const PluginContainer &plugin, wups_loader_hook_type_t hook_type) {
    for (const auto &hook : plugin.getPluginInformation().getHookDataList()) {
        if (hook.getType() == hook_type) {
            DEBUG_FUNCTION_LINE_VERBOSE("Calling hook of type %s for plugin %s [%d]", hook_names[hook.getType()], plugin.getMetaInformation().getName().c_str(), hook_type);
            void *func_ptr = hook.getFunctionPointer();
            if (func_ptr != nullptr) {
                switch (hook_type) {
                    case WUPS_LOADER_HOOK_INIT_WUT_MALLOC:
                    case WUPS_LOADER_HOOK_FINI_WUT_MALLOC:
                    case WUPS_LOADER_HOOK_INIT_WUT_NEWLIB:
                    case WUPS_LOADER_HOOK_FINI_WUT_NEWLIB:
                    case WUPS_LOADER_HOOK_INIT_WUT_STDCPP:
                    case WUPS_LOADER_HOOK_FINI_WUT_STDCPP:
                    case WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB:
                    case WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB:
                    case WUPS_LOADER_HOOK_INIT_WUT_SOCKETS:
                    case WUPS_LOADER_HOOK_FINI_WUT_SOCKETS:
                    case WUPS_LOADER_HOOK_INIT_WRAPPER:
                    case WUPS_LOADER_HOOK_FINI_WRAPPER:
                    case WUPS_LOADER_HOOK_GET_CONFIG_DEPRECATED:
                    case WUPS_LOADER_HOOK_CONFIG_CLOSED_DEPRECATED:
                    case WUPS_LOADER_HOOK_INIT_PLUGIN:
                    case WUPS_LOADER_HOOK_DEINIT_PLUGIN:
                    case WUPS_LOADER_HOOK_APPLICATION_STARTS:
                    case WUPS_LOADER_HOOK_RELEASE_FOREGROUND:
                    case WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND:
                    case WUPS_LOADER_HOOK_APPLICATION_REQUESTS_EXIT:
                    case WUPS_LOADER_HOOK_APPLICATION_ENDS:
                        // clang-format off
                        ((void(*)())((uint32_t *) func_ptr))();
                        // clang-format on
                        break;
                    case WUPS_LOADER_HOOK_INIT_STORAGE:
                    case WUPS_LOADER_HOOK_INIT_STORAGE_DEPRECATED: {
                        if (plugin.getMetaInformation().getWUPSVersion() <= WUPSVersion(0, 7, 1)) {
                            WUPSStorageDeprecated::wups_loader_init_storage_args_t_ args{};
                            args.open_storage_ptr  = &WUPSStorageDeprecated::StorageUtils::OpenStorage;
                            args.close_storage_ptr = &WUPSStorageDeprecated::StorageUtils::CloseStorage;
                            args.plugin_id         = plugin.getMetaInformation().getStorageId().c_str();
                            // clang-format off

                            ((void(*)(WUPSStorageDeprecated::wups_loader_init_storage_args_t_))((uint32_t *) func_ptr))(args);
                            // clang-format on
                            break;
                        }
                        wups_loader_init_storage_args_t_ args{};
                        args.version                      = WUPS_STORAGE_CUR_API_VERSION;
                        args.root_item                    = plugin.getStorageRootItem();
                        args.save_function_ptr            = &StorageUtils::API::SaveStorage;
                        args.force_reload_function_ptr    = &StorageUtils::API::ForceReloadStorage;
                        args.wipe_storage_function_ptr    = &StorageUtils::API::WipeStorage;
                        args.delete_item_function_ptr     = &StorageUtils::API::DeleteItem;
                        args.create_sub_item_function_ptr = &StorageUtils::API::CreateSubItem;
                        args.get_sub_item_function_ptr    = &StorageUtils::API::GetSubItem;
                        args.store_item_function_ptr      = &StorageUtils::API::StoreItem;
                        args.get_item_function_ptr        = &StorageUtils::API::GetItem;
                        args.get_item_size_function_ptr   = &StorageUtils::API::GetItemSize;
                        // clang-format off
                        auto res = ((WUPSStorageError(*)(wups_loader_init_storage_args_t_))((uint32_t *) func_ptr))(args);
                        // clang-format on
                        if (res != WUPS_STORAGE_ERROR_SUCCESS) {
                            // TODO: More error handling? Notification?
                            DEBUG_FUNCTION_LINE_ERR("WUPS_LOADER_HOOK_INIT_STORAGE failed for plugin %s: %s", plugin.getMetaInformation().getName().c_str(), WUPSStorageAPI_GetStatusStr(res));
                        }
                        break;
                    }
                    case WUPS_LOADER_HOOK_INIT_CONFIG: {
                        wups_loader_init_config_args_t args{.arg_version = 1, .plugin_identifier = plugin.getHandle()};
                        auto res = ((WUPSConfigAPIStatus(*)(wups_loader_init_config_args_t))((uint32_t *) func_ptr))(args);
                        // clang-format on
                        if (res != WUPSCONFIG_API_RESULT_SUCCESS) {
                            // TODO: More error handling? Notification?
                            DEBUG_FUNCTION_LINE_ERR("WUPS_LOADER_HOOK_INIT_CONFIG failed for plugin %s: %s", plugin.getMetaInformation().getName().c_str(), WUPSConfigAPI_GetStatusStr(res));
                        }
                        break;
                    }
                    default: {
                        DEBUG_FUNCTION_LINE_ERR("######################################");
                        DEBUG_FUNCTION_LINE_ERR("Hook is not implemented %s [%d]", hook_names[hook_type], hook_type);
                        DEBUG_FUNCTION_LINE_ERR("######################################");
                    }
                }
            } else {
                DEBUG_FUNCTION_LINE_ERR("Failed to call hook. It was not defined");
            }
            break;
        }
    }
}