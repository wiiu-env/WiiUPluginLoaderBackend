#pragma once
#include "ConfigRendererItemGeneric.h"
#include "config/WUPSConfigItem.h"

class ConfigRendererItem : public ConfigRendererItemGeneric {
public:
    explicit ConfigRendererItem(const WUPSConfigAPIBackend::WUPSConfigItem *item) : mItem(item) {
        assert(item);
    }

    void Draw(uint32_t yOffset, bool isHighlighted) const override {
        assert(mItem);
        drawGenericBoxAndText(yOffset, mItem->getDisplayName(), isHighlighted);
        DrawUtils::setFontSize(24);
        DrawUtils::print(SCREEN_WIDTH - 16 * 2, yOffset + 8 + 24, mCurItemText.c_str(), true);
    }

    std::string GetValueToPrint(bool isHighlighted) {
        return isHighlighted ? mItem->getCurrentValueSelectedDisplay() : mItem->getCurrentValueDisplay();
    }

    void Update(bool isHighlighted) override {
        const auto newText = GetValueToPrint(isHighlighted);

        if (mCurItemText != newText) {
            mNeedsDraw = true;
        }
        mCurItemText = newText;
    }

    void ResetNeedsRedraw() override {
        mNeedsDraw = false;
    }

    [[nodiscard]] bool NeedsRedraw() const override {
        return mNeedsDraw;
    }

    void SetIsSelected(bool isSelected) override {
        mItem->onSelected(isSelected);
    }

    void OnButtonPressed(WUPSConfigButtons buttons) override {
        mItem->onButtonPressed(buttons);
    }

    [[nodiscard]] bool IsMovementAllowed() const override {
        return mItem->isMovementAllowed();
    }

    void OnInput(WUPSConfigSimplePadData input) override {
        mItem->onInput(input);
    }
    void OnInputEx(WUPSConfigComplexPadData input) override {
        mItem->onInputEx(input);
    }

private:
    const WUPSConfigAPIBackend::WUPSConfigItem *mItem;
    std::string mCurItemText;
    bool mNeedsDraw = true;
};