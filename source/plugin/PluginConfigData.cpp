#include "PluginConfigData.h"
#include "config/WUPSConfigAPI.h"
#include "plugin/PluginData.h"

PluginConfigData::PluginConfigData(const std::string_view name,
                                   const WUPSConfigAPI_MenuOpenedCallback openedCallback,
                                   const WUPSConfigAPI_MenuClosedCallback closedCallback) : mName(name),
                                                                                            mOpenedCallback(openedCallback),
                                                                                            mClosedCallback(closedCallback) {
}

std::optional<WUPSConfigHandle> PluginConfigData::createConfig() const {
    WUPSConfigHandle handle;
    if (WUPSConfigAPIBackend::Intern::CreateConfig(mName.c_str(), &handle) == WUPSCONFIG_API_RESULT_SUCCESS) {
        return handle;
    }
    return std::nullopt;
}

WUPSConfigAPIStatus PluginConfigData::CallMenuOpenedCallback(const WUPSConfigHandle config) const {
    if (mOpenedCallback == nullptr) {
        return WUPSCONFIG_API_RESULT_MISSING_CALLBACK;
    }
    if (mOpenedCallback(WUPSConfigCategoryHandle(config.handle)) != WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS) {
        return WUPSCONFIG_API_RESULT_UNKNOWN_ERROR;
    }
    return WUPSCONFIG_API_RESULT_SUCCESS;
}

WUPSConfigAPIStatus PluginConfigData::CallMenuClosedCallback() const {
    if (mClosedCallback == nullptr) {
        return WUPSCONFIG_API_RESULT_MISSING_CALLBACK;
    }
    mClosedCallback();
    return WUPSCONFIG_API_RESULT_SUCCESS;
}

std::optional<PluginConfigData> PluginConfigData::create(const WUPSConfigAPIOptions options,
                                                         const WUPSConfigAPI_MenuOpenedCallback openedCallback,
                                                         const WUPSConfigAPI_MenuClosedCallback closedCallback) {
    if (options.version != 1) {
        return std::nullopt;
    }
    PluginConfigData result(options.data.v1.name, openedCallback, closedCallback);

    return result;
}
