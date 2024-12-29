#include "ConfigUtils.h"

#include "ConfigDisplayItem.h"
#include "ConfigRenderer.h"
#include "config/WUPSConfigAPI.h"
#include "globals.h"
#include "hooks.h"
#include "plugin/HookData.h"
#include "plugin/PluginContainer.h"
#include "plugin/PluginLoadWrapper.h"
#include "utils/DrawUtils.h"
#include "utils/WUPSBackendSettings.h"
#include "utils/input/CombinedInput.h"
#include "utils/input/Input.h"
#include "utils/input/VPADInput.h"
#include "utils/input/WPADInput.h"
#include "utils/logger.h"
#include "utils/utils.h"

#include <wups/config.h>
#include <wups/hooks.h>

#include <coreinit/title.h>
#include <sysapp/launch.h>

#include <bits/ranges_algo.h>
#include <memory>
#include <vector>

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
                            break;
                        }
                        auto cur_config_handle = ((void *(*) ())((uint32_t *) hook.getFunctionPointer()))();
                        if (cur_config_handle == nullptr) {
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

    gOnlyAcceptFromThread              = OSGetCurrentThread();
    ConfigSubState subStateReturnValue = SUB_STATE_ERROR;
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

        subStateReturnValue = renderer.Update(baseInput, simpleData, complexData);
        if (subStateReturnValue != SUB_STATE_RUNNING) {
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

    std::vector<PluginLoadWrapper> newActivePluginsList;

    if (subStateReturnValue == SUB_STATE_RETURN_WITH_PLUGIN_RELOAD && renderer.GetActivePluginsIfChanged(newActivePluginsList)) {
        startTime = OSGetTime();
        renderBasicScreen("Applying changes, app will now restart...");

        // Get list of inactive plugins to save them in the config
        // Note: this does only consider plugin loaded from the sd card.
        std::vector<std::string> newInactivePluginsList;
        for (const auto &cur : newActivePluginsList) {
            if (!cur.isLoadAndLink()) {
                auto &source = cur.getPluginData()->getSource();
                if (source.starts_with(getPluginPath()) && source.ends_with(".wps")) {
                    std::size_t found    = source.find_last_of("/\\");
                    std::string filename = source.substr(found + 1);
                    newInactivePluginsList.push_back(filename);
                }
            }
        }
        gLoadOnNextLaunch = newActivePluginsList;
        WUPSBackendSettings::SetInactivePluginFilenames(newInactivePluginsList);
        if (!WUPSBackendSettings::SaveSettings()) {
            DEBUG_FUNCTION_LINE_WARN("Failed to save WUPSBackendSettings");
        }

        _SYSLaunchTitleWithStdArgsInNoSplash(OSGetTitleID(), nullptr);
        // Make sure to wait at least 2 seconds so user can read the screen and
        // are aware the app will restart now.
        auto diffTime = OSTicksToMilliseconds(OSGetTime() - startTime);
        if (diffTime < 2000) {
            OSSleepTicks(OSTicksToMilliseconds(2000 - diffTime));
        }
    } else {
        renderBasicScreen("Saving configs...");
    }
    for (const auto &plugin : gLoadedPlugins) {
        if (const auto configData = plugin.getConfigData()) {
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
    DrawUtils::RenderScreen(&displayMenu);
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
