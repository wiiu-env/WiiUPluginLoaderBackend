#include "ConfigUtils.h"
#include "../../globals.h"
#include "../DrawUtils.h"
#include "../logger.h"
#include "ConfigRenderer.h"
#include "config/WUPSConfigAPI.h"
#include "hooks.h"
#include "utils/input/CombinedInput.h"
#include "utils/input/VPADInput.h"
#include "utils/input/WPADInput.h"

#include <coreinit/screen.h>
#include <gx2/display.h>
#include <memory/mappedmemory.h>
#include <string>
#include <vector>

WUPS_CONFIG_SIMPLE_INPUT ConfigUtils::convertInputs(uint32_t buttons) {
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
    return (WUPS_CONFIG_SIMPLE_INPUT) pressedButtons;
}

void ConfigUtils::displayMenu() {
    renderBasicScreen("Loading configs...");

    std::vector<ConfigDisplayItem> configs;
    for (const auto &plugin : gLoadedPlugins) {
        GeneralConfigInformation info;
        info.name    = plugin.getMetaInformation().getName();
        info.author  = plugin.getMetaInformation().getAuthor();
        info.version = plugin.getMetaInformation().getVersion();

        std::unique_ptr<WUPSConfigAPIBackend::WUPSConfig> config;
        const auto configData = plugin.getConfigData();
        if (configData) {
            const auto configHandleOpt = configData->createConfig();
            if (configHandleOpt) {
                WUPSConfigAPIStatus callbackResult = configData->CallMenuOpenendCallback(configHandleOpt.value());
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
            for (const auto &hook : plugin.getPluginInformation().getHookDataList()) {
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
        if (!config) {
            config = make_unique_nothrow<WUPSConfigAPIBackend::WUPSConfig>(info.name);
        }

        configs.emplace_back(info, std::move(config));
    }

    ConfigRenderer renderer(std::move(configs));
    configs.clear();

    CombinedInput baseInput;
    VPadInput vpadInput;
    WPADInput wpadInputs[4] = {
            WPAD_CHAN_0,
            WPAD_CHAN_1,
            WPAD_CHAN_2,
            WPAD_CHAN_3};

    auto startTime      = OSGetTime();
    bool skipFirstInput = true;

    gOnlyAcceptFromThread = OSGetCurrentThread();
    while (true) {
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
        for (int i = 0; i < 4; i++) {
            complexData.kpad.kpadError[i] = wpadInputs[i].kpadError;
            complexData.kpad.data[i]      = wpadInputs[i].kpad;
        }

        auto subState = renderer.Update(baseInput, simpleData, complexData);
        if (subState != SUB_STATE_RUNNING) {
            break;
        }
        if (renderer.NeedsRedraw()) {
            renderer.Render();
        }
        renderer.ResetNeedsRedraw();

        auto diffTime = OSTicksToMicroseconds(OSGetTime() - startTime);
        if (diffTime < 16000) {
            OSSleepTicks(OSMicrosecondsToTicks(16000 - diffTime));
        }
    }

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
}

#define __SetDCPitchReg ((void (*)(uint32_t, uint32_t))(0x101C400 + 0x1e714))

void ConfigUtils::openConfigMenu() {
    gOnlyAcceptFromThread         = OSGetCurrentThread();
    bool wasHomeButtonMenuEnabled = OSIsHomeButtonMenuEnabled();

    OSScreenInit();

    uint32_t screen_buf0_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t screen_buf1_size = OSScreenGetBufferSizeEx(SCREEN_DRC);
    void *screenbuffer0       = MEMAllocFromMappedMemoryForGX2Ex(screen_buf0_size, 0x100);
    void *screenbuffer1       = MEMAllocFromMappedMemoryForGX2Ex(screen_buf1_size, 0x100);

    // Fix the TV buffer pitch if a 1080p buffer is used.
    if (screen_buf0_size == 0x00FD2000) {
        __SetDCPitchReg(SCREEN_TV, 1920);
    }

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

    OSScreenEnableEx(SCREEN_TV, 1);
    OSScreenEnableEx(SCREEN_DRC, 1);

    // Clear screens
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    // Flip buffers
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

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

    if (gStoredTVBuffer.buffer != nullptr) {
        GX2SetTVBuffer(gStoredTVBuffer.buffer, gStoredTVBuffer.buffer_size, static_cast<GX2TVRenderMode>(gStoredTVBuffer.mode),
                       gStoredTVBuffer.surface_format, gStoredTVBuffer.buffering_mode);
    }

    if (gStoredDRCBuffer.buffer != nullptr) {
        GX2SetDRCBuffer(gStoredDRCBuffer.buffer, gStoredDRCBuffer.buffer_size, static_cast<GX2DrcRenderMode>(gStoredDRCBuffer.mode),
                        gStoredDRCBuffer.surface_format, gStoredDRCBuffer.buffering_mode);
    }
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
    DrawUtils::print(SCREEN_WIDTH - 16, 8 + 24, VERSION_FULL, true);
    DrawUtils::drawRectFilled(8, 8 + 24 + 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);

    // draw bottom bar
    DrawUtils::drawRectFilled(8, SCREEN_HEIGHT - 24 - 8 - 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
    DrawUtils::setFontSize(18);
    DrawUtils::print(16, SCREEN_HEIGHT - 10, "\ue07d Navigate ");
    DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 10, "\ue000 Select", true);

    DrawUtils::setFontSize(24);
    uint32_t sz = DrawUtils::getTextWidth(text.data());

    DrawUtils::print((SCREEN_WIDTH / 2) - (sz / 2), (SCREEN_HEIGHT / 2), text.data());

    // draw home button
    DrawUtils::setFontSize(18);
    const char *exitHint = "\ue044 Exit";
    DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 10, exitHint, true);

    DrawUtils::endDraw();
}
