#include <wums.h>
#include <wups/config.h>
#include "WUPSConfigItem.h"

typedef uint32_t WUPSConfigItemHandle;

int32_t WUPSConfigItem_Create(WUPSConfigItemHandle *out, const char *configID, const char *displayName, WUPSConfigCallbacks_t callbacks, void* context) {
    if (configID == nullptr || displayName == nullptr) {
        return -1;
    }

    *out = (WUPSConfigItemHandle) new WUPSConfigItem(configID, displayName, callbacks, context);
    if (*out != 0) {
        return 0;
    }
    return -2;
};

int32_t WUPSConfigItem_Destroy(WUPSConfigItemHandle handle) {
    if (handle == 0) {
        return -1;
    }

    auto *config = reinterpret_cast<WUPSConfigItem *>(handle);
    delete config;
    return 0;
};

int32_t WUPSConfigItem_SetDisplayName(WUPSConfigItemHandle handle, const char *displayName) {
    if (displayName == nullptr) {
        return -1;
    }

    auto *config = reinterpret_cast<WUPSConfigItem *>(handle);
    config->setDisplayName(displayName);
    return 0;
};

int32_t WUPSConfigItem_GetDisplayName(WUPSConfigItemHandle handle, char *out_buf, int32_t out_len) {
    if (out_buf == nullptr) {
        return -1;
    }
    auto *config = reinterpret_cast<WUPSConfigItem *>(handle);
    snprintf(out_buf, out_len, "%s", config->getDisplayName().c_str());
    return 0;
}

int32_t WUPSConfigItem_SetConfigID(WUPSConfigItemHandle handle, const char *configID) {
    if (configID == nullptr) {
        return -1;
    }

    auto *config = reinterpret_cast<WUPSConfigItem *>(handle);
    config->setConfigID(configID);
    return 0;
};

int32_t WUPSConfigItem_GetConfigID(WUPSConfigItemHandle handle, char *out_buf, int32_t out_len) {
    if (out_buf == nullptr) {
        return -1;
    }
    auto *config = reinterpret_cast<WUPSConfigItem *>(handle);
    snprintf(out_buf, out_len, "%s", config->getConfigID().c_str());
    return 0;
}

WUMS_EXPORT_FUNCTION(WUPSConfigItem_Create);
WUMS_EXPORT_FUNCTION(WUPSConfigItem_Destroy);
WUMS_EXPORT_FUNCTION(WUPSConfigItem_SetDisplayName);
WUMS_EXPORT_FUNCTION(WUPSConfigItem_GetDisplayName);
WUMS_EXPORT_FUNCTION(WUPSConfigItem_SetConfigID);
WUMS_EXPORT_FUNCTION(WUPSConfigItem_GetConfigID);