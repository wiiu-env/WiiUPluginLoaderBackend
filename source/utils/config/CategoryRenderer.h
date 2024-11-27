#pragma once

#include "ConfigDefines.h"
#include "ConfigDisplayItem.h"
#include "ConfigRendererItemGeneric.h"
#include "config/WUPSConfigCategory.h"
#include "utils/input/Input.h"

#include <cstdint>
#include <memory>
#include <vector>

class CategoryRenderer {

public:
    explicit CategoryRenderer(const GeneralConfigInformation *info, const WUPSConfigAPIBackend::WUPSConfigCategory *cat, bool isRoot);

    ~CategoryRenderer();

    ConfigSubState Update(Input &input, const WUPSConfigSimplePadData &simpleInputData, const WUPSConfigComplexPadData &complexInputData);

    void Render() const;

    [[nodiscard]] bool NeedsRedraw() const;

    void ResetNeedsRedraw();

private:
    ConfigSubState UpdateStateMain(const Input &input, const WUPSConfigSimplePadData &simpleInputData, const WUPSConfigComplexPadData &complexInputData);

    void RenderStateMain() const;

    void RenderMainLayout() const;

    enum State {
        STATE_MAIN = 0,
        STATE_SUB  = 1,
    };

    State mState                                           = STATE_MAIN;
    int32_t mCurrentOpen                                   = -1;
    int32_t mCursorPos                                     = 0;
    int32_t mRenderOffset                                  = 0;
    std::unique_ptr<CategoryRenderer> mSubCategoryRenderer = {};
    const GeneralConfigInformation *mInfo                  = {};
    const WUPSConfigAPIBackend::WUPSConfigCategory *mCat   = {};

    std::vector<std::unique_ptr<ConfigRendererItemGeneric>> mItemRenderer = {};
    bool mIsItemMovementAllowed                                           = true;
    bool mFirstFrame                                                      = true;
    bool mIsRoot                                                          = false;
    bool mNeedsRedraw                                                     = true;
};
