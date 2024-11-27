#pragma once

#include "ConfigRendererItemGeneric.h"
#include "config/WUPSConfigItem.h"

#include <cstdint>
#include <string>
#include <wups/config.h>

class ConfigRendererItem final : public ConfigRendererItemGeneric {
public:
    explicit ConfigRendererItem(const WUPSConfigAPIBackend::WUPSConfigItem *item);

    void Draw(uint32_t yOffset, bool isHighlighted) const override;

    [[nodiscard]] std::string GetValueToPrint(bool isHighlighted) const;

    void Update(bool isHighlighted) override;

    void ResetNeedsRedraw() override;

    [[nodiscard]] bool NeedsRedraw() const override;

    void SetIsSelected(bool isSelected) override;

    void OnButtonPressed(WUPSConfigButtons buttons) override;

    [[nodiscard]] bool IsMovementAllowed() const override;

    void OnInput(WUPSConfigSimplePadData input) override;

    void OnInputEx(WUPSConfigComplexPadData input) override;

private:
    const WUPSConfigAPIBackend::WUPSConfigItem *mItem = nullptr;
    std::string mCurItemText;
    bool mNeedsDraw = true;
};