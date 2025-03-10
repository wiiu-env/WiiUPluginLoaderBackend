#pragma once

#include "ConfigDefines.h"
#include "ConfigDisplayItem.h"

#include <wups/config.h>

#include <memory>
#include <vector>

#include <cstdint>

namespace WUPSConfigAPIBackend {
    class WUPSConfig;
    class WUPSConfigCategory;
} // namespace WUPSConfigAPIBackend
class PluginLoadWrapper;
class Input;
class CategoryRenderer;
class ConfigDisplayItem;
class ConfigRenderer {

public:
    explicit ConfigRenderer(std::vector<ConfigDisplayItem> &&vec);

    ~ConfigRenderer();

    ConfigSubState Update(Input &input, const WUPSConfigSimplePadData &simpleInputData, const WUPSConfigComplexPadData &complexInputData);

    void Render() const;

    [[nodiscard]] bool NeedsRedraw() const;

    void ResetNeedsRedraw();

    bool GetActivePluginsIfChanged(std::vector<PluginLoadWrapper> &result);

private:
    ConfigSubState UpdateStateMain(const Input &input);

    void RenderStateMain() const;

    void DrawConfigEntry(uint32_t yOffset, const GeneralConfigInformation &configInformation, bool isHighlighted, bool isActive) const;

    void CallOnCloseCallback(const GeneralConfigInformation &info, const std::vector<std::unique_ptr<WUPSConfigAPIBackend::WUPSConfigCategory>> &categories);
    void CallOnCloseCallback(const GeneralConfigInformation &info, const WUPSConfigAPIBackend::WUPSConfig &config);

    [[nodiscard]] const std::vector<std::reference_wrapper<ConfigDisplayItem>> &GetConfigList() const;

    enum State {
        STATE_MAIN = 0,
        STATE_SUB  = 1,
    };

    std::vector<ConfigDisplayItem> mConfigs;
    std::vector<std::reference_wrapper<ConfigDisplayItem>> mAllConfigs;
    std::vector<std::reference_wrapper<ConfigDisplayItem>> mActiveConfigs;
    std::unique_ptr<CategoryRenderer> mCategoryRenderer;

    State mState = STATE_MAIN;

    int32_t mCursorPos    = 0;
    int32_t mRenderOffset = 0;
    int32_t mCurrentOpen  = -1;

    bool mNeedRedraw            = true;
    bool mSetActivePluginsMode  = false;
    bool mActivePluginsDirty    = false;
    bool mLastInputWasOnWiimote = false;
};
