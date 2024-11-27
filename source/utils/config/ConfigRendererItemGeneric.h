#pragma once

#include <cstdint>
#include <string>
#include <wups/config.h>

class ConfigRendererItemGeneric {
public:
    virtual ~ConfigRendererItemGeneric();

    virtual void drawGenericBoxAndText(uint32_t yOffset, const std::string &displayName, bool isHighlighted) const;

    virtual void Draw(uint32_t yOffset, bool isHighlighted) const = 0;

    virtual void Update(bool) = 0;

    [[nodiscard]] virtual bool NeedsRedraw() const = 0;

    virtual void ResetNeedsRedraw() = 0;

    virtual void SetIsSelected(bool);

    virtual void OnButtonPressed(WUPSConfigButtons);

    virtual void OnInput(WUPSConfigSimplePadData);

    virtual void OnInputEx(WUPSConfigComplexPadData);

    [[nodiscard]] virtual bool IsMovementAllowed() const;
};