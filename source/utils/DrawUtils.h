#pragma once

#include "schrift.h"
#include <cstdint>
#include <functional>

// visible screen sizes
#define SCREEN_WIDTH  854
#define SCREEN_HEIGHT 480

#define COLOR_BACKGROUND         Color(238, 238, 238, 255)
#define COLOR_BACKGROUND_WARN    Color(255, 251, 4, 255)
#define COLOR_TEXT               Color(51, 51, 51, 255)
#define COLOR_TEXT2              Color(72, 72, 72, 255)
#define COLOR_DISABLED           Color(255, 0, 0, 255)
#define COLOR_BORDER             Color(204, 204, 204, 255)
#define COLOR_BORDER_HIGHLIGHTED Color(0x3478e4FF)
#define COLOR_WHITE              Color(0xFFFFFFFF)
#define COLOR_BLACK              Color(0, 0, 0, 255)
#define COLOR_WARNING            COLOR_BLACK

union Color {
    explicit Color(const uint32_t color) {
        this->color = color;
    }

    Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    uint32_t color{};
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
};

class DrawUtils {
public:
    static void initBuffers(void *tvBuffer, uint32_t tvSize, void *drcBuffer, uint32_t drcSize);

    static void beginDraw();

    static void endDraw();

    static void clear(Color col);

    static void drawPixel(uint32_t x, uint32_t y, Color col) { drawPixel(x, y, col.r, col.g, col.b, col.a); }

    static void drawPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    static void drawRectFilled(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color col);

    static void drawRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t borderSize, Color col);

    static void drawBitmap(uint32_t x, uint32_t y, uint32_t target_width, uint32_t target_height, const uint8_t *data);

    static void drawPNG(uint32_t x, uint32_t y, const uint8_t *data);

    static bool initFont();

    static void deinitFont();

    static void setFontSize(uint32_t size);

    static void setFontColor(Color col);

    static void print(uint32_t x, uint32_t y, const char *string, bool alignRight = false);

    static void print(uint32_t x, uint32_t y, const wchar_t *string, bool alignRight = false);

    static uint32_t getTextWidth(const char *string);

    static uint32_t getTextWidth(const wchar_t *string);

    static void RenderScreen(const std::function<void()>& callback);

private:
    static bool mIsBackBuffer;

    static uint8_t *mTVBuffer;
    static uint32_t mTVSize;
    static uint8_t *mDRCBuffer;
    static uint32_t mDRCSize;
    static uint32_t mUsedTVWidth;
    static float mUsedTVScale;
};
