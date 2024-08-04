#pragma once

#include "config/WUPSConfig.h"
#include "plugin/PluginData.h"
#include <memory>
#include <string>

struct GeneralConfigInformation {
    std::string name;
    std::string author;
    std::string version;
    std::shared_ptr<PluginData> pluginData;
};

class ConfigDisplayItem {
public:
    ConfigDisplayItem(GeneralConfigInformation &info, std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> config, bool isActive);

    [[nodiscard]] const GeneralConfigInformation &getConfigInformation() const;

    [[nodiscard]] const WUPSConfigAPIBackend::WUPSConfig &getConfig() const;

    [[nodiscard]] bool isActivePlugin() const;

    void toggleIsActivePlugin();

    void resetIsActivePlugin();

private:
    std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> mConfig;
    GeneralConfigInformation mInfo;
    bool mIsActivePlugin;
    bool mInitialIsActivePlugin;
};