#include "ButtonComboUtils.h"

#include <forward_list>
#include <mutex>
#include <plugin/ButtonComboManager.h>
#include <plugin/PluginContainer.h>
#include <utils/logger.h>
#include <utils/utils.h>

namespace ButtonComboUtils::API {
    static std::forward_list<ButtonComboManager> sButtonComboManager;
    static std::mutex sButtonComboMutex;

    namespace Internal {
        uint32_t CreateButtonComboData() {
            std::lock_guard lock(sButtonComboMutex);
            sButtonComboManager.emplace_front();
            return sButtonComboManager.front().getHandle();
        }

        void RemoveButtonComboData(uint32_t buttonComboManagerHandle) {
            if (!remove_locked_first_if(sButtonComboMutex, sButtonComboManager, [buttonComboManagerHandle](const ButtonComboManager &buttonComboData) { return buttonComboData.getHandle() == buttonComboManagerHandle; })) {
                DEBUG_FUNCTION_LINE_WARN("Tried to remove ButtonComboManager by invalid handle: %08X", buttonComboManagerHandle);
            }
        }
    } // namespace Internal

    namespace {
        WUPSButtonCombo_Error ExecuteForIdentifierLocked(void *identifier, const std::function<WUPSButtonCombo_Error(ButtonComboManager &)> &callback) {
            if (identifier == nullptr) {
                return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
            }
            std::lock_guard lock(sButtonComboMutex);
            for (auto &manager : sButtonComboManager) {
                if (manager.getHandle() == reinterpret_cast<uint32_t>(identifier)) {
                    return callback(manager);
                }
            }
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
    } // namespace

    WUPSButtonCombo_Error AddButtonCombo(void *identifier,
                                         const WUPSButtonCombo_ComboOptions *options,
                                         WUPSButtonCombo_ComboHandle *outHandle,
                                         WUPSButtonCombo_ComboStatus *outStatus) {
        if (options == nullptr || outHandle == nullptr) {
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
        WUPSButtonCombo_ComboStatus tmpStatus;
        const auto res = ExecuteForIdentifierLocked(identifier,
                                                    [&](ButtonComboManager &manager) {
                                                        return manager.AddButtonComboHandle(*options, *outHandle, tmpStatus);
                                                    });
        if (outStatus) { *outStatus = tmpStatus; }
        return res;
    }

    WUPSButtonCombo_Error RemoveButtonCombo(void *identifier,
                                            const WUPSButtonCombo_ComboHandle handle) {
        if (identifier == nullptr || handle == nullptr) {
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
        return ExecuteForIdentifierLocked(identifier,
                                          [&](ButtonComboManager &manager) {
                                              return manager.RemoveButtonCombo(handle);
                                          });
    }

    WUPSButtonCombo_Error GetButtonComboStatus(void *identifier,
                                               const WUPSButtonCombo_ComboHandle handle,
                                               WUPSButtonCombo_ComboStatus *outStatus) {
        if (identifier == nullptr || handle == nullptr || outStatus == nullptr) {
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
        return ExecuteForIdentifierLocked(identifier,
                                          [&](ButtonComboManager &manager) {
                                              return manager.GetButtonComboStatus(handle, *outStatus);
                                          });
    }

    WUPSButtonCombo_Error UpdateButtonComboMeta(void *identifier,
                                                const WUPSButtonCombo_ComboHandle handle,
                                                const WUPSButtonCombo_MetaOptions *metaOptions) {
        if (metaOptions == nullptr) {
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
        return ExecuteForIdentifierLocked(identifier,
                                          [&](ButtonComboManager &manager) {
                                              return manager.UpdateButtonComboMeta(handle, *metaOptions);
                                          });
    }

    WUPSButtonCombo_Error UpdateButtonComboCallback(void *identifier,
                                                    const WUPSButtonCombo_ComboHandle handle,
                                                    const WUPSButtonCombo_CallbackOptions *callbackOptions) {
        if (callbackOptions == nullptr) {
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
        return ExecuteForIdentifierLocked(identifier,
                                          [&](ButtonComboManager &manager) {
                                              return manager.UpdateButtonComboCallback(handle, *callbackOptions);
                                          });
    }

    WUPSButtonCombo_Error UpdateControllerMask(void *identifier,
                                               const WUPSButtonCombo_ComboHandle handle,
                                               const WUPSButtonCombo_ControllerTypes controllerMask,
                                               WUPSButtonCombo_ComboStatus *outStatus) {
        WUPSButtonCombo_ComboStatus tmpStatus;
        const auto res = ExecuteForIdentifierLocked(identifier,
                                                    [&](ButtonComboManager &manager) {
                                                        return manager.UpdateControllerMask(handle, controllerMask, tmpStatus);
                                                    });
        if (outStatus) { *outStatus = tmpStatus; }
        return res;
    }

    WUPSButtonCombo_Error UpdateButtonCombo(void *identifier,
                                            const WUPSButtonCombo_ComboHandle handle,
                                            WUPSButtonCombo_Buttons combo,
                                            WUPSButtonCombo_ComboStatus *outStatus) {
        WUPSButtonCombo_ComboStatus tmpStatus;
        const auto res = ExecuteForIdentifierLocked(identifier,
                                                    [&](ButtonComboManager &manager) {
                                                        return manager.UpdateButtonCombo(handle, combo, tmpStatus);
                                                    });
        if (outStatus) { *outStatus = tmpStatus; }
        return res;
    }

    WUPSButtonCombo_Error UpdateHoldDuration(void *identifier,
                                             const WUPSButtonCombo_ComboHandle handle,
                                             const uint32_t holdDurationInFrames) {
        const auto res = ExecuteForIdentifierLocked(identifier,
                                                    [&](ButtonComboManager &manager) {
                                                        return manager.UpdateHoldDuration(handle, holdDurationInFrames);
                                                    });
        return res;
    }

    WUPSButtonCombo_Error GetButtonComboMeta(void *identifier,
                                             const WUPSButtonCombo_ComboHandle handle,
                                             WUPSButtonCombo_MetaOptionsOut *outOptions) {
        if (outOptions == nullptr) {
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
        return ExecuteForIdentifierLocked(identifier,
                                          [&](ButtonComboManager &manager) {
                                              return manager.GetButtonComboMeta(handle, *outOptions);
                                          });
    }
    WUPSButtonCombo_Error GetButtonComboCallback(void *identifier,
                                                 const WUPSButtonCombo_ComboHandle handle,
                                                 WUPSButtonCombo_CallbackOptions *outOptions) {

        if (outOptions == nullptr) {
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
        return ExecuteForIdentifierLocked(identifier,
                                          [&](ButtonComboManager &manager) {
                                              return manager.GetButtonComboCallback(handle, *outOptions);
                                          });
    }

    WUPSButtonCombo_Error GetButtonComboInfoEx(void *identifier,
                                               const WUPSButtonCombo_ComboHandle handle,
                                               WUPSButtonCombo_ButtonComboInfoEx *outOptions) {
        if (outOptions == nullptr) {
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
        return ExecuteForIdentifierLocked(identifier,
                                          [&](ButtonComboManager &manager) {
                                              return manager.GetButtonComboInfoEx(handle, *outOptions);
                                          });
    }

    WUPSButtonCombo_Error CheckComboAvailable(void *identifier,
                                              const WUPSButtonCombo_ButtonComboOptions *options,
                                              WUPSButtonCombo_ComboStatus *outStatus) {
        if (outStatus == nullptr) {
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
        return ExecuteForIdentifierLocked(identifier,
                                          [&](ButtonComboManager &manager) {
                                              return manager.CheckComboAvailable(*options, *outStatus);
                                          });
    }
    WUPSButtonCombo_Error DetectButtonCombo_Blocking(void *identifier,
                                                     const WUPSButtonCombo_DetectButtonComboOptions *options,
                                                     WUPSButtonCombo_Buttons *outButtonCombo) {
        if (options == nullptr || outButtonCombo == nullptr) {
            return WUPS_BUTTON_COMBO_ERROR_INVALID_ARGS;
        }
        return ExecuteForIdentifierLocked(identifier,
                                          [&](ButtonComboManager &manager) {
                                              return manager.DetectButtonCombo_Blocking(*options, *outButtonCombo);
                                          });
    }
} // namespace ButtonComboUtils::API