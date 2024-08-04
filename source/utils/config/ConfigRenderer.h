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
        std::copy(mConfigs.begin(), mConfigs.end(),
                  std::back_inserter(mAllConfigs));
        std::copy_if(mConfigs.begin(), mConfigs.end(),
                     std::back_inserter(mActiveConfigs),
                     [&](const auto &value) {
                         return value.isActivePlugin();
                     });
    }

    ~ConfigRenderer() = default;

    ConfigSubState Update(Input &input, const WUPSConfigSimplePadData &simpleInputData, const WUPSConfigComplexPadData &complexInputData);

    void Render() const;

    [[nodiscard]] bool NeedsRedraw() const;

    void ResetNeedsRedraw();

private:
    ConfigSubState UpdateStateMain(const Input &input);

    void RenderStateMain() const;

    [[nodiscard]] const std::vector<std::reference_wrapper<ConfigDisplayItem>> &GetConfigList() const {
        if (mSetActivePluginsMode) {
            return mAllConfigs;
        }
        return mActiveConfigs;
    }

    void drawConfigEntry(uint32_t yOffset, const GeneralConfigInformation &configInformation, bool isHighlighted, bool isActive) const;

    enum State {
        STATE_MAIN = 0,
        STATE_SUB  = 1,
    };

    std::vector<ConfigDisplayItem> mConfigs;
    std::vector<std::reference_wrapper<ConfigDisplayItem>> mAllConfigs;
    std::vector<std::reference_wrapper<ConfigDisplayItem>> mActiveConfigs;
    std::unique_ptr<CategoryRenderer> mCategoryRenderer = {};

    State mState = STATE_MAIN;

    int32_t mCursorPos    = 0;
    int32_t mRenderOffset = 0;
    int32_t mCurrentOpen  = -1;
    void CallOnCloseCallback(const GeneralConfigInformation &info, const std::vector<std::unique_ptr<WUPSConfigAPIBackend::WUPSConfigCategory>> &categories);
    void CallOnCloseCallback(const GeneralConfigInformation &info, const WUPSConfigAPIBackend::WUPSConfig &config);

    bool mNeedRedraw           = true;
    bool mSetActivePluginsMode = false;
    bool mActivePluginsDirty   = false;
};
