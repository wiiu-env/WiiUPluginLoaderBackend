#include "hooks.h"
#include "plugin/PluginContainer.h"
#include "utils/StorageUtils.h"
#include "utils/logger.h"
#include <memory>
#include <vector>

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
        "WUPS_LOADER_HOOK_RELEASE_FOREGROUND",
        "WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND",
        "WUPS_LOADER_HOOK_APPLICATION_REQUESTS_EXIT",
        "WUPS_LOADER_HOOK_APPLICATION_ENDS"};

void CallHook(const std::vector<std::unique_ptr<PluginContainer>> &plugins, wups_loader_hook_type_t hook_type) {
    DEBUG_FUNCTION_LINE_VERBOSE("Calling hook of type %s [%d]", hook_names[hook_type], hook_type);
    for (auto &plugin : plugins) {
        CallHook(plugin, hook_type);
    }
}

void CallHook(const std::unique_ptr<PluginContainer> &plugin, wups_loader_hook_type_t hook_type) {
    for (const auto &hook : plugin->getPluginInformation()->getHookDataList()) {
        if (hook->getType() == hook_type) {
            DEBUG_FUNCTION_LINE_VERBOSE("Calling hook of type %s for plugin %s [%d]", hook_names[hook->getType()], plugin->metaInformation->getName().c_str(), hook_type);
            void *func_ptr = hook->getFunctionPointer();
            if (func_ptr != nullptr) {
                if (hook_type == WUPS_LOADER_HOOK_INIT_PLUGIN ||
                    hook_type == WUPS_LOADER_HOOK_DEINIT_PLUGIN ||
                    hook_type == WUPS_LOADER_HOOK_APPLICATION_STARTS ||
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
                    args.plugin_id         = plugin->getMetaInformation()->getStorageId().c_str();
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