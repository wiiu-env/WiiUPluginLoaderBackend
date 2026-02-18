#include "ConfigDisplayItem.h"

#include "config/WUPSConfig.h"

#include <cassert>
#include <memory>

ConfigDisplayItem::ConfigDisplayItem(GeneralConfigInformation &info,
                                     std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> config,
                                     const bool isActive,
                                     const bool isHeapTracking) : mConfig(std::move(config)),
                                                                  mInfo(std::move(info)),
                                                                  mIsActivePlugin(isActive),
                                                                  mInitialIsActivePlugin(isActive),
                                                                  mIsHeapTrackingEnabled(isHeapTracking),
                                                                  mInitialIsHeapTrackingEnabled(isHeapTracking) {
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

bool ConfigDisplayItem::isHeapTrackingEnabled() const {
    return mIsHeapTrackingEnabled;
}

void ConfigDisplayItem::toggleIsHeapTrackingEnabled() {
    mIsHeapTrackingEnabled = !mIsHeapTrackingEnabled;
}

void ConfigDisplayItem::resetIsHeapTrackingEnabled() {
    mIsHeapTrackingEnabled = mInitialIsHeapTrackingEnabled;
}