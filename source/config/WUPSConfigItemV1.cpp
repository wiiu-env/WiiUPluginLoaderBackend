#include "WUPSConfigItemV1.h"
#include "utils/StringTools.h"
#include "utils/logger.h"

namespace WUPSConfigAPIBackend {
    WUPSConfigItemV1::WUPSConfigItemV1(const std::string_view configId,
                                       const std::string_view displayName,
                                       const WUPSConfigAPIItemCallbacksV1 &callbacks,
                                       void *context) : WUPSConfigItem(std::string(displayName)) {
        this->mConfigId     = configId;
        this->mContext      = context;
        this->mCallbacks    = callbacks;
        this->mDefaultValue = getCurrentValueDisplayImpl();
    }

    WUPSConfigItemV1::~WUPSConfigItemV1() {
        if (this->mCallbacks.onDelete == nullptr) {
            DEBUG_FUNCTION_LINE_WARN("onDelete callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        this->mCallbacks.onDelete(mContext);
    }

    std::string WUPSConfigItemV1::getCurrentValueDisplayImpl() const {
        if (this->mCallbacks.getCurrentValueDisplay == nullptr) {
            DEBUG_FUNCTION_LINE_ERR("getCurrentValueDisplay callback not implemented. [%s]", mDisplayName.c_str());
            return "NOT_IMPLEMENTED";
        }
        char buf[80];
        int res = this->mCallbacks.getCurrentValueDisplay(mContext, buf, sizeof(buf));
        if (res != 0) {
            return string_format("[ERROR %d]", res);
        }
        return buf;
    }

    std::string WUPSConfigItemV1::getCurrentValueDisplay() const {
        return getCurrentValueDisplayImpl();
    }

    std::string WUPSConfigItemV1::getCurrentValueSelectedDisplay() const {
        if (this->mCallbacks.getCurrentValueSelectedDisplay == nullptr) {
            DEBUG_FUNCTION_LINE_ERR("getCurrentValueSelectedDisplay callback not implemented. [%s]", mDisplayName.c_str());
            return "NOT_IMPLEMENTED";
        }
        char buf[80];
        int res = this->mCallbacks.getCurrentValueSelectedDisplay(mContext, buf, sizeof(buf));
        if (res != 0) {
            return string_format("[ERROR %d]", res);
        }
        return buf;
    }

    void WUPSConfigItemV1::onSelected(const bool isSelected) const {
        if (this->mCallbacks.onSelected == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("onSelected callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        this->mCallbacks.onSelected(mContext, isSelected);
    }

    void WUPSConfigItemV1::onButtonPressed(const WUPSConfigButtons buttons) const {
        if (this->mCallbacks.onButtonPressed == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("onButtonPressed callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        this->mCallbacks.onButtonPressed(mContext, buttons);
    }

    bool WUPSConfigItemV1::isMovementAllowed() const {
        if (this->mCallbacks.isMovementAllowed == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("isMovementAllowed callback not implemented. [%s]", mDisplayName.c_str());
            return true;
        }
        return this->mCallbacks.isMovementAllowed(mContext);
    }

    void WUPSConfigItemV1::restoreDefault() const {
        if (this->mCallbacks.restoreDefault == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("restoreDefault callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        this->mCallbacks.restoreDefault(mContext);
    }

    void WUPSConfigItemV1::onCloseCallback() {
        if (this->mCallbacks.callCallback == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("callCallback callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        if (mDefaultValue != getCurrentValueDisplay()) {
            this->mCallbacks.callCallback(mContext);
        }
    }

    void WUPSConfigItemV1::setConfigId(const std::string &configId) {
        mConfigId = configId;
    }

    const std::string &WUPSConfigItemV1::getConfigId() {
        return mConfigId;
    }
} // namespace WUPSConfigAPIBackend