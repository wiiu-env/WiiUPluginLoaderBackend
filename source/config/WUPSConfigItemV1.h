#pragma once

#include "WUPSConfigItem.h"

#include <wups/config.h>

#include <string>
namespace WUPSConfigAPIBackend {
    class WUPSConfigItemV1 final : public WUPSConfigItem {
    public:
        WUPSConfigItemV1(std::string_view configId,
                         std::string_view displayName,
                         const WUPSConfigAPIItemCallbacksV1 &callbacks,
                         void *context);

        ~WUPSConfigItemV1() override;

        [[nodiscard]] std::string getCurrentValueDisplayImpl() const;

        [[nodiscard]] std::string getCurrentValueDisplay() const override;

        [[nodiscard]] std::string getCurrentValueSelectedDisplay() const override;

        void onSelected(bool isSelected) const override;

        void onButtonPressed(WUPSConfigButtons buttons) const override;

        [[nodiscard]] bool isMovementAllowed() const override;

        void restoreDefault() const override;

        void onCloseCallback() override;

        void setConfigId(const std::string &configId) override;

        const std::string &getConfigId() override;

    private:
        void *mContext;
        std::string mConfigId;
        std::string mDefaultValue;
        WUPSConfigAPIItemCallbacksV1 mCallbacks{};
    };
} // namespace WUPSConfigAPIBackend