#include "ConfigRenderer.h"
#include "globals.h"
#include "utils/DrawUtils.h"
#include "utils/logger.h"
#include "utils/utils.h"

ConfigRenderer::ConfigRenderer(std::vector<ConfigDisplayItem> &&vec) : mConfigs(std::move(vec)) {
}

ConfigRenderer::~ConfigRenderer() = default;

ConfigSubState ConfigRenderer::Update(Input &input, const WUPSConfigSimplePadData &simpleInputData, const WUPSConfigComplexPadData &complexInputData) {
    switch (mState) {
        case STATE_MAIN:
            return UpdateStateMain(input);
        case STATE_SUB: {
            if (mCategoryRenderer) {
                if (const auto subResult = mCategoryRenderer->Update(input, simpleInputData, complexInputData); subResult != SUB_STATE_RUNNING) {
                    mNeedRedraw = true;
                    mState      = STATE_MAIN;
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
    }
    if (mCategoryRenderer) {
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
    if (mConfigs.empty()) {
        mNeedRedraw = true;
        return SUB_STATE_ERROR;
    }
    const auto prevSelectedItem = mCursorPos;

    const auto totalElementSize = static_cast<int32_t>(mConfigs.size());
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
    } else if (input.data.buttons_d & Input::eButtons::BUTTON_A) {
        if (mCursorPos != mCurrentOpen) {
            mCategoryRenderer.reset();
            mCategoryRenderer = make_unique_nothrow<CategoryRenderer>(&(mConfigs[mCursorPos].getConfigInformation()), &(mConfigs[mCursorPos].getConfig()), true);
        }
        mNeedRedraw  = true;
        mCurrentOpen = mCursorPos;
        mState       = STATE_SUB;
        return SUB_STATE_RUNNING;
    } else if (input.data.buttons_d & (Input::eButtons::BUTTON_B | Input::eButtons::BUTTON_HOME)) {
        mNeedRedraw = true;
        mCategoryRenderer.reset();
        for (const auto &element : mConfigs) {
            CallOnCloseCallback(element.getConfigInformation(), element.getConfig());
        }
        return SUB_STATE_RETURN;
    }

    if (mCursorPos < 0) {
        mCursorPos = totalElementSize - 1;
    } else if (mCursorPos > totalElementSize - 1) {
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
    const auto totalElementSize = static_cast<int32_t>(mConfigs.size());
    // Calculate the range of items to display
    const int start = std::max(0, mRenderOffset);
    const int end   = std::min(start + MAX_BUTTONS_ON_SCREEN, totalElementSize);

    DrawUtils::beginDraw();
    DrawUtils::clear(COLOR_BACKGROUND);

    uint32_t yOffset = 8 + 24 + 8 + 4;
    for (int32_t i = start; i < end; i++) {
        DrawConfigEntry(yOffset, mConfigs[i].getConfigInformation(), i == mCursorPos);
        yOffset += 42 + 8;
    }

    DrawUtils::setFontColor(COLOR_TEXT);

    // draw top bar
    DrawUtils::setFontSize(24);
    DrawUtils::print(16, 6 + 24, "Wii U Plugin System Config Menu");
    DrawUtils::setFontSize(18);
    DrawUtils::print(SCREEN_WIDTH - 16, 8 + 24, MODULE_VERSION_FULL, true);
    DrawUtils::drawRectFilled(8, 8 + 24 + 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);

    // draw bottom bar
    DrawUtils::drawRectFilled(8, SCREEN_HEIGHT - 24 - 8 - 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
    DrawUtils::setFontSize(18);
    DrawUtils::print(16, SCREEN_HEIGHT - 10, "\ue07d/\ue07e Navigate ");
    DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, "\ue000 Select", true);

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
    const auto exitHint = "\ue044 Exit";
    DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 10, exitHint, true);

    DrawUtils::endDraw();
}

void ConfigRenderer::DrawConfigEntry(const uint32_t yOffset, const GeneralConfigInformation &configInformation, const bool isHighlighted) const {
    DrawUtils::setFontColor(COLOR_TEXT);

    if (isHighlighted) {
        DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 4, COLOR_BORDER_HIGHLIGHTED);
    } else {
        DrawUtils::drawRect(16, yOffset, SCREEN_WIDTH - 16 * 2, 44, 2, COLOR_BORDER);
    }

    DrawUtils::setFontSize(24);
    DrawUtils::print(16 * 2, yOffset + 8 + 24, configInformation.name.c_str());
    const uint32_t sz = DrawUtils::getTextWidth(configInformation.name.c_str());
    DrawUtils::setFontSize(12);
    DrawUtils::print(16 * 2 + sz + 4, yOffset + 8 + 24, configInformation.author.c_str());
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

void ConfigRenderer::CallOnCloseCallback(const GeneralConfigInformation &info, const WUPSConfigAPIBackend::WUPSConfig &config) {
    CallOnCloseCallback(info, config.getCategories());
    for (const auto &item : config.getItems()) {
        item->onCloseCallback();
    }
}
