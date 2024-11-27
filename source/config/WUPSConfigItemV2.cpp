#include "WUPSConfigItemV2.h"
#include "utils/StringTools.h"
#include "utils/logger.h"

#include <string>
#include <wups/config.h>

namespace WUPSConfigAPIBackend {
    WUPSConfigItemV2::WUPSConfigItemV2(const std::string_view displayName,
                                       const WUPSConfigAPIItemCallbacksV2 &callbacks,
                                       void *context) : WUPSConfigItem(std::string(displayName)) {
        this->mDisplayName = displayName;
        this->mContext     = context;
        this->mCallbacks   = callbacks;
    }

    WUPSConfigItemV2::~WUPSConfigItemV2() {
        if (this->mCallbacks.onDelete == nullptr) {
            DEBUG_FUNCTION_LINE_WARN("onDelete callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        this->mCallbacks.onDelete(mContext);
    }

    void WUPSConfigItemV2::onSelected(const bool isSelected) const {
        if (this->mCallbacks.onSelected == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("onSelected callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        this->mCallbacks.onSelected(mContext, isSelected);
    }

    std::string WUPSConfigItemV2::getCurrentValueDisplay() const {
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

    std::string WUPSConfigItemV2::getCurrentValueSelectedDisplay() const {
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

    void WUPSConfigItemV2::onInput(const WUPSConfigSimplePadData input) const {
        if (this->mCallbacks.onInput == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("onInput callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        this->mCallbacks.onInput(mContext, input);
    }

    void WUPSConfigItemV2::onInputEx(const WUPSConfigComplexPadData input) const {
        if (this->mCallbacks.onInputEx == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("onInputEx callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        this->mCallbacks.onInputEx(mContext, input);
    }

    bool WUPSConfigItemV2::isMovementAllowed() const {
        if (this->mCallbacks.isMovementAllowed == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("isMovementAllowed callback not implemented. [%s]", mDisplayName.c_str());
            return true;
        }
        return this->mCallbacks.isMovementAllowed(mContext);
    }

    void WUPSConfigItemV2::restoreDefault() const {
        if (this->mCallbacks.restoreDefault == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("restoreDefault callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        this->mCallbacks.restoreDefault(mContext);
    }

    void WUPSConfigItemV2::onCloseCallback() {
        if (this->mCallbacks.onCloseCallback == nullptr) {
            DEBUG_FUNCTION_LINE_VERBOSE("onCloseCallback callback not implemented. [%s]", mDisplayName.c_str());
            return;
        }
        this->mCallbacks.onCloseCallback(mContext);
    }
} // namespace WUPSConfigAPIBackend