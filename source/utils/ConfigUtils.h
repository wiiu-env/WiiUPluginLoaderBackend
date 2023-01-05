#pragma once

#include <gx2/enum.h>
#include <string>

struct StoredBuffer {
    void *buffer;
    uint32_t buffer_size;
    uint32_t mode;
    GX2SurfaceFormat surface_format;
    GX2BufferingMode buffering_mode;
};

class ConfigUtils {
public:
    static void openConfigMenu();

private:
    static void displayMenu();
    static void renderBasicScreen(std::string_view text);
};
