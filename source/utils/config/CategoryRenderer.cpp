#include "CategoryRenderer.h"

#include "ConfigDefines.h"
#include "ConfigRendererItem.h"
#include "ConfigRendererItemCategory.h"
#include "ConfigRendererItemGeneric.h"
#include "ConfigUtils.h"
#include "utils/DrawUtils.h"
#include "utils/StringTools.h"
#include "utils/input/Input.h"
#include "utils/logger.h"

#include <wups/config.h>

#include <algorithm>

#include <cassert>
#include <cstdint>

CategoryRenderer::CategoryRenderer(const GeneralConfigInformation *info, const WUPSConfigAPIBackend::WUPSConfigCategory *cat, const bool isRoot)
    : mInfo(info), mCat(cat), mIsRoot(isRoot) {
    for (uint32_t i = 0; i < cat->getCategories().size() + cat->getItems().size(); i++) {
        if (i < cat->getCategories().size()) {
            auto item = make_unique_nothrow<ConfigRendererItemCategory>(cat->getCategories()[i].get());
            assert(item);
            mItemRenderer.push_back(std::move(item));
        } else {
            const auto itemIndex = static_cast<int32_t>(i - cat->getCategories().size());
            if (itemIndex < 0 || itemIndex >= static_cast<int32_t>(cat->getItems().size())) {
                assert(false);
            }
            auto item = make_unique_nothrow<ConfigRendererItem>(cat->getItems()[itemIndex].get());
            assert(item);
            mItemRenderer.push_back(std::move(item));
        }
    }

    mCursorPos = 0;
    if (!mItemRenderer.empty()) {
        mItemRenderer[mCursorPos]->SetIsSelected(true);
    }

    // Make sure to call Update to get the current text of an item.
    for (uint32_t i = 0; i < mItemRenderer.size(); i++) {
        const bool isHighlighted = (static_cast<int>(i) == mCursorPos);
        mItemRenderer[i]->Update(isHighlighted);
    }
}

CategoryRenderer::~CategoryRenderer() {
    if (mCursorPos < static_cast<int32_t>(mItemRenderer.size())) {
        mItemRenderer[mCursorPos]->SetIsSelected(false);
    }
}

ConfigSubState CategoryRenderer::Update(Input &input, const WUPSConfigSimplePadData &simpleInputData, const WUPSConfigComplexPadData &complexInputData) {
    switch (mState) {
        case STATE_MAIN: {
            const auto res = UpdateStateMain(input, simpleInputData, complexInputData);
            mFirstFrame    = false;
            return res;
        }
        case STATE_SUB: {
            if (mSubCategoryRenderer) {
                if (const auto subResult = mSubCategoryRenderer->Update(input, simpleInputData, complexInputData); subResult != SUB_STATE_RUNNING) {
                    mNeedsRedraw = true;
                    mState       = STATE_MAIN;
                    mFirstFrame  = true;
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

ConfigSubState CategoryRenderer::UpdateStateMain(const Input &input, const WUPSConfigSimplePadData &simpleInputData, const WUPSConfigComplexPadData &complexInputData) {
    if (mIsItemMovementAllowed && input.data.buttons_d & Input::eButtons::BUTTON_B) {
        return SUB_STATE_RETURN;
    }
    if (mItemRenderer.empty()) {
        return SUB_STATE_RUNNING;
    }

    const auto totalElementSize    = mItemRenderer.size();
    const int32_t prevSelectedItem = mCursorPos;

    if (mIsItemMovementAllowed) {
        if (input.data.buttons_d & Input::eButtons::BUTTON_DOWN) {
            mCursorPos++;
        } else if (input.data.buttons_d & Input::eButtons::BUTTON_UP) {
            mCursorPos--;
        } else if (input.data.buttons_d & Input::eButtons::BUTTON_A) {
            if (mCursorPos < static_cast<int32_t>(mCat->getCategories().size())) {
                if (mCurrentOpen != mCursorPos) {
                    mSubCategoryRenderer.reset();
                    mSubCategoryRenderer = make_unique_nothrow<CategoryRenderer>(mInfo, mCat->getCategories()[mCursorPos].get(), false);
                }
                mCurrentOpen = mCursorPos;
                mState       = STATE_SUB;
                mNeedsRedraw = true;
                return SUB_STATE_RUNNING;
            }
        }
    }

    if (mCursorPos < 0) {
        mCursorPos = static_cast<int32_t>(totalElementSize) - 1;
    } else if (mCursorPos > static_cast<int32_t>(totalElementSize - 1)) {
        mCursorPos = 0;
    }

    // Adjust the render offset when reaching the boundaries
    if (mCursorPos < mRenderOffset) {
        mRenderOffset = mCursorPos;
    } else if (mCursorPos >= mRenderOffset + MAX_BUTTONS_ON_SCREEN - 1) {
        mRenderOffset = mCursorPos - MAX_BUTTONS_ON_SCREEN + 1;
    }

    bool posJustChanged = false;
    if (prevSelectedItem != mCursorPos) {
        mItemRenderer[prevSelectedItem]->SetIsSelected(false);
        mItemRenderer[mCursorPos]->SetIsSelected(true);
        posJustChanged = true;
        mNeedsRedraw   = true;
    }

    if (!posJustChanged && !mFirstFrame) {
        // WUPSConfigItemV2
        mItemRenderer[mCursorPos]->OnInput(simpleInputData);
        mItemRenderer[mCursorPos]->OnInputEx(complexInputData);

        // WUPSConfigItemV1
        if (input.data.buttons_d != 0) {
            WUPSConfigButtons buttons = input.data.buttons_d;
            buttons                   = ConfigUtils::convertInputs(buttons);
            if (!mIsItemMovementAllowed) {
                buttons &= ~MOVE_ITEM_INPUT_MASK;
            }
            mItemRenderer[mCursorPos]->OnButtonPressed(buttons);
        }
    }

    mIsItemMovementAllowed = mItemRenderer[mCursorPos]->IsMovementAllowed();

    for (uint32_t i = 0; i < mItemRenderer.size(); i++) {
        const bool isHighlighted = (static_cast<int>(i) == mCursorPos);
        mItemRenderer[i]->Update(isHighlighted);
    }

    return SUB_STATE_RUNNING;
}


void CategoryRenderer::ResetNeedsRedraw() {
    mNeedsRedraw = false;
    if (mSubCategoryRenderer) {
        mSubCategoryRenderer->ResetNeedsRedraw();
    }
    for (const auto &item : mItemRenderer) {
        item->ResetNeedsRedraw();
    }
}


bool CategoryRenderer::NeedsRedraw() const {
    if (mNeedsRedraw) {
        return true;
    }
    if (mSubCategoryRenderer) {
        if (mSubCategoryRenderer->NeedsRedraw()) {
            return true;
        }
    }
    for (const auto &item : mItemRenderer) {
        if (item->NeedsRedraw()) {
            return true;
        }
    }
    return false;
}

void CategoryRenderer::Render() const {
    switch (mState) {
        case STATE_MAIN:
            RenderStateMain();
            break;
        case STATE_SUB: {
            if (mSubCategoryRenderer) {
                mSubCategoryRenderer->Render();
            } else {
                DEBUG_FUNCTION_LINE_WARN("STATE_SUB but mSubCategoryRenderer is NULL");
            }
            break;
        }
    }
}

void CategoryRenderer::RenderStateMain() const {
    if (mItemRenderer.empty()) {
        DrawUtils::beginDraw();
        RenderMainLayout();

        const std::string text(mIsRoot ? "This plugin can not be configured" : "This category is empty");

        DrawUtils::setFontSize(24);
        const uint32_t sz = DrawUtils::getTextWidth(text.c_str());
        DrawUtils::print((SCREEN_WIDTH / 2) - (sz / 2), (SCREEN_HEIGHT / 2), text.c_str());

        DrawUtils::endDraw();
        return;
    }
    const auto totalElementSize = static_cast<int>(mItemRenderer.size());

    // Calculate the range of items to display
    const int start = std::max(0, mRenderOffset);
    const int end   = std::min(start + MAX_BUTTONS_ON_SCREEN, totalElementSize);

    DrawUtils::beginDraw();

    RenderMainLayout();

    uint32_t yOffset = 8 + 24 + 8 + 4;
    for (int32_t i = start; i < end; i++) {
        const bool isHighlighted = (i == mCursorPos);
        mItemRenderer[i]->Draw(yOffset, isHighlighted);
        yOffset += 42 + 8;
    }

    // draw scroll indicator
    DrawUtils::setFontSize(24);
    if (end < totalElementSize) {
        DrawUtils::print(SCREEN_WIDTH / 2 + 12, SCREEN_HEIGHT - 32, "\ufe3e", true);
    }
    if (start > 0) {
        DrawUtils::print(SCREEN_WIDTH / 2 + 12, 32 + 20, "\ufe3d", true);
    }

    DrawUtils::endDraw();
}

void CategoryRenderer::RenderMainLayout() const {
    DrawUtils::clear(COLOR_BACKGROUND);

    DrawUtils::setFontColor(COLOR_TEXT);
    // draw top bar
    DrawUtils::setFontSize(24);
    DrawUtils::print(16, 6 + 24, StringTools::truncate(mInfo->name, 45).c_str());
    DrawUtils::setFontSize(18);
    DrawUtils::print(SCREEN_WIDTH - 16, 8 + 24, mInfo->version.c_str(), true);
    DrawUtils::drawRectFilled(8, 8 + 24 + 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);

    // draw bottom bar
    DrawUtils::drawRectFilled(8, SCREEN_HEIGHT - 24 - 8 - 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
    DrawUtils::setFontSize(18);
    DrawUtils::print(16, SCREEN_HEIGHT - 10, "\ue07d Navigate ");
    DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, "\ue000 Select", true);

    // draw home button
    DrawUtils::setFontSize(18);
    const auto exitHint = "\ue001 Back";
    DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 10, exitHint, true);
}