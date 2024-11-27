#include "ConfigDisplayItem.h"
#include "config/WUPSConfig.h"

ConfigDisplayItem::ConfigDisplayItem(GeneralConfigInformation &info, std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> config) : mConfig(std::move(config)), mInfo(std::move(info)) {
    assert(mConfig);
}
const GeneralConfigInformation &ConfigDisplayItem::getConfigInformation() const {
    return mInfo;
}
const WUPSConfigAPIBackend::WUPSConfig &ConfigDisplayItem::getConfig() const {
    return *mConfig;
}