#include <wums.h>
#include <wups/config.h>
#include "WUPSConfigCategory.h"
#include "../utils/logger.h"

int32_t WUPSConfigCategory_Create(WUPSConfigCategoryHandle *out, const char *name) {
    if (name == nullptr || out == nullptr) {
        return -1;
    }

    *out = (WUPSConfigCategoryHandle) new WUPSConfigCategory(name);
    if (*out != 0) {
        return 0;
    }
    return -2;
};

int32_t WUPSConfigCategory_Destroy(WUPSConfigCategoryHandle handle) {
    if (handle == 0) {
        return -1;
    }

    auto *config = reinterpret_cast<WUPSConfigCategory *>(handle);
    delete config;
    return 0;
};

int32_t WUPSConfigCategory_GetName(WUPSConfigCategoryHandle handle, char *out_buf, int32_t out_len) {
    if (out_buf == nullptr) {
        return -1;
    }
    auto *config = reinterpret_cast<WUPSConfigCategory *>(handle);
    snprintf(out_buf, out_len, "%s", config->getName().c_str());
    return 0;
}

int32_t WUPSConfigCategory_AddItem(WUPSConfigCategoryHandle handle, WUPSConfigItemHandle item_Handle) {
    if (handle == 0 || item_Handle == 0) {
        return -1;
    }
    auto *category = reinterpret_cast<WUPSConfigCategory *>(handle);
    auto *item = reinterpret_cast<WUPSConfigItem *>(item_Handle);
    if (category->addItem(item)) {
        return 0;
    }
    return -2;
}
/*
int32_t WUPSConfigCategory_GetItemCount(WUPSConfigCategoryHandle handle, int32_t *item_count) {
    if (handle == 0 || item_count == nullptr) {
        return -1;
    }
    auto *config = reinterpret_cast<WUPSConfigCategory *>(handle);
    *item_count = config->getItems().size();
    return 0;
}

int32_t WUPSConfigCategory_GetItems(WUPSConfigCategoryHandle handle, WUPSConfigItemHandle *items_out, int32_t items_out_size) {
    if (handle == 0 || items_out == nullptr || items_out_size == 0) {
        return -1;
    }
    auto *config = reinterpret_cast<WUPSConfigCategory *>(handle);
    auto items = config->getItems();
    int32_t index = 0;
    for (auto const &item: items) {
        if (index >= items_out_size) {
            break;
        }
        items_out[index] = (WUPSConfigItemHandle) item;
    }

    return 0;
}*/

WUMS_EXPORT_FUNCTION(WUPSConfigCategory_Create);
WUMS_EXPORT_FUNCTION(WUPSConfigCategory_Destroy);
WUMS_EXPORT_FUNCTION(WUPSConfigCategory_GetName);
WUMS_EXPORT_FUNCTION(WUPSConfigCategory_AddItem);
/*
WUMS_EXPORT_FUNCTION(WUPSConfigCategory_GetItemCount);
WUMS_EXPORT_FUNCTION(WUPSConfigCategory_GetItems);*/