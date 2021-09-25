#include <wums.h>
#include <wups/config.h>
#include "WUPSConfig.h"
#include "../utils/logger.h"

int32_t WUPSConfig_Create(WUPSConfigHandle *out, const char *name) {
    if (name == nullptr || out == nullptr) {
        return -1;
    }

    *out = (WUPSConfigHandle) new WUPSConfig(name);
    if (*out != 0) {
        return 0;
    }
    return -2;
};

int32_t WUPSConfig_Destroy(WUPSConfigHandle handle) {
    if (handle == 0) {
        return -1;
    }

    auto *config = reinterpret_cast<WUPSConfig *>(handle);
    delete config;
    return 0;
};

int32_t WUPSConfig_GetName(WUPSConfigHandle handle, char *out_buf, int32_t out_len) {
    if (out_buf == nullptr) {
        return -1;
    }
    auto *config = reinterpret_cast<WUPSConfig *>(handle);
    snprintf(out_buf, out_len, "%s", config->getName().c_str());
    DEBUG_FUNCTION_LINE("%s", out_buf);
    return 0;
}

int32_t WUPSConfig_AddCategoryByName(WUPSConfigHandle handle, const char *categoryName, WUPSConfigCategoryHandle *out) {
    if (categoryName == nullptr) {
        return -1;
    }
    auto *config = reinterpret_cast<WUPSConfig *>(handle);
    auto res = config->addCategory(std::string(categoryName));
    if (res.has_value()) {
        if (out != nullptr) {
            *out = reinterpret_cast<WUPSConfigCategoryHandle>(res.value());
        } else {
            return -3;
        }
        return 0;
    }
    return -2;
}

int32_t WUPSConfig_AddCategory(WUPSConfigHandle handle, WUPSConfigCategoryHandle category) {
    auto *config = reinterpret_cast<WUPSConfig *>(handle);
    auto res = config->addCategory(reinterpret_cast<WUPSConfigCategory *>(category));
    if (res == nullptr) {
        return -1;
    }
    return 0;
}
/*
int32_t WUPSConfig_GetCategoryCount(WUPSConfigHandle handle, int32_t *category_count) {
    if (category_count == nullptr) {
        return -1;
    }
    auto *config = reinterpret_cast<WUPSConfig *>(handle);
    *category_count = config->getCategories().size();
    return 0;
}

int32_t WUPSConfig_GetCategories(WUPSConfigHandle handle, WUPSConfigCategoryHandle *categories_out, int32_t categories_out_size) {
    if (categories_out == nullptr || categories_out_size == 0) {
        return -1;
    }
    auto *config = reinterpret_cast<WUPSConfig *>(handle);
    auto cats = config->getCategories();
    int32_t index = 0;
    for (auto const &cat: cats) {
        if (index >= categories_out_size) {
            break;
        }
        categories_out[index] = (WUPSConfigCategoryHandle) cat;
    }

    return 0;
}*/

WUMS_EXPORT_FUNCTION(WUPSConfig_Create);
WUMS_EXPORT_FUNCTION(WUPSConfig_Destroy);
WUMS_EXPORT_FUNCTION(WUPSConfig_GetName);
WUMS_EXPORT_FUNCTION(WUPSConfig_AddCategoryByName);
WUMS_EXPORT_FUNCTION(WUPSConfig_AddCategory);
/*
WUMS_EXPORT_FUNCTION(WUPSConfig_GetCategoryCount);
WUMS_EXPORT_FUNCTION(WUPSConfig_GetCategories);*/