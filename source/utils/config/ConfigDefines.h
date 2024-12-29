#pragma once

#include <gx2/surface.h>

#include <cstdint>

#define MAX_BUTTONS_ON_SCREEN 8

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
    SUB_STATE_RETURN_WITH_PLUGIN_RELOAD = 2,
    SUB_STATE_ERROR                     = 3,
};