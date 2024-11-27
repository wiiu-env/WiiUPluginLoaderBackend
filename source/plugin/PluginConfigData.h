#pragma once

#include <optional>
#include <string>
#include <wups/config.h>
#include <wups/config_api.h>

class PluginConfigData {
public:
    PluginConfigData(std::string_view name,
                     WUPSConfigAPI_MenuOpenedCallback openedCallback,
                     WUPSConfigAPI_MenuClosedCallback closedCallback);

    [[nodiscard]] std::optional<WUPSConfigHandle> createConfig() const;

    [[nodiscard]] WUPSConfigAPIStatus CallMenuOpenedCallback(WUPSConfigHandle config) const;

    [[nodiscard]] WUPSConfigAPIStatus CallMenuClosedCallback() const;

    static std::optional<PluginConfigData> create(WUPSConfigAPIOptions options, WUPSConfigAPI_MenuOpenedCallback openedCallback, WUPSConfigAPI_MenuClosedCallback closedCallback);

private:
    std::string mName;
    WUPSConfigAPI_MenuOpenedCallback mOpenedCallback;
    WUPSConfigAPI_MenuClosedCallback mClosedCallback;
};
