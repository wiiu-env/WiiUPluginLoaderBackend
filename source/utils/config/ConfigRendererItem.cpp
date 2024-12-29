#include "ConfigRendererItem.h"

#include "utils/DrawUtils.h"
#include <config/WUPSConfigItem.h>

#include <cassert>

ConfigRendererItem::ConfigRendererItem(const WUPSConfigAPIBackend::WUPSConfigItem *item) : mItem(item) {
    assert(item);
}

void ConfigRendererItem::Draw(const uint32_t yOffset, const bool isHighlighted) const {
    assert(mItem);
    drawGenericBoxAndText(yOffset, mItem->getDisplayName(), isHighlighted);
    DrawUtils::setFontSize(24);
    DrawUtils::print(SCREEN_WIDTH - 16 * 2, yOffset + 8 + 24, mCurItemText.c_str(), true);
}

std::string ConfigRendererItem::GetValueToPrint(const bool isHighlighted) const {
    return isHighlighted ? mItem->getCurrentValueSelectedDisplay() : mItem->getCurrentValueDisplay();
}

void ConfigRendererItem::Update(const bool isHighlighted) {
    const auto newText = GetValueToPrint(isHighlighted);

    if (mCurItemText != newText) {
        mNeedsDraw = true;
    }
    mCurItemText = newText;
}

void ConfigRendererItem::ResetNeedsRedraw() {
    mNeedsDraw = false;
}

[[nodiscard]] bool ConfigRendererItem::NeedsRedraw() const {
    return mNeedsDraw;
}

void ConfigRendererItem::SetIsSelected(const bool isSelected) {
    mItem->onSelected(isSelected);
}

void ConfigRendererItem::OnButtonPressed(const WUPSConfigButtons buttons) {
    mItem->onButtonPressed(buttons);
}

[[nodiscard]] bool ConfigRendererItem::IsMovementAllowed() const {
    return mItem->isMovementAllowed();
}

void ConfigRendererItem::OnInput(const WUPSConfigSimplePadData input) {
    mItem->onInput(input);
}

void ConfigRendererItem::OnInputEx(const WUPSConfigComplexPadData input) {
    mItem->onInputEx(input);
}