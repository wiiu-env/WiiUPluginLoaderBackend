#pragma once

#include "WUPSConfigItem.h"

#include <wups/config.h>

#include <string>

namespace WUPSConfigAPIBackend {
    class WUPSConfigItemV2 final : public WUPSConfigItem {
    public:
        WUPSConfigItemV2(std::string_view displayName, const WUPSConfigAPIItemCallbacksV2 &callbacks, void *context);

        ~WUPSConfigItemV2() override;

        [[nodiscard]] std::string getCurrentValueDisplay() const override;

        [[nodiscard]] std::string getCurrentValueSelectedDisplay() const override;

        void onSelected(bool isSelected) const override;

        void onInput(WUPSConfigSimplePadData input) const override;

        void onInputEx(WUPSConfigComplexPadData input) const override;

        [[nodiscard]] bool isMovementAllowed() const override;

        void restoreDefault() const override;

        void onCloseCallback() override;

    private:
        void *mContext;
        std::string mDefaultValue;
        WUPSConfigAPIItemCallbacksV2 mCallbacks{};
    };
} // namespace WUPSConfigAPIBackend