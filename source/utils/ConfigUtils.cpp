#include "ConfigUtils.h"

#include "../config/WUPSConfig.h"
#include "../globals.h"
#include "DrawUtils.h"
#include "hooks.h"
#include "logger.h"

#include <coreinit/screen.h>
#include <gx2/display.h>
#include <memory/mappedmemory.h>
#include <padscore/kpad.h>
#include <string>
#include <vector>
#include <vpad/input.h>

#define COLOR_BACKGROUND         Color(238, 238, 238, 255)
#define COLOR_TEXT               Color(51, 51, 51, 255)
#define COLOR_TEXT2              Color(72, 72, 72, 255)
#define COLOR_DISABLED           Color(255, 0, 0, 255)
#define COLOR_BORDER             Color(204, 204, 204, 255)
#define COLOR_BORDER_HIGHLIGHTED Color(0x3478e4FF)
#define COLOR_WHITE              Color(0xFFFFFFFF)
#define COLOR_BLACK              Color(0, 0, 0, 255)

struct ConfigDisplayItem {
    WUPSConfig *config{};
    std::string name;
    std::string author;
    std::string version;
};

#define MAX_BUTTONS_ON_SCREEN 8

static uint32_t remapWiiMoteButtons(uint32_t buttons) {
    uint32_t conv_buttons = 0;

    if (buttons & WPAD_BUTTON_LEFT)
        conv_buttons |= VPAD_BUTTON_LEFT;

    if (buttons & WPAD_BUTTON_RIGHT)
        conv_buttons |= VPAD_BUTTON_RIGHT;

    if (buttons & WPAD_BUTTON_DOWN)
        conv_buttons |= VPAD_BUTTON_DOWN;

    if (buttons & WPAD_BUTTON_UP)
        conv_buttons |= VPAD_BUTTON_UP;

    if (buttons & WPAD_BUTTON_PLUS)
        conv_buttons |= VPAD_BUTTON_PLUS;

    if (buttons & WPAD_BUTTON_B)
        conv_buttons |= VPAD_BUTTON_B;

    if (buttons & WPAD_BUTTON_A)
        conv_buttons |= VPAD_BUTTON_A;

    if (buttons & WPAD_BUTTON_MINUS)
        conv_buttons |= VPAD_BUTTON_MINUS;

    if (buttons & WPAD_BUTTON_HOME)
        conv_buttons |= VPAD_BUTTON_HOME;

    return conv_buttons;
}

static uint32_t remapClassicButtons(uint32_t buttons) {
    uint32_t conv_buttons = 0;

    if (buttons & WPAD_CLASSIC_BUTTON_LEFT)
        conv_buttons |= VPAD_BUTTON_LEFT;

    if (buttons & WPAD_CLASSIC_BUTTON_RIGHT)
        conv_buttons |= VPAD_BUTTON_RIGHT;

    if (buttons & WPAD_CLASSIC_BUTTON_DOWN)
        conv_buttons |= VPAD_BUTTON_DOWN;

    if (buttons & WPAD_CLASSIC_BUTTON_UP)
        conv_buttons |= VPAD_BUTTON_UP;

    if (buttons & WPAD_CLASSIC_BUTTON_PLUS)
        conv_buttons |= VPAD_BUTTON_PLUS;

    if (buttons & WPAD_CLASSIC_BUTTON_X)
        conv_buttons |= VPAD_BUTTON_X;

    if (buttons & WPAD_CLASSIC_BUTTON_Y)
        conv_buttons |= VPAD_BUTTON_Y;

    if (buttons & WPAD_CLASSIC_BUTTON_B)
        conv_buttons |= VPAD_BUTTON_B;

    if (buttons & WPAD_CLASSIC_BUTTON_A)
        conv_buttons |= VPAD_BUTTON_A;

    if (buttons & WPAD_CLASSIC_BUTTON_MINUS)
        conv_buttons |= VPAD_BUTTON_MINUS;

    if (buttons & WPAD_CLASSIC_BUTTON_HOME)
        conv_buttons |= VPAD_BUTTON_HOME;

    if (buttons & WPAD_CLASSIC_BUTTON_ZR)
        conv_buttons |= VPAD_BUTTON_ZR;

    if (buttons & WPAD_CLASSIC_BUTTON_ZL)
        conv_buttons |= VPAD_BUTTON_ZL;

    if (buttons & WPAD_CLASSIC_BUTTON_R)
        conv_buttons |= VPAD_BUTTON_R;

    if (buttons & WPAD_CLASSIC_BUTTON_L)
        conv_buttons |= VPAD_BUTTON_L;

    return conv_buttons;
}

void ConfigUtils::displayMenu() {
    std::vector<ConfigDisplayItem> configs;
    for (auto &plugin : gLoadedPlugins) {
        ConfigDisplayItem cfg;
        cfg.name    = plugin->metaInformation->getName();
        cfg.author  = plugin->metaInformation->getAuthor();
        cfg.version = plugin->metaInformation->getVersion();

        for (auto &hook : plugin->getPluginInformation()->getHookDataList()) {

            if (hook->getType() == WUPS_LOADER_HOOK_GET_CONFIG /*WUPS_LOADER_HOOK_GET_CONFIG*/) {
                if (hook->getFunctionPointer() == nullptr) {
                    break;
                }
                auto *cur_config = reinterpret_cast<WUPSConfig *>(((WUPSConfigHandle(*)())((uint32_t *) hook->getFunctionPointer()))());
                if (cur_config == nullptr) {
                    break;
                }
                cfg.config = cur_config;
                configs.push_back(cfg);
                break;
            }
        }
    }

    ConfigDisplayItem *currentConfig    = nullptr;
    WUPSConfigCategory *currentCategory = nullptr;

    uint32_t selectedBtn = 0;
    uint32_t start       = 0;
    uint32_t end         = MAX_BUTTONS_ON_SCREEN;
    if (configs.size() < MAX_BUTTONS_ON_SCREEN) {
        end = configs.size();
    }

    bool redraw = true;
    uint32_t buttonsTriggered;
    uint32_t buttonsReleased;

    VPADStatus vpad_data{};
    VPADReadError vpad_error;
    KPADStatus kpad_data{};
    KPADError kpad_error;

    while (true) {
        buttonsTriggered = 0;
        buttonsReleased  = 0;

        VPADRead(VPAD_CHAN_0, &vpad_data, 1, &vpad_error);
        if (vpad_error == VPAD_READ_SUCCESS) {
            buttonsTriggered = vpad_data.trigger;
            buttonsReleased  = vpad_data.release;
        }

        for (int i = 0; i < 4; i++) {
            if (KPADReadEx((KPADChan) i, &kpad_data, 1, &kpad_error) > 0) {
                if (kpad_error == KPAD_ERROR_OK && kpad_data.extensionType != 0xFF) {
                    if (kpad_data.extensionType == WPAD_EXT_CORE || kpad_data.extensionType == WPAD_EXT_NUNCHUK) {
                        buttonsTriggered |= remapWiiMoteButtons(kpad_data.trigger);
                        buttonsReleased |= remapWiiMoteButtons(kpad_data.release);
                    } else {
                        buttonsTriggered |= remapClassicButtons(kpad_data.classic.trigger);
                        buttonsReleased |= remapClassicButtons(kpad_data.classic.release);
                    }
                }
            }
        }

        if (buttonsReleased & VPAD_BUTTON_HOME) {
            break;
        }

        if (configs.empty()) {
            renderNoConfigFoundScreen();
            continue;
        }

        if (!currentConfig || !currentConfig->config) {
            if (buttonsTriggered & VPAD_BUTTON_DOWN) {
                if (selectedBtn < configs.size() - 1) {
                    selectedBtn++;
                    redraw = true;
                }
            } else if (buttonsTriggered & VPAD_BUTTON_UP) {
                if (selectedBtn > 0) {
                    selectedBtn--;
                    redraw = true;
                }
            }
            if (buttonsTriggered & VPAD_BUTTON_A) {
                currentConfig = &configs[selectedBtn];
                if (currentConfig == nullptr) {
                    break;
                }

                selectedBtn = 0;
                start       = 0;
                end         = MAX_BUTTONS_ON_SCREEN;

                auto cats = currentConfig->config->getCategories();
                if (cats.size() < MAX_BUTTONS_ON_SCREEN) {
                    end = cats.size();
                }

                redraw = true;
                continue;
            }

            if (selectedBtn >= end) {
                end   = selectedBtn + 1;
                start = end - MAX_BUTTONS_ON_SCREEN;
            } else if (selectedBtn < start) {
                start = selectedBtn;
                end   = start + MAX_BUTTONS_ON_SCREEN;
            }

            if (redraw) {
                DrawUtils::beginDraw();
                DrawUtils::clear(COLOR_BACKGROUND);

                // draw buttons
                uint32_t index = 8 + 24 + 8 + 4;
                for (uint32_t i = start; i < end; i++) {
                    DrawUtils::setFontColor(COLOR_TEXT);

                    if (i == selectedBtn) {
                        DrawUtils::drawRect(16, index, SCREEN_WIDTH - 16 * 2, 44, 4, COLOR_BORDER_HIGHLIGHTED);
                    } else {
                        DrawUtils::drawRect(16, index, SCREEN_WIDTH - 16 * 2, 44, 2, COLOR_BORDER);
                    }

                    DrawUtils::setFontSize(24);
                    DrawUtils::print(16 * 2, index + 8 + 24, configs[i].name.c_str());
                    uint32_t sz = DrawUtils::getTextWidth(configs[i].name.c_str());
                    DrawUtils::setFontSize(12);
                    DrawUtils::print(16 * 2 + sz + 4, index + 8 + 24, configs[i].author.c_str());
                    DrawUtils::print(SCREEN_WIDTH - 16 * 2, index + 8 + 24, configs[i].version.c_str(), true);
                    index += 42 + 8;
                }

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
                DrawUtils::print(16, SCREEN_HEIGHT - 8, "\ue07d Navigate ");

                // draw scroll indicator
                DrawUtils::setFontSize(24);
                if (end < configs.size()) {
                    DrawUtils::print(SCREEN_WIDTH / 2 + 12, SCREEN_HEIGHT - 32, "\ufe3e", true);
                }
                if (start > 0) {
                    DrawUtils::print(SCREEN_WIDTH / 2 + 12, 32 + 20, "\ufe3d", true);
                }

                // draw home button
                DrawUtils::setFontSize(18);
                const char *exitHint = "\ue044 Exit";
                DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 8, exitHint, true);

                DrawUtils::endDraw();
                redraw = false;
            }

            continue;
        }


        if (!currentCategory) {
            auto cats = currentConfig->config->getCategories();
            if (buttonsTriggered & VPAD_BUTTON_DOWN) {
                if (selectedBtn < cats.size() - 1) {
                    selectedBtn++;
                    redraw = true;
                }
            } else if (buttonsTriggered & VPAD_BUTTON_UP) {
                if (selectedBtn > 0) {
                    selectedBtn--;
                    redraw = true;
                }
            } else if (buttonsTriggered & VPAD_BUTTON_A) {
                currentCategory = cats[selectedBtn];
                if (currentCategory == nullptr) {
                    DEBUG_FUNCTION_LINE_ERR("currentCategory was NULL");
                    break;
                }

                selectedBtn = 0;
                start       = 0;
                end         = MAX_BUTTONS_ON_SCREEN;

                auto items = currentCategory->getItems();
                if (items.size() < MAX_BUTTONS_ON_SCREEN) {
                    end = items.size();
                }

                redraw = true;
                continue;
            } else if (buttonsTriggered & VPAD_BUTTON_B) {
                currentConfig   = nullptr;
                currentCategory = nullptr;
                selectedBtn     = 0;
                start           = 0;
                end             = MAX_BUTTONS_ON_SCREEN;
                if (configs.size() < MAX_BUTTONS_ON_SCREEN) {
                    end = configs.size();
                }
                redraw = true;
                continue;
            }

            if (selectedBtn >= end) {
                end   = selectedBtn + 1;
                start = end - MAX_BUTTONS_ON_SCREEN;
            } else if (selectedBtn < start) {
                start = selectedBtn;
                end   = start + MAX_BUTTONS_ON_SCREEN;
            }

            if (redraw) {
                DrawUtils::beginDraw();
                DrawUtils::clear(COLOR_BACKGROUND);

                // draw buttons
                uint32_t index = 8 + 24 + 8 + 4;
                for (uint32_t i = start; i < end; i++) {
                    DrawUtils::setFontColor(COLOR_TEXT);

                    if (i == selectedBtn) {
                        DrawUtils::drawRect(16, index, SCREEN_WIDTH - 16 * 2, 44, 4, COLOR_BORDER_HIGHLIGHTED);
                    } else {
                        DrawUtils::drawRect(16, index, SCREEN_WIDTH - 16 * 2, 44, 2, COLOR_BORDER);
                    }

                    DrawUtils::setFontSize(24);
                    DrawUtils::print(16 * 2, index + 8 + 24, cats[i]->getName().c_str());
                    index += 42 + 8;
                }

                DrawUtils::setFontColor(COLOR_TEXT);

                // draw top bar
                DrawUtils::setFontSize(24);
                DrawUtils::print(16, 6 + 24, StringTools::truncate(currentConfig->config->getName(), 45).c_str());
                DrawUtils::setFontSize(18);
                DrawUtils::print(SCREEN_WIDTH - 16, 8 + 24, currentConfig->version.c_str(), true);
                DrawUtils::drawRectFilled(8, 8 + 24 + 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);

                // draw bottom bar
                DrawUtils::drawRectFilled(8, SCREEN_HEIGHT - 24 - 8 - 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
                DrawUtils::setFontSize(18);
                DrawUtils::print(16, SCREEN_HEIGHT - 8, "\ue07d Navigate ");

                // draw scroll indicator
                DrawUtils::setFontSize(24);
                if (end < configs.size()) {
                    DrawUtils::print(SCREEN_WIDTH / 2 + 12, SCREEN_HEIGHT - 32, "\ufe3e", true);
                }
                if (start > 0) {
                    DrawUtils::print(SCREEN_WIDTH / 2 + 12, 32 + 20, "\ufe3d", true);
                }

                // draw home button
                DrawUtils::setFontSize(18);
                const char *exitHint = "\ue044 Exit";
                DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 8, exitHint, true);

                DrawUtils::endDraw();
                redraw = false;
            }

            continue;
        }

        const std::vector<WUPSConfigItem *> config_items = currentCategory->getItems();

        if (buttonsTriggered & VPAD_BUTTON_DOWN) {
            if (selectedBtn < config_items.size() - 1) {
                selectedBtn++;
                redraw = true;
            }
        } else if (buttonsTriggered & VPAD_BUTTON_UP) {
            if (selectedBtn > 0) {
                selectedBtn--;
                redraw = true;
            }
        } else if (buttonsTriggered & VPAD_BUTTON_B) {
            currentCategory = nullptr;
            selectedBtn     = 0;
            start           = 0;
            end             = MAX_BUTTONS_ON_SCREEN;
            auto catSize    = currentConfig->config->getCategories().size();
            if (catSize < MAX_BUTTONS_ON_SCREEN) {
                end = catSize;
            }
            redraw = true;
            continue;
        }

        WUPSConfigButtons pressedButtons = WUPS_CONFIG_BUTTON_NONE;
        if (buttonsTriggered & VPAD_BUTTON_A) {
            pressedButtons |= WUPS_CONFIG_BUTTON_A;
        }
        if (buttonsTriggered & VPAD_BUTTON_LEFT) {
            pressedButtons |= WUPS_CONFIG_BUTTON_LEFT;
        }
        if (buttonsTriggered & VPAD_BUTTON_RIGHT) {
            pressedButtons |= WUPS_CONFIG_BUTTON_RIGHT;
        }
        if (buttonsTriggered & VPAD_BUTTON_L) {
            pressedButtons |= WUPS_CONFIG_BUTTON_L;
        }
        if (buttonsTriggered & VPAD_BUTTON_R) {
            pressedButtons |= WUPS_CONFIG_BUTTON_R;
        }
        if (buttonsTriggered & VPAD_BUTTON_ZL) {
            pressedButtons |= WUPS_CONFIG_BUTTON_ZL;
        }
        if (buttonsTriggered & VPAD_BUTTON_ZR) {
            pressedButtons |= WUPS_CONFIG_BUTTON_ZR;
        }
        if (buttonsTriggered & VPAD_BUTTON_X) {
            pressedButtons |= WUPS_CONFIG_BUTTON_Y;
        }
        if (buttonsTriggered & VPAD_BUTTON_Y) {
            pressedButtons |= WUPS_CONFIG_BUTTON_X;
        }
        if (buttonsTriggered & VPAD_BUTTON_STICK_L) {
            pressedButtons |= WUPS_CONFIG_BUTTON_STICK_L;
        }
        if (buttonsTriggered & VPAD_BUTTON_STICK_R) {
            pressedButtons |= WUPS_CONFIG_BUTTON_STICK_R;
        }
        if (buttonsTriggered & VPAD_BUTTON_PLUS) {
            pressedButtons |= WUPS_CONFIG_BUTTON_PLUS;
        }
        if (buttonsTriggered & VPAD_BUTTON_MINUS) {
            pressedButtons |= WUPS_CONFIG_BUTTON_MINUS;
        }

        if (pressedButtons != WUPS_CONFIG_BUTTON_NONE) {
            redraw = true;
        }

        if (selectedBtn >= end) {
            end   = selectedBtn + 1;
            start = end - MAX_BUTTONS_ON_SCREEN;
        } else if (selectedBtn < start) {
            start = selectedBtn;
            end   = start + MAX_BUTTONS_ON_SCREEN;
        }

        if (redraw) {
            DrawUtils::beginDraw();
            DrawUtils::clear(COLOR_BACKGROUND);

            // draw buttons
            uint32_t index = 8 + 24 + 8 + 4;
            for (uint32_t i = start; i < end; i++) {
                DrawUtils::setFontColor(COLOR_TEXT);

                if (i == selectedBtn) {
                    DrawUtils::drawRect(16, index, SCREEN_WIDTH - 16 * 2, 44, 4, COLOR_BORDER_HIGHLIGHTED);
                } else {
                    DrawUtils::drawRect(16, index, SCREEN_WIDTH - 16 * 2, 44, 2, COLOR_BORDER);
                }

                DrawUtils::setFontSize(24);
                DrawUtils::print(16 * 2, index + 8 + 24, config_items[i]->getDisplayName().c_str());
                if (i == selectedBtn) {
                    if (pressedButtons != WUPS_CONFIG_BUTTON_NONE) {
                        config_items[i]->onButtonPressed(pressedButtons);
                    }
                    DrawUtils::print(SCREEN_WIDTH - 16 * 2, index + 8 + 24, config_items[i]->getCurrentValueSelectedDisplay().c_str(), true);
                } else {
                    DrawUtils::print(SCREEN_WIDTH - 16 * 2, index + 8 + 24, config_items[i]->getCurrentValueDisplay().c_str(), true);
                }
                index += 42 + 8;
            }

            DrawUtils::setFontColor(COLOR_TEXT);

            auto headline = string_format("%s - %s", currentConfig->config->getName().c_str(), currentCategory->getName().c_str());
            // draw top bar
            DrawUtils::setFontSize(24);
            DrawUtils::print(16, 6 + 24, StringTools::truncate(headline, 45).c_str());
            DrawUtils::drawRectFilled(8, 8 + 24 + 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
            DrawUtils::setFontSize(18);
            DrawUtils::print(SCREEN_WIDTH - 16, 8 + 24, currentConfig->version.c_str(), true);

            // draw bottom bar
            DrawUtils::drawRectFilled(8, SCREEN_HEIGHT - 24 - 8 - 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
            DrawUtils::setFontSize(18);
            DrawUtils::print(16, SCREEN_HEIGHT - 8, "\ue07d Navigate ");
            DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 8, "\ue001 Back", true);

            // draw scroll indicator
            DrawUtils::setFontSize(24);
            if (end < configs.size()) {
                DrawUtils::print(SCREEN_WIDTH / 2 + 12, SCREEN_HEIGHT - 32, "\ufe3e", true);
            }
            if (start > 0) {
                DrawUtils::print(SCREEN_WIDTH / 2 + 12, 32 + 20, "\ufe3d", true);
            }

            // draw home button
            DrawUtils::setFontSize(18);
            const char *exitHint = "\ue044 Exit";
            DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 8, exitHint, true);

            DrawUtils::endDraw();
            redraw = pressedButtons != WUPS_CONFIG_BUTTON_NONE;
        }
    }

    for (const auto &element : configs) {
        for (const auto &cat : element.config->getCategories()) {
            for (const auto &item : cat->getItems()) {
                if (item->isDirty()) {
                    item->callCallback();
                }
            }
        }
    }

    CallHook(gLoadedPlugins, WUPS_LOADER_HOOK_CONFIG_CLOSED);

    for (const auto &element : configs) {
        delete element.config;
    }
}

#define __SetDCPitchReg ((void (*)(uint32_t, uint32_t))(0x101C400 + 0x1e714))

void ConfigUtils::openConfigMenu() {
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
    DrawUtils::initFont();

    // disable the home button menu to prevent opening it when exiting
    OSEnableHomeButtonMenu(false);

    displayMenu();

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
}
void ConfigUtils::renderNoConfigFoundScreen() {
    DrawUtils::beginDraw();
    DrawUtils::clear(COLOR_BACKGROUND);
    DrawUtils::setFontColor(COLOR_TEXT);

    // draw top bar
    DrawUtils::setFontSize(24);
    DrawUtils::print(16, 6 + 24, "Wii U Plugin System Config Menu");
    DrawUtils::setFontSize(18);
    DrawUtils::print(SCREEN_WIDTH - 16, 8 + 24, "v1.0", true);
    DrawUtils::drawRectFilled(8, 8 + 24 + 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);

    // draw bottom bar
    DrawUtils::drawRectFilled(8, SCREEN_HEIGHT - 24 - 8 - 4, SCREEN_WIDTH - 8 * 2, 3, COLOR_BLACK);
    DrawUtils::setFontSize(18);
    DrawUtils::print(16, SCREEN_HEIGHT - 8, "\ue07d Navigate ");

    const char *noConfigFoundText = "No configurable plugins loaded";
    DrawUtils::setFontSize(24);
    uint32_t sz = DrawUtils::getTextWidth(noConfigFoundText);

    DrawUtils::print((SCREEN_WIDTH / 2) - (sz / 2), (SCREEN_HEIGHT / 2), noConfigFoundText);

    // draw home button
    DrawUtils::setFontSize(18);
    const char *exitHint = "\ue044 Exit";
    DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 8, exitHint, true);

    DrawUtils::endDraw();
}
