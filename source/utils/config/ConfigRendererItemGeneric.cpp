#include "ConfigRendererItemGeneric.h"

#include "utils/DrawUtils.h"

#include <string>
#include <wups/config.h>

ConfigRendererItemGeneric::~ConfigRendererItemGeneric() = default;

void ConfigRendererItemGeneric::drawGenericBoxAndText(const uint32_t yOffset, const std::string &displayName, const bool isHighlighted) const {
    if (isHighlighted) {
        DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 4, COLOR_BORDER_HIGHLIGHTED);
    } else {
        DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 2, COLOR_BORDER);
    }

    DrawUtils::setFontSize(24);
    DrawUtils::setFontColor(COLOR_TEXT);
    DrawUtils::print(16 * 2, yOffset + 8 + 24, displayName.c_str());
}

void ConfigRendererItemGeneric::SetIsSelected(bool) {
}

void ConfigRendererItemGeneric::OnButtonPressed(WUPSConfigButtons) {
}

void ConfigRendererItemGeneric::OnInput(WUPSConfigSimplePadData) {
}

void ConfigRendererItemGeneric::OnInputEx(WUPSConfigComplexPadData) {
}

bool ConfigRendererItemGeneric::IsMovementAllowed() const {
    return true;
}