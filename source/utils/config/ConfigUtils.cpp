#include "ConfigUtils.h"
#include "ConfigRenderer.h"
#include "config/WUPSConfigAPI.h"
#include "hooks.h"
#include "utils/DrawUtils.h"
#include "utils/dc.h"
#include "utils/input/CombinedInput.h"
#include "utils/input/Input.h"
#include "utils/input/VPADInput.h"
#include "utils/input/WPADInput.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "version.h"

#include <algorithm>
#include <globals.h>
#include <memory/mappedmemory.h>
#include <memory>
#include <vector>
#include <wups/config.h>

WUPS_CONFIG_SIMPLE_INPUT ConfigUtils::convertInputs(const uint32_t buttons) {
    WUPSConfigButtons pressedButtons = WUPS_CONFIG_BUTTON_NONE;
    if (buttons & Input::eButtons::BUTTON_A) {
        pressedButtons |= WUPS_CONFIG_BUTTON_A;
    }
    if (buttons & Input::eButtons::BUTTON_LEFT) {
        pressedButtons |= WUPS_CONFIG_BUTTON_LEFT;
    }
    if (buttons & Input::eButtons::BUTTON_RIGHT) {
        pressedButtons |= WUPS_CONFIG_BUTTON_RIGHT;
    }
    if (buttons & Input::eButtons::BUTTON_L) {
        pressedButtons |= WUPS_CONFIG_BUTTON_L;
    }
    if (buttons & Input::eButtons::BUTTON_R) {
        pressedButtons |= WUPS_CONFIG_BUTTON_R;
    }
    if (buttons & Input::eButtons::BUTTON_ZL) {
        pressedButtons |= WUPS_CONFIG_BUTTON_ZL;
    }
    if (buttons & Input::eButtons::BUTTON_ZR) {
        pressedButtons |= WUPS_CONFIG_BUTTON_ZR;
    }
    if (buttons & Input::eButtons::BUTTON_X) {
        pressedButtons |= WUPS_CONFIG_BUTTON_X;
    }
    if (buttons & Input::eButtons::BUTTON_Y) {
        pressedButtons |= WUPS_CONFIG_BUTTON_Y;
    }
    if (buttons & Input::eButtons::BUTTON_STICK_L) {
        pressedButtons |= WUPS_CONFIG_BUTTON_STICK_L;
    }
    if (buttons & Input::eButtons::BUTTON_STICK_R) {
        pressedButtons |= WUPS_CONFIG_BUTTON_STICK_R;
    }
    if (buttons & Input::eButtons::BUTTON_PLUS) {
        pressedButtons |= WUPS_CONFIG_BUTTON_PLUS;
    }
    if (buttons & Input::eButtons::BUTTON_MINUS) {
        pressedButtons |= WUPS_CONFIG_BUTTON_MINUS;
    }
    if (buttons & Input::eButtons::BUTTON_B) {
        pressedButtons |= WUPS_CONFIG_BUTTON_B;
    }
    if (buttons & Input::eButtons::BUTTON_UP) {
        pressedButtons |= WUPS_CONFIG_BUTTON_UP;
    }
    if (buttons & Input::eButtons::BUTTON_DOWN) {
        pressedButtons |= WUPS_CONFIG_BUTTON_DOWN;
    }
    return static_cast<WUPS_CONFIG_SIMPLE_INPUT>(pressedButtons);
}

void ConfigUtils::displayMenu() {
    renderBasicScreen("Loading configs...");

    std::vector<ConfigDisplayItem> configs;
    for (const auto &plugin : gLoadedPlugins) {
        GeneralConfigInformation info;
        info.name       = plugin.getMetaInformation().getName();
        info.author     = plugin.getMetaInformation().getAuthor();
        info.version    = plugin.getMetaInformation().getVersion();
        info.pluginData = plugin.getPluginDataCopy();

        std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> config;

        if (plugin.isLinkedAndLoaded()) {
            if (const auto configData = plugin.getConfigData()) {
                if (const auto configHandleOpt = configData->createConfig()) {
                    WUPSConfigAPIStatus callbackResult = configData->CallMenuOpenedCallback(configHandleOpt.value());
                    config                             = WUPSConfigAPIBackend::Intern::PopConfigByHandle(configHandleOpt.value());
                    if (!config) {
                        DEBUG_FUNCTION_LINE_ERR("Failed to get config for handle: %08X", configHandleOpt.value().handle);
                    } else if (callbackResult != WUPSCONFIG_API_RESULT_SUCCESS) {
                        DEBUG_FUNCTION_LINE_ERR("Callback failed for %s: %s", info.name.c_str(), WUPSConfigAPI_GetStatusStr(callbackResult));
                        config.reset();
                    } else {
                        info.name = config->getName();
                    }
                } else {
                    DEBUG_FUNCTION_LINE_ERR("Failed to create config for plugin: \"%s\"", info.name.c_str());
                }
            } else {
                for (const auto &hook : plugin.getPluginLinkInformation().getHookDataList()) {
                    if (hook.getType() == WUPS_LOADER_HOOK_GET_CONFIG_DEPRECATED) {
                        if (hook.getFunctionPointer() == nullptr) {
                            DEBUG_FUNCTION_LINE_ERR("Hook had invalid ptr");
                            break;
                        }
                        auto cur_config_handle = ((void *(*) ())((uint32_t *) hook.getFunctionPointer()))();
                        if (cur_config_handle == nullptr) {
                            DEBUG_FUNCTION_LINE_WARN("Hook returned empty handle");
                            break;
                        }
                        config = WUPSConfigAPIBackend::Intern::PopConfigByHandle(WUPSConfigHandle(cur_config_handle));
                        if (!config) {
                            DEBUG_FUNCTION_LINE_ERR("Failed to find config for handle: %08X", cur_config_handle);
                        }
                        break;
                    }
                }
            }
        }
        if (!config) {
            config = make_unique_nothrow<WUPSConfigAPIBackend::WUPSConfig>(info.name);
        }

        configs.emplace_back(info, std::move(config), plugin.isLinkedAndLoaded());
    }

    // Sort Configs by name
    std::ranges::sort(configs,
                      [](const ConfigDisplayItem &lhs, const ConfigDisplayItem &rhs) {
                          auto &str1 = lhs.getConfigInformation().name;
                          auto &str2 = rhs.getConfigInformation().name;
                          return std::ranges::lexicographical_compare(str1, str2,
                                                                      [](const char &char1, const char &char2) {
                                                                          return tolower(char1) < tolower(char2);
                                                                      });
                      });

    ConfigRenderer renderer(std::move(configs));
    configs.clear();

    CombinedInput baseInput;
    VPadInput vpadInput;
    WPADInput wpadInputs[7] = {
            WPAD_CHAN_0,
            WPAD_CHAN_1,
            WPAD_CHAN_2,
            WPAD_CHAN_3,
            WPAD_CHAN_4,
            WPAD_CHAN_5,
            WPAD_CHAN_6,
    };

    OSTime startTime;
    bool skipFirstInput = true;

    gOnlyAcceptFromThread = OSGetCurrentThread();
    while (true) {
        startTime = OSGetTime();
        if (gConfigMenuShouldClose) {
            gConfigMenuShouldClose = false;
            break;
        }
        baseInput.reset();
        if (vpadInput.update(1280, 720)) {
            baseInput.combine(vpadInput);
        }
        for (auto &wpadInput : wpadInputs) {
            if (wpadInput.update(1280, 720)) {
                baseInput.combine(wpadInput);
            }
        }

        if (skipFirstInput) {
            skipFirstInput     = false;
            baseInput.lastData = baseInput.data;
        }

        WUPSConfigSimplePadData simpleData;
        simpleData.buttons_d    = convertInputs(baseInput.data.buttons_d);
        simpleData.buttons_r    = convertInputs(baseInput.data.buttons_r);
        simpleData.buttons_h    = convertInputs(baseInput.data.buttons_h);
        simpleData.x            = baseInput.data.x;
        simpleData.y            = baseInput.data.y;
        simpleData.touched      = baseInput.data.touched;
        simpleData.validPointer = baseInput.data.validPointer;

        WUPSConfigComplexPadData complexData;
        complexData.vpad.data      = vpadInput.vpad;
        complexData.vpad.tpCalib   = vpadInput.tpCalib;
        complexData.vpad.vpadError = vpadInput.vpadError;
        for (int i = 0; i < 7; i++) {
            complexData.kpad.kpadError[i] = wpadInputs[i].kpadError;
            complexData.kpad.data[i]      = wpadInputs[i].kpad;
        }

        auto subState = renderer.Update(baseInput, simpleData, complexData);
        if (subState != SUB_STATE_RUNNING) {
            break;
        }
        if (renderer.NeedsRedraw() || baseInput.data.buttons_d || baseInput.data.buttons_r) {
            renderer.Render();
        }
        renderer.ResetNeedsRedraw();

        auto diffTime = OSTicksToMicroseconds(OSGetTime() - startTime);
        if (diffTime < 16000) {
            OSSleepTicks(OSMicrosecondsToTicks(16000 - diffTime));
        }
    }

    startTime = OSGetTime();
    renderBasicScreen("Saving configs...");

    for (const auto &plugin : gLoadedPlugins) {
        const auto configData = plugin.getConfigData();
        if (configData) {
            if (configData->CallMenuClosedCallback() == WUPSCONFIG_API_RESULT_MISSING_CALLBACK) {
                DEBUG_FUNCTION_LINE_WARN("CallMenuClosedCallback is missing for %s", plugin.getMetaInformation().getName().c_str());
            }
        } else {
            CallHook(plugin, WUPS_LOADER_HOOK_CONFIG_CLOSED_DEPRECATED);
        }
    }

    WUPSConfigAPIBackend::Intern::CleanAllHandles();

    // we want wait at least 300ms to avoid leaking inputs from the config menu to the application
    auto diffTime = OSTicksToMilliseconds(OSGetTime() - startTime);
    if (diffTime < 300) {
        OSSleepTicks(OSMillisecondsToTicks(300 - diffTime));
    }
}

void ConfigUtils::openConfigMenu() {
    gOnlyAcceptFromThread               = OSGetCurrentThread();
    const bool wasHomeButtonMenuEnabled = OSIsHomeButtonMenuEnabled();

    // Save copy of DC reg values
    const auto tvRender1 = DCReadReg32(SCREEN_TV, D1GRPH_CONTROL_REG);
    const auto tvRender2 = DCReadReg32(SCREEN_TV, D1GRPH_ENABLE_REG);
    const auto tvPitch1  = DCReadReg32(SCREEN_TV, D1GRPH_PITCH_REG);
    const auto tvPitch2  = DCReadReg32(SCREEN_TV, D1OVL_PITCH_REG);

    const auto drcRender1 = DCReadReg32(SCREEN_DRC, D1GRPH_CONTROL_REG);
    const auto drcRender2 = DCReadReg32(SCREEN_DRC, D1GRPH_ENABLE_REG);
    const auto drcPitch1  = DCReadReg32(SCREEN_DRC, D1GRPH_PITCH_REG);
    const auto drcPitch2  = DCReadReg32(SCREEN_DRC, D1OVL_PITCH_REG);

    OSScreenInit();

    const uint32_t screen_buf0_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    const uint32_t screen_buf1_size = OSScreenGetBufferSizeEx(SCREEN_DRC);
    void *screenbuffer0             = MEMAllocFromMappedMemoryForGX2Ex(screen_buf0_size, 0x100);
    void *screenbuffer1             = MEMAllocFromMappedMemoryForGX2Ex(screen_buf1_size, 0x100);

    bool skipScreen0Free = false;
    bool skipScreen1Free = false;
    bool doShutdownKPAD  = false;

    if (!screenbuffer0 || !screenbuffer1) {
        if (screenbuffer0 == nullptr) {
            if (gStoredTVBuffer.buffer_size >= screen_buf0_size) {
                screenbuffer0   = gStoredTVBuffer.buffer;
                skipScreen0Free = true;
                DEBUG_FUNCTION_LINE_VERBOSE("Use storedTVBuffer");
            }
        }
        if (screenbuffer1 == nullptr) {
            if (gStoredDRCBuffer.buffer_size >= screen_buf1_size) {
                screenbuffer1   = gStoredDRCBuffer.buffer;
                skipScreen1Free = true;
                DEBUG_FUNCTION_LINE_VERBOSE("Use storedDRCBuffer");
            }
        }
        if (!screenbuffer0 || !screenbuffer1) {
            DEBUG_FUNCTION_LINE_ERR("Failed to alloc buffers");
            goto error_exit;
        }
    }

    OSScreenSetBufferEx(SCREEN_TV, screenbuffer0);
    OSScreenSetBufferEx(SCREEN_DRC, screenbuffer1);

    // Clear screens
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    // Flip buffers
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

    // Clear screens
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    // Flip buffers
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

    OSScreenEnableEx(SCREEN_TV, 1);
    OSScreenEnableEx(SCREEN_DRC, 1);

    DrawUtils::initBuffers(screenbuffer0, screen_buf0_size, screenbuffer1, screen_buf1_size);
    if (!DrawUtils::initFont()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init Font");
        goto error_exit;
    }

    // disable the home button menu to prevent opening it when exiting
    OSEnableHomeButtonMenu(false);

    KPADStatus status;
    KPADError err;
    if (KPADReadEx(WPAD_CHAN_0, &status, 0, &err) == 0 && err == KPAD_ERROR_UNINITIALIZED) {
        doShutdownKPAD = true;
        KPADInit();
    }

    displayMenu();

    if (doShutdownKPAD) {
        KPADShutdown();
    }

    OSEnableHomeButtonMenu(wasHomeButtonMenuEnabled);

    DrawUtils::deinitFont();

error_exit:
    // Restore DC reg values
    DCWriteReg32(SCREEN_TV, D1GRPH_CONTROL_REG, tvRender1);
    DCWriteReg32(SCREEN_TV, D1GRPH_ENABLE_REG, tvRender2);
    DCWriteReg32(SCREEN_TV, D1GRPH_PITCH_REG, tvPitch1);
    DCWriteReg32(SCREEN_TV, D1OVL_PITCH_REG, tvPitch2);

    DCWriteReg32(SCREEN_DRC, D1GRPH_CONTROL_REG, drcRender1);
    DCWriteReg32(SCREEN_DRC, D1GRPH_ENABLE_REG, drcRender2);
    DCWriteReg32(SCREEN_DRC, D1GRPH_PITCH_REG, drcPitch1);
    DCWriteReg32(SCREEN_DRC, D1OVL_PITCH_REG, drcPitch2);

    if (!skipScreen0Free && screenbuffer0) {
        MEMFreeToMappedMemory(screenbuffer0);
    }

    if (!skipScreen1Free && screenbuffer1) {
        MEMFreeToMappedMemory(screenbuffer1);
    }
    gOnlyAcceptFromThread = nullptr;
}

void ConfigUtils::renderBasicScreen(std::string_view text) {
    DrawUtils::beginDraw();
    DrawUtils::clear(COLOR_BACKGROUND);
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
    DrawUtils::print(16, SCREEN_HEIGHT - 10, "\ue07d Navigate ");
    DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, "\ue000 Select", true);

    DrawUtils::setFontSize(24);
    const uint32_t sz = DrawUtils::getTextWidth(text.data());

    DrawUtils::print((SCREEN_WIDTH / 2) - (sz / 2), (SCREEN_HEIGHT / 2), text.data());

    // draw home button
    DrawUtils::setFontSize(18);
    const auto exitHint = "\ue044 Exit";
    DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 10, exitHint, true);

    DrawUtils::endDraw();
}
