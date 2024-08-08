#pragma once
#include "ConfigRendererItemGeneric.h"
#include "config/WUPSConfigCategory.h"

class ConfigRendererItemCategory : public ConfigRendererItemGeneric {
public:
    explicit ConfigRendererItemCategory(const WUPSConfigAPIBackend::WUPSConfigCategory *category) : mCategory(category), mTextOffset(0) {
        assert(category);
    }

    void Draw(uint32_t yOffset, bool isHighlighted) const override {
        drawGenericBoxAndText(yOffset, ConfigRenderItemFont::GetOffsettedText(mCategory->getName(), mTextOffset), isHighlighted);
    }

    void Update(bool) override {
    }

    [[nodiscard]] bool NeedsRedraw() const override {
        return false;
    }

    void ResetNeedsRedraw() override {
    }

    void ResetTextOffset() override {
        mTextOffset = 0;
    }

    uint32_t GetTextOffset() const override {
        return mTextOffset;
    }

    void IncrementTextOffset() override {
        if (!mCategory) return;
        ConfigRenderItemFont::IncrementOffset(mCategory->getName(), mTextOffset);
    }

    void DecrementTextOffset() override {
        if (mTextOffset > 0) mTextOffset--;
    }

private:
    const WUPSConfigAPIBackend::WUPSConfigCategory *mCategory;
    uint32_t mTextOffset = 0;
};
