#pragma once
#include "config/WUPSConfig.h"
#include "plugin/PluginData.h"
#include <memory>

struct GeneralConfigInformation {
    std::string name;
    std::string author;
    std::string version;
    std::shared_ptr<PluginData> pluginData;
};

class ConfigDisplayItem {
public:
    ConfigDisplayItem(GeneralConfigInformation &info, std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> config, bool isActive) : mConfig(std::move(config)), mInfo(std::move(info)), mIsActivePlugin(isActive) {
        assert(mConfig);
    }
    [[nodiscard]] const GeneralConfigInformation &getConfigInformation() const {
        return mInfo;
    }
    [[nodiscard]] const WUPSConfigAPIBackend::WUPSConfig &getConfig() const {
        return *mConfig;
    }

    [[nodiscard]] bool isActivePlugin() const {
        return mIsActivePlugin;
    }

    void toggleIsActivePlugin() {
        mIsActivePlugin = !mIsActivePlugin;
    }

private:
    std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> mConfig;
    GeneralConfigInformation mInfo;
    bool mIsActivePlugin;
};