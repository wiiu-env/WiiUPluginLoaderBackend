#pragma once
#include "ConfigRendererItemGeneric.h"
#include "config/WUPSConfigCategory.h"

class ConfigRendererItemCategory : public ConfigRendererItemGeneric {
public:
    explicit ConfigRendererItemCategory(const WUPSConfigAPIBackend::WUPSConfigCategory *category) : mCategory(category) {
        assert(category);
    }

    void Draw(uint32_t yOffset, bool isHighlighted) const override {
        drawGenericBoxAndText(yOffset, mCategory->getName(), isHighlighted);
    }

    void Update(bool) override {
    }

    [[nodiscard]] bool NeedsRedraw() const override {
        return false;
    }

    void ResetNeedsRedraw() override {
    }

private:
    const WUPSConfigAPIBackend::WUPSConfigCategory *mCategory;
};