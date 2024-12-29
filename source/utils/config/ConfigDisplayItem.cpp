#include "ConfigDisplayItem.h"

#include "config/WUPSConfig.h"

#include <memory>

#include <cassert>

ConfigDisplayItem::ConfigDisplayItem(GeneralConfigInformation &info,
                                     std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> config,
                                     const bool isActive) : mConfig(std::move(config)),
                                                            mInfo(std::move(info)),
                                                            mIsActivePlugin(isActive),
                                                            mInitialIsActivePlugin(isActive) {
    assert(mConfig);
}

const GeneralConfigInformation &ConfigDisplayItem::getConfigInformation() const {
    return mInfo;
}

const WUPSConfigAPIBackend::WUPSConfig &ConfigDisplayItem::getConfig() const {
    return *mConfig;
}

bool ConfigDisplayItem::isActivePlugin() const {
    return mIsActivePlugin;
}

void ConfigDisplayItem::toggleIsActivePlugin() {
    mIsActivePlugin = !mIsActivePlugin;
}

void ConfigDisplayItem::resetIsActivePlugin() {
    mIsActivePlugin = mInitialIsActivePlugin;
}