#pragma once

#include <cstdint>
#include <gx2/surface.h>

#define COLOR_BACKGROUND         Color(238, 238, 238, 255)
#define COLOR_TEXT               Color(51, 51, 51, 255)
#define COLOR_TEXT2              Color(72, 72, 72, 255)
#define COLOR_DISABLED           Color(255, 0, 0, 255)
#define COLOR_BORDER             Color(204, 204, 204, 255)
#define COLOR_BORDER_HIGHLIGHTED Color(0x3478e4FF)
#define COLOR_WHITE              Color(0xFFFFFFFF)
#define COLOR_BLACK              Color(0, 0, 0, 255)

#define MAX_BUTTONS_ON_SCREEN    8

struct StoredBuffer {
    void *buffer;
    uint32_t buffer_size;
    uint32_t mode;
    GX2SurfaceFormat surface_format;
    GX2BufferingMode buffering_mode;
};

enum ConfigSubState {
    SUB_STATE_RUNNING                   = 0,
    SUB_STATE_RETURN                    = 1,
    SUB_STATE_RETURN_WITH_PLUGIN_RELOAD = 1,
    SUB_STATE_ERROR                     = 2,
};