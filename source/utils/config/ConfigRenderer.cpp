#include "ConfigRenderer.h"
#include "ConfigRendererStates.h"

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
#include <iterator>
#include <utility>

ConfigRenderer::ConfigRenderer(std::vector<ConfigDisplayItem> &&vec) : mConfigs(std::move(vec)) {
    SetListState(std::make_unique<DefaultListState>());
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

    // Reset transient return state
    mNextSubState = SUB_STATE_RUNNING;

    switch (mState) {
        case STATE_MAIN:
            return UpdateStateMain(input);
        case STATE_SUB: {
            if (mCategoryRenderer) {
                auto subResult = mCategoryRenderer->Update(input, simpleInputData, complexInputData);
                if (subResult != SUB_STATE_RUNNING) {
                    mNeedRedraw      = true;
                    mPluginListDirty = false;
                    mState           = STATE_MAIN;
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

void ConfigRenderer::RequestRedraw() {
    mNeedRedraw = true;
}

ConfigSubState ConfigRenderer::UpdateStateMain(const Input &input) {
    if (!mListState) return SUB_STATE_ERROR;

    auto &configs               = GetDisplayedConfigList();
    const auto prevSelectedItem = mCursorPos;
    auto totalElementSize       = (int32_t) configs.size();

    // Delegate specific inputs to the State
    bool inputHandled = mListState->HandleInput(*this, input);

    if (mNextSubState != SUB_STATE_RUNNING) {
        return mNextSubState;
    }

    if (inputHandled) {
        return SUB_STATE_RUNNING;
    }

    // Handle Navigation (Common to all states)
    if (input.data.buttons_d & Input::eButtons::BUTTON_DOWN) {
        mCursorPos++;
    } else if (input.data.buttons_d & Input::eButtons::BUTTON_LEFT) {
        mCursorPos -= MAX_BUTTONS_ON_SCREEN - 1;
        if (mCursorPos < 0) mCursorPos = 0;
    } else if (input.data.buttons_d & Input::eButtons::BUTTON_RIGHT) {
        mCursorPos += MAX_BUTTONS_ON_SCREEN - 1;
        if (mCursorPos >= totalElementSize) mCursorPos = totalElementSize - 1;
    } else if (input.data.buttons_d & Input::eButtons::BUTTON_UP) {
        mCursorPos--;
    }

    if (totalElementSize > 0) {
        if (mCursorPos < 0) mCursorPos = totalElementSize - 1;
        else if (mCursorPos >= totalElementSize)
            mCursorPos = 0;
    } else {
        mCursorPos = 0;
    }

    // Adjust render offset
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
    auto &configs = GetDisplayedConfigList();

    DrawUtils::beginDraw();
    DrawUtils::clear(COLOR_BACKGROUND);

    auto totalElementSize = (int32_t) configs.size();
    int start             = std::max(0, mRenderOffset);
    int end               = std::min(start + MAX_BUTTONS_ON_SCREEN, totalElementSize);

    if (configs.empty()) {
        DrawUtils::setFontSize(24);
        std::string noConfigText = "No plugins available";

        if (!mListState->IsMainView()) {
            noConfigText = "No active plugins";
        }

        uint32_t szNoConfig = DrawUtils::getTextWidth(noConfigText.data());

        if (mListState->IsMainView()) {
            DrawUtils::print((SCREEN_WIDTH / 2) - (szNoConfig / 2), (SCREEN_HEIGHT / 2), noConfigText.data());
        } else {
            const auto activateHint = string_format("Press %s to activate inactive plugins", mLastInputWasOnWiimote ? "\uE048" : "\uE002");
            const auto szHint       = DrawUtils::getTextWidth(activateHint.c_str());
            DrawUtils::print((SCREEN_WIDTH / 2) - (szNoConfig / 2), (SCREEN_HEIGHT / 2) - 16, noConfigText.data());
            DrawUtils::print((SCREEN_WIDTH / 2) - (szHint / 2), (SCREEN_HEIGHT / 2) + 16, activateHint.data());
        }
    } else {
        uint32_t yOffset = 8 + 24 + 8 + 4;
        for (int32_t i = start; i < end; i++) {
            DrawConfigEntry(yOffset, configs[i].get(), i == mCursorPos);
            yOffset += 42 + 8;
        }
    }

    DrawUtils::setFontColor(COLOR_TEXT);

    // Top Bar
    DrawUtils::setFontSize(24);
    DrawUtils::print(16, 6 + 24, mListState->GetTitle().c_str());

    if (mListState->IsMainView()) {
        auto countInactivePlugins = mConfigs.size() - mFilteredConfigs.size();
        if (countInactivePlugins > 0) {
            DrawUtils::setFontSize(14);
            const std::string plugin_unloaded = string_format("Found %d inactive plugins", countInactivePlugins);
            DrawUtils::print(SCREEN_WIDTH - 16 - DrawUtils::getTextWidth(MODULE_VERSION_FULL) - 32, 8 + 24, plugin_unloaded.c_str(), true);
        }
    }

    DrawUtils::setFontSize(18);
    DrawUtils::print(SCREEN_WIDTH - 16, 8 + 24, MODULE_VERSION_FULL, true);
    DrawUtils::drawRectFilled(8, 8 + 24 + 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);

    // Bottom Bar
    DrawUtils::drawRectFilled(8, SCREEN_HEIGHT - 24 - 8 - 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
    DrawUtils::setFontSize(18);

    if (totalElementSize > 0) {
        DrawUtils::print(16, SCREEN_HEIGHT - 10, "\uE07D/\uE07E Navigate ");
    }
    DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, mListState->GetBottomBar(mLastInputWasOnWiimote).c_str(), true);

    // Scroll Indicator
    DrawUtils::setFontSize(24);
    if (end < totalElementSize) {
        DrawUtils::print(SCREEN_WIDTH / 2 + 12, SCREEN_HEIGHT - 32, "\ufe3e", true);
    }
    if (start > 0) {
        DrawUtils::print(SCREEN_WIDTH / 2 + 12, 32 + 20, "\ufe3d", true);
    }

    // Home Button
    DrawUtils::setFontSize(18);
    const char *exitHint = mListState->IsMainView() ? "\ue001 Abort" : "\ue044 Exit";
    DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 10, exitHint, true);

    DrawUtils::endDraw();
}

void ConfigRenderer::DrawConfigEntry(uint32_t yOffset, const ConfigDisplayItem &item, bool isHighlighted) const {
    DrawUtils::setFontColor(COLOR_TEXT);

    if (isHighlighted) {
        DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 4, COLOR_BORDER_HIGHLIGHTED);
    } else {
        DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 2, COLOR_BORDER);
    }

    int textXOffset = 16 * 2;

    // Delegate Icon drawing to state, returns true if icon was drawn
    if (mListState->RenderItemIcon(item, textXOffset, yOffset + 8 + 24)) {
        textXOffset += 32;
    }

    DrawUtils::setFontSize(24);

    const auto &configInformation = item.getConfigInformation();
    DrawUtils::print(textXOffset, yOffset + 8 + 24, configInformation.name.c_str());
    uint32_t sz = DrawUtils::getTextWidth(configInformation.name.c_str());
    DrawUtils::setFontSize(12);
    DrawUtils::print(textXOffset + sz + 4, yOffset + 8 + 24, configInformation.author.c_str());
    DrawUtils::print(SCREEN_WIDTH - 16 * 2, yOffset + 8 + 24, configInformation.version.c_str(), true);
}

void ConfigRenderer::SetListState(std::unique_ptr<ConfigListState> state) {
    mListState    = std::move(state);
    mNeedRedraw   = true;
    mCursorPos    = 0;
    mRenderOffset = 0;
    // Fallback to "show all"
    std::function<bool(const ConfigDisplayItem &)> pred = [](const auto &) { return true; };
    if (mListState) {
        pred = mListState->GetConfigFilter();
    }
    // Copy references into filteredConfigView
    mFilteredConfigs.clear();
    std::ranges::copy_if(mConfigs, std::back_inserter(mFilteredConfigs),
                         std::move(pred));
}

const std::vector<ConfigDisplayItem> &ConfigRenderer::GetConfigItems() {
    return mConfigs;
}

std::vector<std::reference_wrapper<ConfigDisplayItem>> &ConfigRenderer::GetFilteredConfigItems() {
    return mFilteredConfigs;
}

int32_t ConfigRenderer::GetCursorPos() const {
    return mCursorPos;
}

void ConfigRenderer::EnterSelectedCategory() {
    auto &items = GetDisplayedConfigList();
    if (mCursorPos < 0 || static_cast<size_t>(mCursorPos) >= items.size()) return;

    if (mCursorPos != mCurrentOpen) {
        mCategoryRenderer.reset();
        mCategoryRenderer = make_unique_nothrow<CategoryRenderer>(&(items[mCursorPos].get().getConfigInformation()), &(items[mCursorPos].get().getConfig()), true);
    }
    mNeedRedraw  = true;
    mCurrentOpen = mCursorPos;
    mState       = STATE_SUB;
}

void ConfigRenderer::SavePendingConfigs() {
    for (const auto &element : mConfigs) {
        CallOnCloseCallback(element.getConfigInformation(), element.getConfig());
    }
}

void ConfigRenderer::Exit() {
    mNeedRedraw = true;
    mCategoryRenderer.reset();
    SavePendingConfigs();
    mNextSubState = SUB_STATE_RETURN;
}

void ConfigRenderer::ExitWithReload() {
    mNeedRedraw = true;
    mCategoryRenderer.reset();
    SavePendingConfigs();
    mNextSubState = SUB_STATE_RETURN_WITH_PLUGIN_RELOAD;
}

void ConfigRenderer::SetPluginsListDirty(bool dirty) {
    mPluginListDirty = dirty;
}

bool ConfigRenderer::GetPluginsListIfChanged(std::vector<PluginLoadWrapper> &result) {
    if (mPluginListDirty) {
        result.clear();
        for (const auto &cur : mConfigs) {
            result.emplace_back(cur.getConfigInformation().pluginData, cur.isActivePlugin(), cur.isHeapTrackingEnabled());
        }
        return true;
    }
    return false;
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

void ConfigRenderer::CallOnCloseCallback(const GeneralConfigInformation &info, const WUPSConfigAPIBackend::WUPSConfig &config) {
    CallOnCloseCallback(info, config.getCategories());
    for (const auto &item : config.getItems()) {
        item->onCloseCallback();
    }
}

const std::vector<std::reference_wrapper<ConfigDisplayItem>> &ConfigRenderer::GetDisplayedConfigList() const {
    return mFilteredConfigs;
}