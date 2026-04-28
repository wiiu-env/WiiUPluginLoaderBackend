#include "ConfigRendererItem.h"

#include "utils/DrawUtils.h"
#include "utils/logger.h"

#include <config/WUPSConfigItem.h>

#include <cassert>

namespace {
    std::string truncateTextToWidth(const std::string &str, uint32_t maxWidth) {
        if (DrawUtils::getTextWidth(str.c_str()) <= maxWidth) {
            return str;
        }

        std::string ellipsis         = "...";
        const uint32_t ellipsisWidth = DrawUtils::getTextWidth(ellipsis.c_str());

        if (maxWidth <= ellipsisWidth) {
            return ellipsis;
        }

        std::string result = str;
        while (!result.empty()) {
            result.pop_back(); // Remove one character from the end
            if (DrawUtils::getTextWidth((result + ellipsis).c_str()) <= maxWidth) {
                return result + ellipsis;
            }
        }

        return ellipsis;
    }
} // namespace

ConfigRendererItem::ConfigRendererItem(const WUPSConfigAPIBackend::WUPSConfigItem *item) : mItem(item) {
    assert(item);
}
void ConfigRendererItem::Draw(const uint32_t yOffset, const bool isHighlighted) const {
    assert(mItem);

    const auto displayName = mItem->getDisplayName();

    DrawUtils::setFontSize(24);

    const uint32_t widthValueText = DrawUtils::getTextWidth(mCurItemText.c_str());
    const uint32_t maxNameWidth   = (780 > widthValueText) ? (780 - widthValueText) : 0;

    const std::string renderItemName = truncateTextToWidth(displayName, maxNameWidth);

    // Draw the item
    drawGenericBoxAndText(yOffset, renderItemName, isHighlighted);
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