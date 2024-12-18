#pragma once

#include <buttoncombo/defines.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include <wups/button_combo.h>


class ButtonComboWrapper;

class ButtonComboManager {
public:
    explicit ButtonComboManager();
    ~ButtonComboManager();


    ButtonComboManager(const ButtonComboManager &) = delete;

    ButtonComboManager(ButtonComboManager &&src) noexcept;

    ButtonComboManager &operator=(ButtonComboManager &&src) noexcept;

    WUPSButtonCombo_Error AddButtonComboHandle(const WUPSButtonCombo_ComboOptions &options,
                                               WUPSButtonCombo_ComboHandle &outHandle,
                                               WUPSButtonCombo_ComboStatus &outStatus);

    WUPSButtonCombo_Error RemoveButtonCombo(WUPSButtonCombo_ComboHandle handle);

    WUPSButtonCombo_Error GetButtonComboStatus(WUPSButtonCombo_ComboHandle handle,
                                               WUPSButtonCombo_ComboStatus &outStatus);

    WUPSButtonCombo_Error UpdateButtonComboMeta(WUPSButtonCombo_ComboHandle handle,
                                                const WUPSButtonCombo_MetaOptions &metaOptions);

    WUPSButtonCombo_Error UpdateButtonComboCallback(WUPSButtonCombo_ComboHandle handle,
                                                    const WUPSButtonCombo_CallbackOptions &callbackOptions);

    WUPSButtonCombo_Error UpdateControllerMask(WUPSButtonCombo_ComboHandle handle,
                                               WUPSButtonCombo_ControllerTypes controllerMask,
                                               WUPSButtonCombo_ComboStatus &outStatus);

    WUPSButtonCombo_Error UpdateButtonCombo(WUPSButtonCombo_ComboHandle handle,
                                            WUPSButtonCombo_Buttons combo,
                                            WUPSButtonCombo_ComboStatus &outStatus);

    WUPSButtonCombo_Error UpdateHoldDuration(WUPSButtonCombo_ComboHandle handle,
                                             uint32_t holdDurationInFrames);

    WUPSButtonCombo_Error GetButtonComboMeta(WUPSButtonCombo_ComboHandle handle,
                                             WUPSButtonCombo_MetaOptionsOut &outOptions);

    WUPSButtonCombo_Error GetButtonComboCallback(WUPSButtonCombo_ComboHandle handle,
                                                 WUPSButtonCombo_CallbackOptions &outOptions);

    WUPSButtonCombo_Error GetButtonComboInfoEx(WUPSButtonCombo_ComboHandle handle,
                                               WUPSButtonCombo_ButtonComboInfoEx &outOptions);

    WUPSButtonCombo_Error CheckComboAvailable(const WUPSButtonCombo_ButtonComboOptions &options,
                                              WUPSButtonCombo_ComboStatus &outStatus);

    WUPSButtonCombo_Error DetectButtonCombo_Blocking(const WUPSButtonCombo_DetectButtonComboOptions &options,
                                                     WUPSButtonCombo_Buttons &outButtonCombo);

    WUPSButtonCombo_Error ExecuteForWrapper(const WUPSButtonCombo_ComboHandle &handle, const std::function<WUPSButtonCombo_Error(ButtonComboWrapper &)> &callback);

    [[nodiscard]] uint32_t getHandle() const;

private:
    std::vector<ButtonComboWrapper> mComboWrappers;
    std::unique_ptr<uint32_t> mHandle = std::make_unique<uint32_t>();
};
