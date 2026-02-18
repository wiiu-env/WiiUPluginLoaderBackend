#pragma once

#include "ConfigDefines.h"
#include "ConfigDisplayItem.h"

#include <wups/config.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace WUPSConfigAPIBackend {
    class WUPSConfig;
    class WUPSConfigCategory;
} // namespace WUPSConfigAPIBackend
class PluginLoadWrapper;
class Input;
class CategoryRenderer;
class ConfigDisplayItem;
class ConfigListState;

class ConfigRenderer {

public:
    explicit ConfigRenderer(std::vector<ConfigDisplayItem> &&vec);

    ~ConfigRenderer();

    ConfigSubState Update(Input &input, const WUPSConfigSimplePadData &simpleInputData, const WUPSConfigComplexPadData &complexInputData);

    void Render() const;

    [[nodiscard]] bool NeedsRedraw() const;
    void ResetNeedsRedraw();
    void RequestRedraw();

    bool GetPluginsListIfChanged(std::vector<PluginLoadWrapper> &result);

    void SetListState(std::unique_ptr<ConfigListState> state);
    const std::vector<ConfigDisplayItem> &GetConfigItems();
    std::vector<std::reference_wrapper<ConfigDisplayItem>> &GetFilteredConfigItems(); // Mutable access
    int32_t GetCursorPos() const;
    void EnterSelectedCategory();
    void Exit();
    void ExitWithReload();
    void SetPluginsListDirty(bool dirty);

private:
    ConfigSubState UpdateStateMain(const Input &input);
    void RenderStateMain() const;
    void DrawConfigEntry(uint32_t yOffset, const ConfigDisplayItem &item, bool isHighlighted) const;

    void CallOnCloseCallback(const GeneralConfigInformation &info, const std::vector<std::unique_ptr<WUPSConfigAPIBackend::WUPSConfigCategory>> &categories);
    void CallOnCloseCallback(const GeneralConfigInformation &info, const WUPSConfigAPIBackend::WUPSConfig &config);
    void SavePendingConfigs();


    [[nodiscard]] const std::vector<std::reference_wrapper<ConfigDisplayItem>> &GetDisplayedConfigList() const;

    enum State {
        STATE_MAIN = 0,
        STATE_SUB  = 1,
    };

    std::vector<ConfigDisplayItem> mConfigs;
    mutable std::vector<std::reference_wrapper<ConfigDisplayItem>> mFilteredConfigs;

    std::unique_ptr<ConfigListState> mListState;
    std::unique_ptr<CategoryRenderer> mCategoryRenderer;

    State mState = STATE_MAIN;
    // Used to signal the main loop to return a specific state
    ConfigSubState mNextSubState = SUB_STATE_RUNNING;

    int32_t mCursorPos    = 0;
    int32_t mRenderOffset = 0;
    int32_t mCurrentOpen  = -1;

    bool mNeedRedraw            = true;
    bool mPluginListDirty       = false;
    bool mLastInputWasOnWiimote = false;
};