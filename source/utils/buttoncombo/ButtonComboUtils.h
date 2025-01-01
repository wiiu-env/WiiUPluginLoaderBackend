#pragma once

#include <wups/button_combo.h>

#include <cstdint>

namespace ButtonComboUtils::API {
    namespace Internal {
        uint32_t CreateButtonComboData();
        void RemoveButtonComboData(uint32_t buttonComboManagerHandle);
    } // namespace Internal

    WUPSButtonCombo_Error AddButtonCombo(void *identifier,
                                         const WUPSButtonCombo_ComboOptions *options,
                                         WUPSButtonCombo_ComboHandle *outHandle,
                                         WUPSButtonCombo_ComboStatus *outStatus);

    WUPSButtonCombo_Error RemoveButtonCombo(void *identifier,
                                            WUPSButtonCombo_ComboHandle handle);

    WUPSButtonCombo_Error GetButtonComboStatus(void *identifier,
                                               WUPSButtonCombo_ComboHandle handle,
                                               WUPSButtonCombo_ComboStatus *outStatus);

    WUPSButtonCombo_Error UpdateButtonComboMeta(void *identifier,
                                                WUPSButtonCombo_ComboHandle handle,
                                                const WUPSButtonCombo_MetaOptions *metaOptions);

    WUPSButtonCombo_Error UpdateButtonComboCallback(void *identifier,
                                                    WUPSButtonCombo_ComboHandle handle,
                                                    const WUPSButtonCombo_CallbackOptions *callbackOptions);

    WUPSButtonCombo_Error UpdateControllerMask(void *identifier,
                                               WUPSButtonCombo_ComboHandle handle,
                                               WUPSButtonCombo_ControllerTypes controllerMask,
                                               WUPSButtonCombo_ComboStatus *outStatus);

    WUPSButtonCombo_Error UpdateButtonCombo(void *identifier,
                                            WUPSButtonCombo_ComboHandle handle,
                                            WUPSButtonCombo_Buttons combo,
                                            WUPSButtonCombo_ComboStatus *outStatus);

    WUPSButtonCombo_Error UpdateHoldDuration(void *identifier,
                                             WUPSButtonCombo_ComboHandle handle,
                                             uint32_t holdDurationInMs);

    WUPSButtonCombo_Error GetButtonComboMeta(void *identifier,
                                             WUPSButtonCombo_ComboHandle handle,
                                             WUPSButtonCombo_MetaOptionsOut *outOptions);

    WUPSButtonCombo_Error GetButtonComboCallback(void *identifier,
                                                 WUPSButtonCombo_ComboHandle handle,
                                                 WUPSButtonCombo_CallbackOptions *outOptions);

    WUPSButtonCombo_Error GetButtonComboInfoEx(void *identifier,
                                               WUPSButtonCombo_ComboHandle handle,
                                               WUPSButtonCombo_ButtonComboInfoEx *outOptions);

    WUPSButtonCombo_Error CheckComboAvailable(void *identifier,
                                              const WUPSButtonCombo_ButtonComboOptions *options,
                                              WUPSButtonCombo_ComboStatus *outStatus);

    WUPSButtonCombo_Error DetectButtonCombo_Blocking(void *identifier,
                                                     const WUPSButtonCombo_DetectButtonComboOptions *options,
                                                     WUPSButtonCombo_Buttons *outButtonCombo);

} // namespace ButtonComboUtils::API
