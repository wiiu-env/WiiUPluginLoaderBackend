#pragma once
#include "config/WUPSConfig.h"
#include <memory>

struct GeneralConfigInformation {
    std::string name;
    std::string author;
    std::string version;
};

class ConfigDisplayItem {
public:
    ConfigDisplayItem(GeneralConfigInformation &info, std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> config) : mConfig(std::move(config)), mInfo(std::move(info)) {
        assert(mConfig);
    }
    [[nodiscard]] const GeneralConfigInformation &getConfigInformation() const {
        return mInfo;
    }
    [[nodiscard]] const WUPSConfigAPIBackend::WUPSConfig &getConfig() const {
        return *mConfig;
    }

private:
    std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> mConfig;
    GeneralConfigInformation mInfo;
};