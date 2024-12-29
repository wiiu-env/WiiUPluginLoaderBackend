#include "WUPSConfig.h"
#include "WUPSConfigAPI.h"
#include "WUPSConfigItem.h"
#include "utils/logger.h"

#include <wums/exports.h>
#include <wups/config.h>

int32_t WUPSConfig_Create(void **out, const char *name) {
    if (out == nullptr) {
        return -1;
    }
    *out = nullptr;
    WUPSConfigHandle outConfig;
    if (WUPSConfigAPIBackend::Intern::CreateConfig(name, &outConfig) == WUPSCONFIG_API_RESULT_SUCCESS) {
        *out = outConfig.handle;
        return 0;
    }

    return -1;
}

int32_t WUPSConfig_Destroy(WUPSConfigHandle) {
    return 0;
}

int32_t WUPSConfig_GetName(void *handle, char *out_buf, int32_t out_len) {
    if (out_buf == nullptr) {
        DEBUG_FUNCTION_LINE("Invalid param: \"out_buf\" was NULL");
        return -1;
    }
    auto config = WUPSConfigAPIBackend::Intern::GetConfigByHandle(WUPSConfigHandle(handle));
    if (!config) {
        DEBUG_FUNCTION_LINE_WARN("Failed to find WUPSConfig by handle %08X", handle);
        return -1;
    }
    snprintf(out_buf, out_len, "%s", config->getName().c_str());
    return 0;
}

int32_t WUPSConfig_AddCategory(void *configHandle, void *categoryHandle) {
    if (WUPSConfigAPIBackend::Category::AddCategory(WUPSConfigCategoryHandle(configHandle), WUPSConfigCategoryHandle(categoryHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return -1;
    }
    return 0;
}

int32_t WUPSConfig_AddCategoryByName(void *handle, const char *categoryName, void **out) {
    if (handle == nullptr || categoryName == nullptr || out == nullptr) {
        DEBUG_FUNCTION_LINE("Invalid param: \"handle\" or \"categoryName\" or \"out\" was NULL");
        return -1;
    }
    *out                                       = nullptr;
    WUPSConfigAPICreateCategoryOptions options = {
            .version = WUPS_API_CATEGORY_OPTION_VERSION_V1,
            .data    = {
                       .v1 = {
                               .name = categoryName,
                    },
            },
    };
    WUPSConfigCategoryHandle catHandle;
    if (WUPSConfigAPIBackend::Category::Create(options, &catHandle) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_WARN("Failed to create category");
        return -2;
    }

    if (WUPSConfigAPIBackend::Category::AddCategory(WUPSConfigCategoryHandle(handle), catHandle) < 0) {
        WUPSConfigAPIBackend::Intern::PopCategoryByHandle(catHandle); // make sure to destroy created category.
        return -3;
    }
    *out = catHandle.handle;
    return 0;
}

WUMS_EXPORT_FUNCTION(WUPSConfig_Create);
WUMS_EXPORT_FUNCTION(WUPSConfig_Destroy);
WUMS_EXPORT_FUNCTION(WUPSConfig_GetName);
WUMS_EXPORT_FUNCTION(WUPSConfig_AddCategoryByName);
WUMS_EXPORT_FUNCTION(WUPSConfig_AddCategory);

int32_t WUPSConfigCategory_Create(void **out, const char *name) {
    if (out == nullptr) {
        return -1;
    }
    *out                                       = nullptr;
    WUPSConfigAPICreateCategoryOptions options = {
            .version = WUPS_API_CATEGORY_OPTION_VERSION_V1,
            .data    = {
                       .v1 = {
                               .name = name,
                    },
            },
    };
    WUPSConfigCategoryHandle outCat;
    if (WUPSConfigAPIBackend::Category::Create(options, &outCat) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return -1;
    }
    *out = outCat.handle;

    return 0;
}

int32_t WUPSConfigCategory_Destroy(void *handle) {
    if (WUPSConfigAPIBackend::Category::Destroy(WUPSConfigCategoryHandle(handle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return -1;
    }

    return 0;
}

int32_t WUPSConfigCategory_GetName(void *handle, char *out_buf, int32_t out_len) {
    if (handle == nullptr || out_buf == nullptr || out_len == 0) {
        DEBUG_FUNCTION_LINE("Invalid param: \"handle\" or \"out_buf\" was NULL");
        return -1;
    }

    auto *category = WUPSConfigAPIBackend::Intern::GetCategoryByHandle(WUPSConfigCategoryHandle(handle), true);
    if (!category) {
        DEBUG_FUNCTION_LINE_WARN("Failed to find existing category for handle %08X", handle);
        return -2;
    }
    snprintf(out_buf, out_len, "%s", category->getName().c_str());
    return 0;
}

int32_t WUPSConfigCategory_AddItem(void *handle, void *item_Handle) {
    if (WUPSConfigAPIBackend::Category::AddItem(WUPSConfigCategoryHandle(handle), WUPSConfigItemHandle(item_Handle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return -1;
    }

    return 0;
}

WUMS_EXPORT_FUNCTION(WUPSConfigCategory_Create);
WUMS_EXPORT_FUNCTION(WUPSConfigCategory_Destroy);
WUMS_EXPORT_FUNCTION(WUPSConfigCategory_GetName);
WUMS_EXPORT_FUNCTION(WUPSConfigCategory_AddItem);

int32_t WUPSConfigItem_Create(void **out, const char *configId, const char *displayName, WUPSConfigAPIItemCallbacksV1 callbacks, void *context) {
    if (out == nullptr) {
        return -1;
    }
    *out                                   = nullptr;
    WUPSConfigAPICreateItemOptions options = {
            .version = WUPS_API_ITEM_OPTION_VERSION_V1,
            .data    = {
                       .v1 = {
                               .configId    = configId,
                               .displayName = displayName,
                               .context     = context,
                               .callbacks   = callbacks,
                    },
            },
    };
    WUPSConfigItemHandle outItem;
    if (WUPSConfigAPIBackend::Item::Create(options, &outItem) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return -1;
    }
    *out = outItem.handle;
    return 0;
}

int32_t WUPSConfigItem_Destroy(void *handle) {
    if (WUPSConfigAPIBackend::Item::Destroy(WUPSConfigItemHandle(handle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return -1;
    }
    return 0;
}

int32_t WUPSConfigItem_SetDisplayName(void *handle, const char *displayName) {
    if (displayName == nullptr || handle == nullptr) {
        DEBUG_FUNCTION_LINE("Invalid param: \"handle\" or \"displayName\" was NULL");
        return -1;
    }

    auto *config = WUPSConfigAPIBackend::Intern::GetItemByHandle(WUPSConfigItemHandle(handle));
    if (!config) {
        DEBUG_FUNCTION_LINE_ERR("Failed to find item for handle %08X", handle);
        return -2;
    }
    config->setDisplayName(displayName);
    return 0;
}

int32_t WUPSConfigItem_GetDisplayName(void *handle, char *out_buf, int32_t out_len) {
    if (handle == nullptr || out_buf == nullptr) {
        DEBUG_FUNCTION_LINE("Invalid param: \"handle\" or \"out_buf\" was NULL");
        return -1;
    }
    auto *config = WUPSConfigAPIBackend::Intern::GetItemByHandle(WUPSConfigItemHandle(handle));
    if (!config) {
        DEBUG_FUNCTION_LINE_ERR("Failed to find item for handle %08X", handle);
        return -2;
    }
    snprintf(out_buf, out_len, "%s", config->getDisplayName().c_str());
    return 0;
}

int32_t WUPSConfigItem_SetConfigID(void *handle, const char *configId) {
    if (handle == nullptr || configId == nullptr) {
        DEBUG_FUNCTION_LINE("Invalid param: \"handle\" or \"configId\" was NULL");
        return -1;
    }

    auto *config = WUPSConfigAPIBackend::Intern::GetItemByHandle(WUPSConfigItemHandle(handle));
    if (!config) {
        DEBUG_FUNCTION_LINE_ERR("Failed to find item for handle %08X", handle);
        return -2;
    }
    config->setConfigId(configId);
    return 0;
}

int32_t WUPSConfigItem_GetConfigID(void *handle, char *out_buf, int32_t out_len) {
    if (handle == nullptr || out_buf == nullptr) {
        DEBUG_FUNCTION_LINE("Invalid param: \"handle\" or \"out_buf\" was NULL");
        return -1;
    }
    auto *config = WUPSConfigAPIBackend::Intern::GetItemByHandle(WUPSConfigItemHandle(handle));
    if (!config) {
        DEBUG_FUNCTION_LINE_ERR("Failed to find item for handle %08X", handle);
        return -2;
    }
    snprintf(out_buf, out_len, "%s", config->getConfigId().c_str());
    return 0;
}

WUMS_EXPORT_FUNCTION(WUPSConfigItem_Create);
WUMS_EXPORT_FUNCTION(WUPSConfigItem_Destroy);
WUMS_EXPORT_FUNCTION(WUPSConfigItem_SetDisplayName);
WUMS_EXPORT_FUNCTION(WUPSConfigItem_GetDisplayName);
WUMS_EXPORT_FUNCTION(WUPSConfigItem_SetConfigID);
WUMS_EXPORT_FUNCTION(WUPSConfigItem_GetConfigID);