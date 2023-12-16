#pragma once
#include "../DrawUtils.h"
#include "ConfigDefines.h"
#include <wups/config.h>

class ConfigRendererItemGeneric {
public:
    virtual ~ConfigRendererItemGeneric() = default;
    virtual void drawGenericBoxAndText(uint32_t yOffset, const std::string &displayName, bool isHighlighted) const {
        if (isHighlighted) {
            DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 4, COLOR_BORDER_HIGHLIGHTED);
        } else {
            DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 2, COLOR_BORDER);
        }

        DrawUtils::setFontSize(24);
        DrawUtils::setFontColor(COLOR_TEXT);
        DrawUtils::print(16 * 2, yOffset + 8 + 24, displayName.c_str());
    }

    virtual void Draw(uint32_t yOffset, bool isHighlighted) const = 0;

    virtual void SetIsSelected(bool) {
    }

    virtual void OnButtonPressed(WUPSConfigButtons) {
    }
    virtual void OnInput(WUPSConfigSimplePadData) {
    }
    virtual void OnInputEx(WUPSConfigComplexPadData) {
    }

    [[nodiscard]] virtual bool IsMovementAllowed() const {
        return true;
    }
};