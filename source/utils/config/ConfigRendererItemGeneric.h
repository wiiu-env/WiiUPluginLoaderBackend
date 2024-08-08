#pragma once
#include "../DrawUtils.h"
#include "ConfigDefines.h"
#include <wups/config.h>

namespace ConfigRenderItemFont {
    constexpr uint32_t FONT_SIZE                 = 24;
    constexpr uint32_t MIN_TEXT_WIDTH_FOR_SCROLL = 64;

    inline void IncrementOffset(std::string text, uint32_t &offset) {
        if (text.size() < MIN_TEXT_WIDTH_FOR_SCROLL) {
            offset = 0;
            return;
        }

        if (offset >= text.size()) {
            offset = 0;
            return;
        }

        text.erase(0, offset);
        if (text.size() < MIN_TEXT_WIDTH_FOR_SCROLL) {
            offset = 0;
            return;
        }

        offset++;
    }

    inline std::string GetOffsettedText(const std::string &text, uint32_t offset) {
        if (text.size() < MIN_TEXT_WIDTH_FOR_SCROLL || offset == 0) return text;

        std::string offsettedText = text;
        offsettedText.erase(0, offset);

        return offsettedText;
    }
} // namespace ConfigRenderItemFont

class ConfigRendererItemGeneric {
public:
    virtual ~ConfigRendererItemGeneric() = default;
    virtual void drawGenericBoxAndText(uint32_t yOffset, const std::string &displayName, bool isHighlighted) const {
        if (isHighlighted) {
            DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 4, COLOR_BORDER_HIGHLIGHTED);
        } else {
            DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 2, COLOR_BORDER);
        }

        DrawUtils::setFontSize(ConfigRenderItemFont::FONT_SIZE);

        DrawUtils::setFontColor(COLOR_TEXT);
        DrawUtils::print(16 * 2, yOffset + 8 + ConfigRenderItemFont::FONT_SIZE, displayName.c_str());
    }

    virtual void Draw(uint32_t yOffset, bool isHighlighted) const = 0;

    virtual void Update(bool) = 0;

    [[nodiscard]] virtual bool NeedsRedraw() const = 0;

    virtual void ResetNeedsRedraw() = 0;

    virtual void SetIsSelected(bool) {
    }

    virtual void ResetTextOffset() = 0;

    virtual uint32_t GetTextOffset() const {
        return 0;
    }
    virtual void IncrementTextOffset() = 0;
    virtual void DecrementTextOffset() = 0;

    virtual void
    OnButtonPressed(WUPSConfigButtons) {
    }
    virtual void OnInput(WUPSConfigSimplePadData) {
    }
    virtual void OnInputEx(WUPSConfigComplexPadData) {
    }

    [[nodiscard]] virtual bool IsMovementAllowed() const {
        return true;
    }
};
