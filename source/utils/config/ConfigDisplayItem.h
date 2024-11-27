#pragma once

#include "config/WUPSConfig.h"
#include <memory>
#include <string>

struct GeneralConfigInformation {
    std::string name;
    std::string author;
    std::string version;
};

class ConfigDisplayItem {
public:
    ConfigDisplayItem(GeneralConfigInformation &info, std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> config);

    [[nodiscard]] const GeneralConfigInformation &getConfigInformation() const;

    [[nodiscard]] const WUPSConfigAPIBackend::WUPSConfig &getConfig() const;

private:
    std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> mConfig;
    GeneralConfigInformation mInfo;
};