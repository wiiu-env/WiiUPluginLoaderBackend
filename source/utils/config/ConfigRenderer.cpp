#include "ConfigRenderer.h"

#include "CategoryRenderer.h"
#include "ConfigDisplayItem.h"
#include "config/WUPSConfigItem.h"
#include "globals.h"
#include "plugin/PluginLoadWrapper.h"
#include "utils/DrawUtils.h"
#include "utils/StringTools.h"
#include "utils/input/Input.h"
#include "utils/logger.h"

#include <algorithm>

ConfigRenderer::ConfigRenderer(std::vector<ConfigDisplayItem> &&vec) : mConfigs(std::move(vec)) {
    std::ranges::copy(mConfigs,
                      std::back_inserter(mAllConfigs));
    std::ranges::copy_if(mConfigs,
                         std::back_inserter(mActiveConfigs),
                         [&](const auto &value) {
                             return value.isActivePlugin();
                         });
}

ConfigRenderer::~ConfigRenderer() = default;

ConfigSubState ConfigRenderer::Update(Input &input, const WUPSConfigSimplePadData &simpleInputData, const WUPSConfigComplexPadData &complexInputData) {
    // Check if the last input was on a wiimote
    for (uint32_t i = 0; i < std::size(complexInputData.kpad.data); i++) {
        const KPADError &kpadError = complexInputData.kpad.kpadError[i];
        const KPADStatus &status   = complexInputData.kpad.data[i];

        const bool isWiimote = status.extensionType == WPAD_EXT_CORE || status.extensionType == WPAD_EXT_NUNCHUK ||
                               status.extensionType == WPAD_EXT_MPLUS || status.extensionType == WPAD_EXT_MPLUS_NUNCHUK;

        if (kpadError == KPAD_ERROR_OK) {
            if (isWiimote && status.hold != 0) {
                mLastInputWasOnWiimote = true;
            } else if (!isWiimote && status.classic.hold != 0) {
                mLastInputWasOnWiimote = false;
            }
        }
    }
    if (complexInputData.vpad.vpadError == VPAD_READ_SUCCESS && complexInputData.vpad.data.hold != 0) {
        mLastInputWasOnWiimote = false;
    }
    switch (mState) {
        case STATE_MAIN:
            return UpdateStateMain(input);
        case STATE_SUB: {
            if (mCategoryRenderer) {
                auto subResult = mCategoryRenderer->Update(input, simpleInputData, complexInputData);
                if (subResult != SUB_STATE_RUNNING) {
                    mNeedRedraw         = true;
                    mActivePluginsDirty = false;
                    mState              = STATE_MAIN;
                    return SUB_STATE_RUNNING;
                }
                return SUB_STATE_RUNNING;
            } else {
                DEBUG_FUNCTION_LINE_WARN("State is RENDERER_STATE_CAT but mCategoryRenderer is null. Resetting state.");
                mState     = STATE_MAIN;
                mCursorPos = 0;
            }
        }
    }
    return SUB_STATE_ERROR;
}

void ConfigRenderer::Render() const {
    switch (mState) {
        case STATE_MAIN:
            RenderStateMain();
            break;
        case STATE_SUB: {
            if (mCategoryRenderer) {
                mCategoryRenderer->Render();
            } else {
                DEBUG_FUNCTION_LINE_WARN("render failed: state was RENDERER_STATE_CAT but mCategoryRenderer is NULL");
            }
            break;
        }
    }
}

bool ConfigRenderer::NeedsRedraw() const {
    if (mNeedRedraw) {
        return true;
    } else if (mCategoryRenderer) {
        return mCategoryRenderer->NeedsRedraw();
    }
    return false;
}

void ConfigRenderer::ResetNeedsRedraw() {
    mNeedRedraw = false;
    if (mCategoryRenderer) {
        mCategoryRenderer->ResetNeedsRedraw();
    }
}

ConfigSubState ConfigRenderer::UpdateStateMain(const Input &input) {
    auto &configs = GetConfigList();

    const auto prevSelectedItem = mCursorPos;

    const auto &savePendingConfigFn = [&configs, this]() {
        for (const auto &element : configs) {
            CallOnCloseCallback(element.get().getConfigInformation(), element.get().getConfig());
        }
    };

    auto totalElementSize = (int32_t) configs.size();
    if (input.data.buttons_d & Input::eButtons::BUTTON_DOWN) {
        mCursorPos++;
    } else if (input.data.buttons_d & Input::eButtons::BUTTON_LEFT) {
        // Paging up
        mCursorPos -= MAX_BUTTONS_ON_SCREEN - 1;
        // Don't jump past the top
        if (mCursorPos < 0)
            mCursorPos = 0;
    } else if (input.data.buttons_d & Input::eButtons::BUTTON_RIGHT) {
        // Paging down
        mCursorPos += MAX_BUTTONS_ON_SCREEN - 1;
        // Don't jump past the bottom
        if (mCursorPos >= totalElementSize)
            mCursorPos = totalElementSize - 1;
    } else if (input.data.buttons_d & Input::eButtons::BUTTON_UP) {
        mCursorPos--;
    } else if (input.data.buttons_d & Input::eButtons::BUTTON_PLUS) {
        if (mSetActivePluginsMode) {
            mNeedRedraw = true;
            mCategoryRenderer.reset();
            savePendingConfigFn();
            return SUB_STATE_RETURN_WITH_PLUGIN_RELOAD;
        }
    } else if (input.data.buttons_d & Input::eButtons::BUTTON_X) {
        if (!mSetActivePluginsMode && !mAllConfigs.empty()) {
            mSetActivePluginsMode = true;
            mNeedRedraw           = true;
            return SUB_STATE_RUNNING;
        }
    } else if (input.data.buttons_d & Input::eButtons::BUTTON_A) {
        if (mSetActivePluginsMode) {
            mActivePluginsDirty = true;
            mNeedRedraw         = true;
            configs[mCursorPos].get().toggleIsActivePlugin();
            return SUB_STATE_RUNNING;
        } else if (!configs.empty()) {
            if (mCursorPos != mCurrentOpen) {
                mCategoryRenderer.reset();
                mCategoryRenderer = make_unique_nothrow<CategoryRenderer>(&(configs[mCursorPos].get().getConfigInformation()), &(configs[mCursorPos].get().getConfig()), true);
            }
            mNeedRedraw  = true;
            mCurrentOpen = mCursorPos;
            mState       = STATE_SUB;
            return SUB_STATE_RUNNING;
        }
    } else if (input.data.buttons_d & (Input::eButtons::BUTTON_B | Input::eButtons::BUTTON_HOME)) {
        if (mSetActivePluginsMode) {
            for (auto &cur : mConfigs) {
                cur.resetIsActivePlugin();
            }
            mActivePluginsDirty   = false;
            mNeedRedraw           = true;
            mSetActivePluginsMode = false;
            return SUB_STATE_RUNNING;
        } else {
            mNeedRedraw = true;
            mCategoryRenderer.reset();
            savePendingConfigFn();
            return SUB_STATE_RETURN;
        }
    }

    if (mCursorPos < 0) {
        mCursorPos = totalElementSize - 1;
    } else if (mCursorPos >= totalElementSize) {
        mCursorPos = 0;
    }
    if (mCursorPos < 0) {
        mCursorPos = 0;
    }

    // Adjust the render offset when reaching the boundaries
    if (mCursorPos < mRenderOffset) {
        mRenderOffset = mCursorPos;
    } else if (mCursorPos >= mRenderOffset + MAX_BUTTONS_ON_SCREEN - 1) {
        mRenderOffset = mCursorPos - MAX_BUTTONS_ON_SCREEN + 1;
    }

    if (prevSelectedItem != mCursorPos) {
        mNeedRedraw = true;
    }

    return SUB_STATE_RUNNING;
}

void ConfigRenderer::RenderStateMain() const {
    auto &configs = GetConfigList();

    DrawUtils::beginDraw();
    DrawUtils::clear(COLOR_BACKGROUND);

    auto totalElementSize = (int32_t) configs.size();

    // Calculate the range of items to display
    int start = std::max(0, mRenderOffset);
    int end   = std::min(start + MAX_BUTTONS_ON_SCREEN, totalElementSize);

    if (mActiveConfigs.empty() && !mSetActivePluginsMode) {
        DrawUtils::setFontSize(24);
        std::string noConfigText = "No active plugins";
        uint32_t szNoConfig      = DrawUtils::getTextWidth(noConfigText.data());

        if (!mAllConfigs.empty()) {
            const auto activateHint = string_format("Press %s to activate inactive plugins", mLastInputWasOnWiimote ? "\uE048" : "\uE002");
            const auto szHint       = DrawUtils::getTextWidth(activateHint.c_str());

            DrawUtils::print((SCREEN_WIDTH / 2) - (szNoConfig / 2), (SCREEN_HEIGHT / 2) - 16, noConfigText.data());
            DrawUtils::print((SCREEN_WIDTH / 2) - (szHint / 2), (SCREEN_HEIGHT / 2) + 16, activateHint.data());
        } else {
            DrawUtils::print((SCREEN_WIDTH / 2) - (szNoConfig / 2), (SCREEN_HEIGHT / 2), noConfigText.data());
        }
    } else {
        uint32_t yOffset = 8 + 24 + 8 + 4;
        for (int32_t i = start; i < end; i++) {
            DrawConfigEntry(yOffset, configs[i].get().getConfigInformation(), i == mCursorPos, configs[i].get().isActivePlugin());
            yOffset += 42 + 8;
        }
    }

    DrawUtils::setFontColor(COLOR_TEXT);

    // draw top bar
    DrawUtils::setFontSize(24);
    if (mSetActivePluginsMode) {
        DrawUtils::print(16, 6 + 24, "Please select the plugins that should be active");
    } else {
        DrawUtils::print(16, 6 + 24, "Wii U Plugin System Config Menu");

        auto countInactivePlugins = mAllConfigs.size() - mActiveConfigs.size();
        if (countInactivePlugins > 0) {
            DrawUtils::setFontSize(14);
            const std::string plugin_unloaded = string_format("Found %d inactive plugins", countInactivePlugins);
            DrawUtils::print(SCREEN_WIDTH - 16 - DrawUtils::getTextWidth(MODULE_VERSION_FULL) - 32, 8 + 24, plugin_unloaded.c_str(), true);
        }
    }
    DrawUtils::setFontSize(18);
    DrawUtils::print(SCREEN_WIDTH - 16, 8 + 24, MODULE_VERSION_FULL, true);
    DrawUtils::drawRectFilled(8, 8 + 24 + 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);

    // draw bottom bar
    DrawUtils::drawRectFilled(8, SCREEN_HEIGHT - 24 - 8 - 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
    DrawUtils::setFontSize(18);
    DrawUtils::print(16, SCREEN_HEIGHT - 10, "\uE07D/\uE07E Navigate ");
    if (mSetActivePluginsMode) {
        DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, "\uE000 Toggle | \uE045 Apply", true);
    } else if (totalElementSize > 0) {
        const auto text = string_format("\ue000 Select | %s Manage plugins", mLastInputWasOnWiimote ? "\uE048" : "\uE002");
        DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, text.c_str(), true);
    }

    // draw scroll indicator
    DrawUtils::setFontSize(24);
    if (end < totalElementSize) {
        DrawUtils::print(SCREEN_WIDTH / 2 + 12, SCREEN_HEIGHT - 32, "\ufe3e", true);
    }
    if (start > 0) {
        DrawUtils::print(SCREEN_WIDTH / 2 + 12, 32 + 20, "\ufe3d", true);
    }

    // draw home button
    DrawUtils::setFontSize(18);
    const char *exitHint = "\ue044 Exit";
    if (mSetActivePluginsMode) {
        exitHint = "\ue001 Abort";
    }
    DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 10, exitHint, true);

    DrawUtils::endDraw();
}

void ConfigRenderer::DrawConfigEntry(uint32_t yOffset, const GeneralConfigInformation &configInformation, bool isHighlighted, bool isActive) const {
    DrawUtils::setFontColor(COLOR_TEXT);

    if (isHighlighted) {
        DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 4, COLOR_BORDER_HIGHLIGHTED);
    } else {
        DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 2, COLOR_BORDER);
    }

    int textXOffset = 16 * 2;
    if (mSetActivePluginsMode) {
        DrawUtils::setFontSize(24);
        if (isActive) {
            DrawUtils::print(textXOffset, yOffset + 8 + 24, "\u25C9");
        } else {
            DrawUtils::print(textXOffset, yOffset + 8 + 24, "\u25CE");
        }
        textXOffset += 32;
    }

    DrawUtils::setFontSize(24);

    DrawUtils::print(textXOffset, yOffset + 8 + 24, configInformation.name.c_str());
    uint32_t sz = DrawUtils::getTextWidth(configInformation.name.c_str());
    DrawUtils::setFontSize(12);
    DrawUtils::print(textXOffset + sz + 4, yOffset + 8 + 24, configInformation.author.c_str());
    DrawUtils::print(SCREEN_WIDTH - 16 * 2, yOffset + 8 + 24, configInformation.version.c_str(), true);
}

void ConfigRenderer::CallOnCloseCallback(const GeneralConfigInformation &info, const std::vector<std::unique_ptr<WUPSConfigAPIBackend::WUPSConfigCategory>> &categories) {
    for (const auto &cat : categories) {
        if (!cat->getCategories().empty()) {
            CallOnCloseCallback(info, cat->getCategories());
        }
        for (const auto &item : cat->getItems()) {
            item->onCloseCallback();
        }
    }
}

bool ConfigRenderer::GetActivePluginsIfChanged(std::vector<PluginLoadWrapper> &result) {
    if (mActivePluginsDirty) {
        std::vector<std::string> inactive_plugins;
        result.clear();
        for (const auto &cur : mConfigs) {
            result.emplace_back(cur.getConfigInformation().pluginData, cur.isActivePlugin());
        }
        return true;
    }
    return false;
}

void ConfigRenderer::CallOnCloseCallback(const GeneralConfigInformation &info, const WUPSConfigAPIBackend::WUPSConfig &config) {
    CallOnCloseCallback(info, config.getCategories());
    for (const auto &item : config.getItems()) {
        item->onCloseCallback();
    }
}

const std::vector<std::reference_wrapper<ConfigDisplayItem>> &ConfigRenderer::GetConfigList() const {
    if (mSetActivePluginsMode) {
        return mAllConfigs;
    }
    return mActiveConfigs;
}
