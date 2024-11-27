#include "WUPSConfigItem.h"

namespace WUPSConfigAPIBackend {
    WUPSConfigItem::WUPSConfigItem(std::string displayName) : mDisplayName(std::move(displayName)) {
    }
    WUPSConfigItem::~WUPSConfigItem() = default;

    void WUPSConfigItem::onButtonPressed(WUPSConfigButtons) const {}

    const std::string &WUPSConfigItem::getDisplayName() const {
        return mDisplayName;
    }

    void WUPSConfigItem::setConfigId(const std::string &) {}

    const std::string &WUPSConfigItem::getConfigId() {
        return mStubConfigId;
    }

    void WUPSConfigItem::setDisplayName(std::string displayName) {
        mDisplayName = std::move(displayName);
    }

    void WUPSConfigItem::onInput(WUPSConfigSimplePadData) const {}

    void WUPSConfigItem::onInputEx(WUPSConfigComplexPadData) const {}
} // namespace WUPSConfigAPIBackend