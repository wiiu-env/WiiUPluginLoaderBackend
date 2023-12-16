#pragma once

#include "config/WUPSConfigAPI.h"
#include <optional>
#include <string>
#include <wups/config_api.h>

class PluginConfigData {
public:
    PluginConfigData(std::string_view name,
                     WUPSConfigAPI_MenuOpenedCallback openedCallback,
                     WUPSConfigAPI_MenuClosedCallback closedCallback) : mName(name),
                                                                        mOpenedCallback(openedCallback),
                                                                        mClosedCallback(closedCallback) {
    }

    std::optional<WUPSConfigHandle> createConfig() {
        WUPSConfigHandle handle;
        if (WUPSConfigAPIBackend::Intern::CreateConfig(mName.c_str(), &handle) == WUPSCONFIG_API_RESULT_SUCCESS) {
            return handle;
        }
        return std::nullopt;
    }

    WUPSConfigAPIStatus CallMenuOpenendCallback(WUPSConfigHandle config) {
        if (mOpenedCallback == nullptr) {
            return WUPSCONFIG_API_RESULT_MISSING_CALLBACK;
        }
        if (mOpenedCallback(WUPSConfigCategoryHandle(config.handle)) != WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS) {
            return WUPSCONFIG_API_RESULT_UNKNOWN_ERROR;
        }
        return WUPSCONFIG_API_RESULT_SUCCESS;
    }

    WUPSConfigAPIStatus CallMenuClosedCallback() {
        if (mClosedCallback == nullptr) {
            return WUPSCONFIG_API_RESULT_MISSING_CALLBACK;
        }
        mClosedCallback();
        return WUPSCONFIG_API_RESULT_SUCCESS;
    }

    static std::optional<PluginConfigData> create(WUPSConfigAPIOptions options, WUPSConfigAPI_MenuOpenedCallback openedCallback, WUPSConfigAPI_MenuClosedCallback closedCallback) {
        if (options.version != 1) {
            return std::nullopt;
        }
        PluginConfigData result(options.data.v1.name, openedCallback, closedCallback);

        return result;
    }

private:
    std::string mName;
    WUPSConfigAPI_MenuOpenedCallback mOpenedCallback;
    WUPSConfigAPI_MenuClosedCallback mClosedCallback;
};
