#include "ButtonComboManager.h"
#include "utils/logger.h"
#include "utils/utils.h"

#include <buttoncombo/api.h>
#include <buttoncombo/defines.h>

#include <wups/button_combo_internal.h>

#include <optional>

namespace {
    WUPSButtonCombo_Error convertError(const ButtonComboModule_Error other) {
        switch (other) {
            case BUTTON_COMBO_MODULE_ERROR_SUCCESS:
                return WUPS_BUTTON_COMBO_ERROR_SUCCESS;
            case BUTTON_COMBO_MODULE_ERROR_INVALID_ARGUMENT:
            case BUTTON_COMBO_MODULE_ERROR_INVALID_COMBO:
            case BUTTON_COMBO_MODULE_ERROR_INVALID_COMBO_TYPE:
            case BUTTON_COMBO_MODULE_ERROR_DURATION_MISSING:
                return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
            case BUTTON_COMBO_MODULE_ERROR_INCOMPATIBLE_OPTIONS_VERSION:
            case BUTTON_COMBO_MODULE_ERROR_MODULE_NOT_FOUND:
            case BUTTON_COMBO_MODULE_ERROR_MODULE_MISSING_EXPORT:
            case BUTTON_COMBO_MODULE_ERROR_UNSUPPORTED_API_VERSION:
            case BUTTON_COMBO_MODULE_ERROR_UNSUPPORTED_COMMAND:
            case BUTTON_COMBO_MODULE_ERROR_LIB_UNINITIALIZED:
                return WUPS_BUTTON_COMBO_ERROR_INTERNAL_INVALID_VERSION;
            case BUTTON_COMBO_MODULE_ERROR_UNKNOWN_ERROR:
                break;
            case BUTTON_COMBO_MODULE_ERROR_HANDLE_NOT_FOUND:
                return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
            case BUTTON_COMBO_MODULE_ERROR_ABORTED:
                return WUPS_BUTTON_COMBO_ERROR_ABORTED;
        }
        return WUPS_BUTTON_COMBO_ERROR_UNKNOWN_ERROR;
    }

    ButtonComboModule_ControllerTypes convert(const WUPSButtonCombo_ControllerTypes other) {
        ButtonComboModule_ControllerTypes res = BUTTON_COMBO_MODULE_CONTROLLER_NONE;
        if (other & WUPS_BUTTON_COMBO_CONTROLLER_VPAD_0) {
            res |= BUTTON_COMBO_MODULE_CONTROLLER_VPAD_0;
        }
        if (other & WUPS_BUTTON_COMBO_CONTROLLER_VPAD_1) {
            res |= BUTTON_COMBO_MODULE_CONTROLLER_VPAD_1;
        }
        if (other & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_0) {
            res |= BUTTON_COMBO_MODULE_CONTROLLER_WPAD_0;
        }
        if (other & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_1) {
            res |= BUTTON_COMBO_MODULE_CONTROLLER_WPAD_1;
        }
        if (other & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_2) {
            res |= BUTTON_COMBO_MODULE_CONTROLLER_WPAD_2;
        }
        if (other & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_3) {
            res |= BUTTON_COMBO_MODULE_CONTROLLER_WPAD_3;
        }
        if (other & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_4) {
            res |= BUTTON_COMBO_MODULE_CONTROLLER_WPAD_4;
        }
        if (other & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_5) {
            res |= BUTTON_COMBO_MODULE_CONTROLLER_WPAD_5;
        }
        if (other & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_6) {
            res |= BUTTON_COMBO_MODULE_CONTROLLER_WPAD_6;
        }
        return res;
    }

    WUPSButtonCombo_ControllerTypes convert(const ButtonComboModule_ControllerTypes other) {
        WUPSButtonCombo_ControllerTypes res = WUPS_BUTTON_COMBO_CONTROLLER_NONE;
        if (other & BUTTON_COMBO_MODULE_CONTROLLER_VPAD_0) {
            res |= WUPS_BUTTON_COMBO_CONTROLLER_VPAD_0;
        }
        if (other & BUTTON_COMBO_MODULE_CONTROLLER_VPAD_1) {
            res |= WUPS_BUTTON_COMBO_CONTROLLER_VPAD_1;
        }
        if (other & BUTTON_COMBO_MODULE_CONTROLLER_WPAD_0) {
            res |= WUPS_BUTTON_COMBO_CONTROLLER_WPAD_0;
        }
        if (other & BUTTON_COMBO_MODULE_CONTROLLER_WPAD_1) {
            res |= WUPS_BUTTON_COMBO_CONTROLLER_WPAD_1;
        }
        if (other & BUTTON_COMBO_MODULE_CONTROLLER_WPAD_2) {
            res |= WUPS_BUTTON_COMBO_CONTROLLER_WPAD_2;
        }
        if (other & BUTTON_COMBO_MODULE_CONTROLLER_WPAD_3) {
            res |= WUPS_BUTTON_COMBO_CONTROLLER_WPAD_3;
        }
        if (other & BUTTON_COMBO_MODULE_CONTROLLER_WPAD_4) {
            res |= WUPS_BUTTON_COMBO_CONTROLLER_WPAD_4;
        }
        if (other & BUTTON_COMBO_MODULE_CONTROLLER_WPAD_5) {
            res |= WUPS_BUTTON_COMBO_CONTROLLER_WPAD_5;
        }
        if (other & BUTTON_COMBO_MODULE_CONTROLLER_WPAD_6) {
            res |= WUPS_BUTTON_COMBO_CONTROLLER_WPAD_6;
        }
        return res;
    }

    ButtonComboModule_Buttons convert(const WUPSButtonCombo_Buttons other) {
        uint32_t res = 0;
        if (other & WUPS_BUTTON_COMBO_BUTTON_A) {
            res |= BCMPAD_BUTTON_A;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_B) {
            res |= BCMPAD_BUTTON_B;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_X) {
            res |= BCMPAD_BUTTON_X;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_Y) {
            res |= BCMPAD_BUTTON_Y;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_LEFT) {
            res |= BCMPAD_BUTTON_LEFT;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_RIGHT) {
            res |= BCMPAD_BUTTON_RIGHT;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_UP) {
            res |= BCMPAD_BUTTON_UP;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_DOWN) {
            res |= BCMPAD_BUTTON_DOWN;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_ZL) {
            res |= BCMPAD_BUTTON_ZL;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_ZR) {
            res |= BCMPAD_BUTTON_ZR;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_L) {
            res |= BCMPAD_BUTTON_L;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_R) {
            res |= BCMPAD_BUTTON_R;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_PLUS) {
            res |= BCMPAD_BUTTON_PLUS;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_MINUS) {
            res |= BCMPAD_BUTTON_MINUS;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_STICK_R) {
            res |= BCMPAD_BUTTON_STICK_R;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_STICK_L) {
            res |= BCMPAD_BUTTON_STICK_L;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_TV) {
            res |= BCMPAD_BUTTON_TV;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_RESERVED_BIT) {
            res |= BCMPAD_BUTTON_RESERVED_BIT;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_1) {
            res |= BCMPAD_BUTTON_1;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_2) {
            res |= BCMPAD_BUTTON_2;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_C) {
            res |= BCMPAD_BUTTON_C;
        }
        if (other & WUPS_BUTTON_COMBO_BUTTON_Z) {
            res |= BCMPAD_BUTTON_Z;
        }
        return static_cast<ButtonComboModule_Buttons>(res);
    }

    WUPSButtonCombo_Buttons convert(const ButtonComboModule_Buttons other) {
        uint32_t res = 0;
        if (other & BCMPAD_BUTTON_A) {
            res |= WUPS_BUTTON_COMBO_BUTTON_A;
        }
        if (other & BCMPAD_BUTTON_B) {
            res |= WUPS_BUTTON_COMBO_BUTTON_B;
        }
        if (other & BCMPAD_BUTTON_X) {
            res |= WUPS_BUTTON_COMBO_BUTTON_X;
        }
        if (other & BCMPAD_BUTTON_Y) {
            res |= WUPS_BUTTON_COMBO_BUTTON_Y;
        }
        if (other & BCMPAD_BUTTON_LEFT) {
            res |= WUPS_BUTTON_COMBO_BUTTON_LEFT;
        }
        if (other & BCMPAD_BUTTON_RIGHT) {
            res |= WUPS_BUTTON_COMBO_BUTTON_RIGHT;
        }
        if (other & BCMPAD_BUTTON_UP) {
            res |= WUPS_BUTTON_COMBO_BUTTON_UP;
        }
        if (other & BCMPAD_BUTTON_DOWN) {
            res |= WUPS_BUTTON_COMBO_BUTTON_DOWN;
        }
        if (other & BCMPAD_BUTTON_ZL) {
            res |= WUPS_BUTTON_COMBO_BUTTON_ZL;
        }
        if (other & BCMPAD_BUTTON_ZR) {
            res |= WUPS_BUTTON_COMBO_BUTTON_ZR;
        }
        if (other & BCMPAD_BUTTON_L) {
            res |= WUPS_BUTTON_COMBO_BUTTON_L;
        }
        if (other & BCMPAD_BUTTON_R) {
            res |= WUPS_BUTTON_COMBO_BUTTON_R;
        }
        if (other & BCMPAD_BUTTON_PLUS) {
            res |= WUPS_BUTTON_COMBO_BUTTON_PLUS;
        }
        if (other & BCMPAD_BUTTON_MINUS) {
            res |= WUPS_BUTTON_COMBO_BUTTON_MINUS;
        }
        if (other & BCMPAD_BUTTON_STICK_R) {
            res |= WUPS_BUTTON_COMBO_BUTTON_STICK_R;
        }
        if (other & BCMPAD_BUTTON_STICK_L) {
            res |= WUPS_BUTTON_COMBO_BUTTON_STICK_L;
        }
        if (other & BCMPAD_BUTTON_TV) {
            res |= WUPS_BUTTON_COMBO_BUTTON_TV;
        }
        if (other & BCMPAD_BUTTON_RESERVED_BIT) {
            res |= WUPS_BUTTON_COMBO_BUTTON_RESERVED_BIT;
        }
        if (other & BCMPAD_BUTTON_1) {
            res |= WUPS_BUTTON_COMBO_BUTTON_1;
        }
        if (other & BCMPAD_BUTTON_2) {
            res |= WUPS_BUTTON_COMBO_BUTTON_2;
        }
        if (other & BCMPAD_BUTTON_C) {
            res |= WUPS_BUTTON_COMBO_BUTTON_C;
        }
        if (other & BCMPAD_BUTTON_Z) {
            res |= WUPS_BUTTON_COMBO_BUTTON_Z;
        }
        return static_cast<WUPSButtonCombo_Buttons>(res);
    }

    ButtonComboModule_ComboType convertType(const WUPSButtonCombo_ComboType other) {
        switch (other) {
            case WUPS_BUTTON_COMBO_COMBO_TYPE_INVALID:
                return BUTTON_COMBO_MODULE_TYPE_INVALID;
            case WUPS_BUTTON_COMBO_COMBO_TYPE_HOLD:
                return BUTTON_COMBO_MODULE_TYPE_HOLD;
            case WUPS_BUTTON_COMBO_COMBO_TYPE_PRESS_DOWN:
                return BUTTON_COMBO_MODULE_TYPE_PRESS_DOWN;
            case WUPS_BUTTON_COMBO_COMBO_TYPE_HOLD_OBSERVER:
                return BUTTON_COMBO_MODULE_TYPE_HOLD_OBSERVER;
            case WUPS_BUTTON_COMBO_COMBO_TYPE_PRESS_DOWN_OBSERVER:
                return BUTTON_COMBO_MODULE_TYPE_PRESS_DOWN_OBSERVER;
        }
        return BUTTON_COMBO_MODULE_TYPE_INVALID;
    }

    WUPSButtonCombo_ComboType convertType(const ButtonComboModule_ComboType other) {
        switch (other) {
            case BUTTON_COMBO_MODULE_TYPE_INVALID:
                return WUPS_BUTTON_COMBO_COMBO_TYPE_INVALID;
            case BUTTON_COMBO_MODULE_TYPE_HOLD:
                return WUPS_BUTTON_COMBO_COMBO_TYPE_HOLD;
            case BUTTON_COMBO_MODULE_TYPE_PRESS_DOWN:
                return WUPS_BUTTON_COMBO_COMBO_TYPE_PRESS_DOWN;
            case BUTTON_COMBO_MODULE_TYPE_HOLD_OBSERVER:
                return WUPS_BUTTON_COMBO_COMBO_TYPE_HOLD_OBSERVER;
            case BUTTON_COMBO_MODULE_TYPE_PRESS_DOWN_OBSERVER:
                return WUPS_BUTTON_COMBO_COMBO_TYPE_PRESS_DOWN_OBSERVER;
        }
        return WUPS_BUTTON_COMBO_COMBO_TYPE_INVALID;
    }

    WUPSButtonCombo_ComboStatus convertStatus(const ButtonComboModule_ComboStatus other) {
        switch (other) {
            case BUTTON_COMBO_MODULE_COMBO_STATUS_INVALID_STATUS:
                return WUPS_BUTTON_COMBO_COMBO_STATUS_INVALID_STATUS;
            case BUTTON_COMBO_MODULE_COMBO_STATUS_VALID:
                return WUPS_BUTTON_COMBO_COMBO_STATUS_VALID;
            case BUTTON_COMBO_MODULE_COMBO_STATUS_CONFLICT:
                return WUPS_BUTTON_COMBO_COMBO_STATUS_CONFLICT;
        }
        return WUPS_BUTTON_COMBO_COMBO_STATUS_INVALID_STATUS;
    }

    ButtonComboModule_MetaOptions convert(const WUPSButtonCombo_MetaOptions &other) {
        ButtonComboModule_MetaOptions options;
        options.label = other.label;
        return options;
    }

    ButtonComboModule_ButtonComboInfoEx convert(const WUPSButtonCombo_ButtonComboInfoEx &other) {
        ButtonComboModule_ButtonComboInfoEx options;
        options.basicCombo.combo          = convert(other.basicCombo.combo);
        options.basicCombo.controllerMask = convert(other.basicCombo.controllerMask);
        options.type                      = convertType(other.type);
        options.optionalHoldForXMs        = other.optionalHoldForXMs;
        return options;
    }

    WUPSButtonCombo_ButtonComboInfoEx convert(const ButtonComboModule_ButtonComboInfoEx &other) {
        WUPSButtonCombo_ButtonComboInfoEx options;
        options.type                      = convertType(other.type);
        options.basicCombo.combo          = convert(other.basicCombo.combo);
        options.basicCombo.controllerMask = convert(other.basicCombo.controllerMask);
        options.optionalHoldForXMs        = other.optionalHoldForXMs;
        return options;
    }

    ButtonComboModule_ButtonComboOptions convert(const WUPSButtonCombo_ButtonComboOptions &other) {
        ButtonComboModule_ButtonComboOptions options;
        options.combo          = convert(other.combo);
        options.controllerMask = convert(other.controllerMask);
        return options;
    }

    ButtonComboModule_DetectButtonComboOptions convert(const WUPSButtonCombo_DetectButtonComboOptions &other) {
        ButtonComboModule_DetectButtonComboOptions options;
        options.controllerMask   = convert(other.controllerMask);
        options.abortButtonCombo = convert(other.abortButtonCombo);
        options.holdAbortForInMs = other.holdAbortForInMs;
        options.holdComboForInMs = other.holdComboForInMs;
        return options;
    }

    struct ComboCallbackWrapperData {
        WUPSButtonCombo_ComboCallback callback = nullptr;
        void *context                          = nullptr;
    };

    void ButtonComboCallbackWrapper(const ButtonComboModule_ControllerTypes triggeredBy, const ButtonComboModule_ComboHandle handle, void *context) {
        const auto *data = static_cast<ComboCallbackWrapperData *>(context);
        data->callback(convert(triggeredBy), WUPSButtonCombo_ComboHandle(handle.handle), data->context);
    }
} // namespace

class ButtonComboWrapper {
public:
    static std::optional<ButtonComboWrapper> Create(const WUPSButtonCombo_ComboOptions &otherOptions, WUPSButtonCombo_ComboStatus &outStatus, WUPSButtonCombo_Error &outError) {
        ButtonComboModule_ComboStatus status = BUTTON_COMBO_MODULE_COMBO_STATUS_INVALID_STATUS;
        auto contextData                     = std::make_unique<ComboCallbackWrapperData>(otherOptions.callbackOptions.callback, otherOptions.callbackOptions.context);
        ButtonComboModule_ComboOptions convertedOptions;
        convertedOptions.version            = BUTTON_COMBO_MODULE_COMBO_OPTIONS_VERSION;
        convertedOptions.metaOptions        = convert(otherOptions.metaOptions);
        convertedOptions.callbackOptions    = {.callback = ButtonComboCallbackWrapper, .context = contextData.get()};
        convertedOptions.buttonComboOptions = convert(otherOptions.buttonComboOptions);

        ButtonComboModule_Error err;
        auto res = ButtonComboModule::ButtonCombo::Create(convertedOptions, status, err);

        outError  = convertError(err);
        outStatus = convertStatus(status);

        if (!res || err != BUTTON_COMBO_MODULE_ERROR_SUCCESS) {
            return {};
        }
        return ButtonComboWrapper(std::move(*res), std::move(contextData));
    }


    ButtonComboWrapper(const ButtonComboWrapper &) = delete;
    ButtonComboWrapper &operator=(ButtonComboWrapper &src) = delete;

    ButtonComboWrapper(ButtonComboWrapper &&src) noexcept : mButtonComboModuleCombo(std::move(src.mButtonComboModuleCombo)),
                                                            mContextData(std::move(src.mContextData)),
                                                            mHandle(std::move(src.mHandle)) {
    }

    ButtonComboWrapper &operator=(ButtonComboWrapper &&src) noexcept {
        if (this != &src) {
            this->mButtonComboModuleCombo = std::move(src.mButtonComboModuleCombo);
            this->mContextData            = std::move(src.mContextData);
            this->mHandle                 = std::move(src.mHandle);
        }
        return *this;
    }

    [[nodiscard]] WUPSButtonCombo_ComboHandle getHandle() const {
        return WUPSButtonCombo_ComboHandle(reinterpret_cast<void *>(*mHandle));
    }

    WUPSButtonCombo_Error GetButtonComboStatus(WUPSButtonCombo_ComboStatus &outStatus) const {
        ButtonComboModule_ComboStatus status = BUTTON_COMBO_MODULE_COMBO_STATUS_INVALID_STATUS;
        const auto res                       = mButtonComboModuleCombo.GetButtonComboStatus(status);
        if (res == BUTTON_COMBO_MODULE_ERROR_SUCCESS) {
            outStatus = convertStatus(status);
        }
        return convertError(res);
    }

    [[nodiscard]] WUPSButtonCombo_Error UpdateButtonComboMeta(const WUPSButtonCombo_MetaOptions &metaOptions) const {
        const auto convertedOptions = convert(metaOptions);
        return convertError(mButtonComboModuleCombo.UpdateButtonComboMeta(convertedOptions));
    }

    [[nodiscard]] WUPSButtonCombo_Error UpdateButtonComboCallback(const WUPSButtonCombo_CallbackOptions &callbackOptions) const {
        mContextData->callback = callbackOptions.callback;
        mContextData->context  = callbackOptions.context;
        return WUPS_BUTTON_COMBO_ERROR_SUCCESS;
    }

    [[nodiscard]] WUPSButtonCombo_Error UpdateControllerMask(const WUPSButtonCombo_ControllerTypes controllerMask,
                                                             WUPSButtonCombo_ComboStatus &outStatus) const {
        ButtonComboModule_ComboStatus comboStatus = BUTTON_COMBO_MODULE_COMBO_STATUS_INVALID_STATUS;
        const auto res                            = convertError(mButtonComboModuleCombo.UpdateControllerMask(convert(controllerMask), comboStatus));
        outStatus                                 = convertStatus(comboStatus);
        return res;
    }

    [[nodiscard]] WUPSButtonCombo_Error UpdateButtonCombo(const WUPSButtonCombo_Buttons combo,
                                                          WUPSButtonCombo_ComboStatus &outStatus) const {
        ButtonComboModule_ComboStatus comboStatus = BUTTON_COMBO_MODULE_COMBO_STATUS_INVALID_STATUS;
        const auto res                            = convertError(mButtonComboModuleCombo.UpdateButtonCombo(convert(combo), comboStatus));
        outStatus                                 = convertStatus(comboStatus);
        return res;
    }

    [[nodiscard]] WUPSButtonCombo_Error UpdateHoldDuration(const uint32_t holdDurationInMs) const {
        return convertError(mButtonComboModuleCombo.UpdateHoldDuration(holdDurationInMs));
    }

    [[nodiscard]] WUPSButtonCombo_Error GetButtonComboMeta(WUPSButtonCombo_MetaOptionsOut &outOptions) const {
        ButtonComboModule_MetaOptionsOut options = {.labelBuffer = outOptions.labelBuffer, .labelBufferLength = outOptions.labelBufferLength};
        return convertError(mButtonComboModuleCombo.GetButtonComboMeta(options));
    }

    WUPSButtonCombo_Error GetButtonComboCallback(WUPSButtonCombo_CallbackOptions &outOptions) const {
        outOptions.callback = mContextData->callback;
        outOptions.context  = mContextData->context;

        return WUPS_BUTTON_COMBO_ERROR_SUCCESS;
    }

    WUPSButtonCombo_Error GetButtonComboInfoEx(WUPSButtonCombo_ButtonComboInfoEx &outOptions) const {
        ButtonComboModule_ButtonComboInfoEx tmpOptions;
        const auto res = convertError(mButtonComboModuleCombo.GetButtonComboInfoEx(tmpOptions));
        outOptions     = convert(tmpOptions);
        return res;
    }

private:
    ButtonComboWrapper(ButtonComboModule::ButtonCombo combo, std::unique_ptr<ComboCallbackWrapperData> contextData) : mButtonComboModuleCombo(std::move(combo)),
                                                                                                                      mContextData(std::move(contextData)) {
        // Abuse this as a stable handle that references itself and survives std::move
        *mHandle = reinterpret_cast<uint32_t>(mHandle.get());
    }

    ButtonComboModule::ButtonCombo mButtonComboModuleCombo;
    std::unique_ptr<ComboCallbackWrapperData> mContextData;
    std::unique_ptr<uint32_t> mHandle = std::make_unique<uint32_t>();
};

ButtonComboManager::ButtonComboManager() {
    // Abuse this as a stable handle that references itself and survives std::move
    *mHandle = reinterpret_cast<uint32_t>(mHandle.get());
}

ButtonComboManager::~ButtonComboManager() = default;

ButtonComboManager::ButtonComboManager(ButtonComboManager &&src) : mComboWrappers(std::move(src.mComboWrappers)), mHandle(std::move(src.mHandle)) {
}

ButtonComboManager &ButtonComboManager::operator=(ButtonComboManager &&src) {
    if (this != &src) {
        this->mHandle        = std::move(src.mHandle);
        this->mComboWrappers = std::move(src.mComboWrappers);
    }
    return *this;
}

WUPSButtonCombo_Error ButtonComboManager::AddButtonComboHandle(const WUPSButtonCombo_ComboOptions &options,
                                                               WUPSButtonCombo_ComboHandle &outHandle,
                                                               WUPSButtonCombo_ComboStatus &outStatus) {
    WUPSButtonCombo_Error err = WUPS_BUTTON_COMBO_ERROR_UNKNOWN_ERROR;
    auto comboOpt             = ButtonComboWrapper::Create(options, outStatus, err);
    if (!comboOpt || err != WUPS_BUTTON_COMBO_ERROR_SUCCESS) {
        return err;
    }

    outHandle = comboOpt->getHandle();

    mComboWrappers.emplace_front(std::move(*comboOpt));

    return WUPS_BUTTON_COMBO_ERROR_SUCCESS;
}

WUPSButtonCombo_Error ButtonComboManager::RemoveButtonCombo(const WUPSButtonCombo_ComboHandle handle) {
    if (remove_first_if(mComboWrappers, [&handle](const auto &comboWrapper) { return comboWrapper.getHandle() == handle; })) {
        // Destructor removes it from the button combo module
        return WUPS_BUTTON_COMBO_ERROR_SUCCESS;
    }
    DEBUG_FUNCTION_LINE_WARN("Could not remove button combo: not found! %08X", handle);

    return WUPS_BUTTON_COMBO_ERROR_NOT_FOUND;
}

WUPSButtonCombo_Error ButtonComboManager::GetButtonComboStatus(const WUPSButtonCombo_ComboHandle handle,
                                                               WUPSButtonCombo_ComboStatus &outStatus) {
    return ExecuteForWrapper(handle, [&](const ButtonComboWrapper &wrapper) {
        return wrapper.GetButtonComboStatus(outStatus);
    });
}

WUPSButtonCombo_Error ButtonComboManager::UpdateButtonComboMeta(const WUPSButtonCombo_ComboHandle handle,
                                                                const WUPSButtonCombo_MetaOptions &metaOptions) {
    return ExecuteForWrapper(handle, [&](const ButtonComboWrapper &wrapper) {
        return wrapper.UpdateButtonComboMeta(metaOptions);
    });
}

WUPSButtonCombo_Error ButtonComboManager::UpdateButtonComboCallback(const WUPSButtonCombo_ComboHandle handle,
                                                                    const WUPSButtonCombo_CallbackOptions &callbackOptions) {
    return ExecuteForWrapper(handle, [&](const ButtonComboWrapper &wrapper) {
        return wrapper.UpdateButtonComboCallback(callbackOptions);
    });
}

WUPSButtonCombo_Error ButtonComboManager::UpdateControllerMask(const WUPSButtonCombo_ComboHandle handle,
                                                               const WUPSButtonCombo_ControllerTypes controllerMask,
                                                               WUPSButtonCombo_ComboStatus &outStatus) {
    return ExecuteForWrapper(handle, [&](const ButtonComboWrapper &wrapper) {
        return wrapper.UpdateControllerMask(controllerMask, outStatus);
    });
}

WUPSButtonCombo_Error ButtonComboManager::UpdateButtonCombo(const WUPSButtonCombo_ComboHandle handle,
                                                            const WUPSButtonCombo_Buttons combo,
                                                            WUPSButtonCombo_ComboStatus &outStatus) {
    return ExecuteForWrapper(handle, [&](const ButtonComboWrapper &wrapper) {
        return wrapper.UpdateButtonCombo(combo, outStatus);
    });
}

WUPSButtonCombo_Error ButtonComboManager::UpdateHoldDuration(const WUPSButtonCombo_ComboHandle handle,
                                                             const uint32_t holdDurationInMs) {
    return ExecuteForWrapper(handle, [&](const ButtonComboWrapper &wrapper) {
        return wrapper.UpdateHoldDuration(holdDurationInMs);
    });
}

WUPSButtonCombo_Error ButtonComboManager::GetButtonComboMeta(const WUPSButtonCombo_ComboHandle handle,
                                                             WUPSButtonCombo_MetaOptionsOut &outOptions) {
    return ExecuteForWrapper(handle, [&](const ButtonComboWrapper &wrapper) {
        return wrapper.GetButtonComboMeta(outOptions);
    });
}

WUPSButtonCombo_Error ButtonComboManager::GetButtonComboCallback(const WUPSButtonCombo_ComboHandle handle,
                                                                 WUPSButtonCombo_CallbackOptions &outOptions) {
    return ExecuteForWrapper(handle, [&](const ButtonComboWrapper &wrapper) {
        return wrapper.GetButtonComboCallback(outOptions);
    });
}

WUPSButtonCombo_Error ButtonComboManager::GetButtonComboInfoEx(const WUPSButtonCombo_ComboHandle handle,
                                                               WUPSButtonCombo_ButtonComboInfoEx &outOptions) {
    return ExecuteForWrapper(handle, [&](const ButtonComboWrapper &wrapper) {
        return wrapper.GetButtonComboInfoEx(outOptions);
    });
}

WUPSButtonCombo_Error ButtonComboManager::CheckComboAvailable(const WUPSButtonCombo_ButtonComboOptions &options,
                                                              WUPSButtonCombo_ComboStatus &outStatus) {
    const auto convertedOptions          = convert(options);
    ButtonComboModule_ComboStatus status = BUTTON_COMBO_MODULE_COMBO_STATUS_INVALID_STATUS;
    const auto res                       = convertError(ButtonComboModule::CheckComboAvailable(convertedOptions, status));
    outStatus                            = convertStatus(status);
    return res;
}

WUPSButtonCombo_Error ButtonComboManager::DetectButtonCombo_Blocking(const WUPSButtonCombo_DetectButtonComboOptions &options, WUPSButtonCombo_Buttons &outButtonCombo) {
    const auto convertedOptions = convert(options);
    auto combo                  = static_cast<ButtonComboModule_Buttons>(0);
    const auto res              = convertError(ButtonComboModule::DetectButtonCombo_Blocking(convertedOptions, combo));
    outButtonCombo              = convert(combo);
    return res;
}

WUPSButtonCombo_Error ButtonComboManager::ExecuteForWrapper(const WUPSButtonCombo_ComboHandle &handle, const std::function<WUPSButtonCombo_Error(ButtonComboWrapper &)> &callback) {
    if (handle == nullptr) {
        return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
    }
    for (auto &wrapper : mComboWrappers) {
        if (wrapper.getHandle() == handle) {
            return callback(wrapper);
        }
    }
    return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
}

uint32_t ButtonComboManager::getHandle() const {
    return *mHandle;
}