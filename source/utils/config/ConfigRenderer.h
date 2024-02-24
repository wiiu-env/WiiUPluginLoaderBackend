#pragma once
#include "../DrawUtils.h"
#include "../input/Input.h"
#include "../logger.h"
#include "CategoryRenderer.h"
#include "globals.h"
#include <memory>
#include <vector>

class ConfigRenderer {

public:
    explicit ConfigRenderer(std::vector<ConfigDisplayItem> &&vec) : mConfigs(std::move(vec)) {
    }
    ~ConfigRenderer() = default;

    ConfigSubState Update(Input &input, const WUPSConfigSimplePadData &simpleInputData, const WUPSConfigComplexPadData &complexInputData);

    void Render() const;

private:
    ConfigSubState UpdateStateMain(const Input &input);

    void RenderStateMain() const;

    void drawConfigEntry(uint32_t yOffset, const GeneralConfigInformation &configInformation, bool isHighlighted) const;

    enum State {
        STATE_MAIN = 0,
        STATE_SUB  = 1,
    };

    std::vector<ConfigDisplayItem> mConfigs;
    std::unique_ptr<CategoryRenderer> mCategoryRenderer = {};

    State mState = STATE_MAIN;

    int32_t mCursorPos    = 0;
    int32_t mRenderOffset = 0;
    int32_t mCurrentOpen  = -1;
    void CallOnCloseCallback(const GeneralConfigInformation &info, const std::vector<std::unique_ptr<WUPSConfigAPIBackend::WUPSConfigCategory>> &categories);
    void CallOnCloseCallback(const GeneralConfigInformation &info, const WUPSConfigAPIBackend::WUPSConfig &config);
};
