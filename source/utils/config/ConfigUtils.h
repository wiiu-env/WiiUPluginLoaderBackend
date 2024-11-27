#pragma once

#include <cstdint>
#include <string_view>
#include <wups/config.h>

#define MOVE_ITEM_INPUT_MASK (WUPS_CONFIG_BUTTON_B | WUPS_CONFIG_BUTTON_DOWN | WUPS_CONFIG_BUTTON_UP)

class ConfigUtils {
public:
    static void openConfigMenu();

    static WUPS_CONFIG_SIMPLE_INPUT convertInputs(uint32_t buttons);

private:
    static void displayMenu();
    static void renderBasicScreen(std::string_view text);
};
