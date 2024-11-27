#include "DrawUtils.h"

#include "dc.h"
#include "logger.h"
#include "utils.h"
#include <avm/tv.h>
#include <coreinit/cache.h>
#include <coreinit/memory.h>
#include <coreinit/screen.h>
#include <cstdlib>
#include <png.h>

// buffer width
#define DRC_WIDTH 0x380

bool DrawUtils::mIsBackBuffer;

uint8_t *DrawUtils::mTVBuffer    = nullptr;
uint32_t DrawUtils::mTVSize      = 0;
uint8_t *DrawUtils::mDRCBuffer   = nullptr;
uint32_t DrawUtils::mDRCSize     = 0;
uint32_t DrawUtils::mUsedTVWidth = 1280;
float DrawUtils::mUsedTVScale    = 1.5f;
static SFT pFont                 = {};

static Color font_col(0xFFFFFFFF);

void DrawUtils::initBuffers(void *tvBuffer, const uint32_t tvSize, void *drcBuffer, const uint32_t drcSize) {
    DrawUtils::mTVBuffer  = static_cast<uint8_t *>(tvBuffer);
    DrawUtils::mTVSize    = tvSize;
    DrawUtils::mDRCBuffer = static_cast<uint8_t *>(drcBuffer);
    DrawUtils::mDRCSize   = drcSize;

    bool bigScale = true;
    switch (TVEGetCurrentPort()) {
        case TVE_PORT_HDMI:
            bigScale = true;
            break;
        case TVE_PORT_COMPONENT:
        case TVE_PORT_COMPOSITE:
        case TVE_PORT_SCART:
            bigScale = false;
            break;
    }

    AVMTvResolution tvResolution = AVM_TV_RESOLUTION_720P;
    if (AVMGetTVScanMode(&tvResolution)) {
        switch (tvResolution) {
            case AVM_TV_RESOLUTION_480P:
            case AVM_TV_RESOLUTION_720P:
            case AVM_TV_RESOLUTION_720P_3D:
            case AVM_TV_RESOLUTION_1080I:
            case AVM_TV_RESOLUTION_1080P:
            case AVM_TV_RESOLUTION_576P:
            case AVM_TV_RESOLUTION_720P_50HZ:
            case AVM_TV_RESOLUTION_1080I_50HZ:
            case AVM_TV_RESOLUTION_1080P_50HZ:
                bigScale = true;
                break;
            case AVM_TV_RESOLUTION_576I:
            case AVM_TV_RESOLUTION_480I:
            case AVM_TV_RESOLUTION_480I_PAL60:
                break;
        }
    }

    auto tvScanBufferWidth = DCReadReg32(SCREEN_TV, D1GRPH_X_END_REG);

    if (tvScanBufferWidth == 640) { // 480i/480p/576i 4:3
        DrawUtils::mUsedTVWidth = 640;
        SetDCPitchReg(SCREEN_TV, 640);
        DrawUtils::mUsedTVScale = bigScale ? 0.75 : 0.75f;
    } else if (tvScanBufferWidth == 854) { // 480i/480p/576i 16:9
        DrawUtils::mUsedTVWidth = 896;
        SetDCPitchReg(SCREEN_TV, 896);
        DrawUtils::mUsedTVScale = bigScale ? 1.0 : 1.0f;
    } else if (tvScanBufferWidth == 1280) { // 720p 16:9
        DrawUtils::mUsedTVWidth = 1280;
        SetDCPitchReg(SCREEN_TV, 1280);
        if (bigScale) {
            DrawUtils::mUsedTVScale = 1.5;
        } else {
            DrawUtils::mUsedTVScale = 0.75f;
            if (tvResolution == AVM_TV_RESOLUTION_480I_PAL60 || tvResolution == AVM_TV_RESOLUTION_480I) {
                AVMTvAspectRatio tvAspectRatio;
                if (AVMGetTVAspectRatio(&tvAspectRatio) && tvAspectRatio == AVM_TV_ASPECT_RATIO_16_9) {
                    DEBUG_FUNCTION_LINE_WARN("force big scaling for 480i + 16:9");
                    DrawUtils::mUsedTVScale = 1.5;
                }
            }
        }
    } else if (tvScanBufferWidth == 1920) { // 1080i/1080p 16:9
        DrawUtils::mUsedTVWidth = 1920;
        SetDCPitchReg(SCREEN_TV, 1920);
        DrawUtils::mUsedTVScale = bigScale ? 2.25 : 1.125f;
    } else {
        DrawUtils::mUsedTVWidth = tvScanBufferWidth;
        SetDCPitchReg(SCREEN_TV, tvScanBufferWidth);
        DrawUtils::mUsedTVScale = 1.0f;
        DEBUG_FUNCTION_LINE_WARN("Unknown tv width detected, config menu might not show properly");
    }
}

void DrawUtils::beginDraw() {
    const uint32_t pixel = *reinterpret_cast<uint32_t *>(mTVBuffer);

    // check which buffer is currently used
    OSScreenPutPixelEx(SCREEN_TV, 0, 0, 0xABCDEF90);
    if (*reinterpret_cast<uint32_t *>(mTVBuffer) == 0xABCDEF90) {
        mIsBackBuffer = false;
    } else {
        mIsBackBuffer = true;
    }

    // restore the pixel we used for checking
    *reinterpret_cast<uint32_t *>(mTVBuffer) = pixel;
}

void DrawUtils::endDraw() {
    // OSScreenFlipBuffersEx already flushes the cache?
    // DCFlushRange(tvBuffer, tvSize);
    // DCFlushRange(drcBuffer, drcSize);

    OSScreenFlipBuffersEx(SCREEN_DRC);
    OSScreenFlipBuffersEx(SCREEN_TV);
}

void DrawUtils::clear(const Color col) {
    OSScreenClearBufferEx(SCREEN_TV, col.color);
    OSScreenClearBufferEx(SCREEN_DRC, col.color);
}

void DrawUtils::drawPixel(const uint32_t x, const uint32_t y, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
    if (a == 0) {
        return;
    }

    const float opacity = a / 255.0f;

    // put pixel in the drc buffer
    uint32_t i = (x + y * DRC_WIDTH) * 4;
    if (i + 3 < mDRCSize / 2) {
        if (mIsBackBuffer) {
            i += mDRCSize / 2;
        }
        if (a == 0xFF) {
            mDRCBuffer[i]     = r;
            mDRCBuffer[i + 1] = g;
            mDRCBuffer[i + 2] = b;
        } else {
            mDRCBuffer[i]     = r * opacity + mDRCBuffer[i] * (1 - opacity);
            mDRCBuffer[i + 1] = g * opacity + mDRCBuffer[i + 1] * (1 - opacity);
            mDRCBuffer[i + 2] = b * opacity + mDRCBuffer[i + 2] * (1 - opacity);
        }
    }

    // scale and put pixel in the tv buffer
    for (uint32_t yy = (y * DrawUtils::mUsedTVScale); yy < ((y * DrawUtils::mUsedTVScale) + (uint32_t) DrawUtils::mUsedTVScale); yy++) {
        for (uint32_t xx = (x * DrawUtils::mUsedTVScale); xx < ((x * DrawUtils::mUsedTVScale) + (uint32_t) DrawUtils::mUsedTVScale); xx++) {
            uint32_t i = (xx + yy * DrawUtils::mUsedTVWidth) * 4;
            if (i + 3 < mTVSize / 2) {
                if (mIsBackBuffer) {
                    i += mTVSize / 2;
                }
                if (a == 0xFF) {
                    mTVBuffer[i]     = r;
                    mTVBuffer[i + 1] = g;
                    mTVBuffer[i + 2] = b;
                } else {
                    mTVBuffer[i]     = r * opacity + mTVBuffer[i] * (1 - opacity);
                    mTVBuffer[i + 1] = g * opacity + mTVBuffer[i + 1] * (1 - opacity);
                    mTVBuffer[i + 2] = b * opacity + mTVBuffer[i + 2] * (1 - opacity);
                }
            }
        }
    }
}

void DrawUtils::drawRectFilled(const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h, const Color col) {
    for (uint32_t yy = y; yy < y + h; yy++) {
        for (uint32_t xx = x; xx < x + w; xx++) {
            drawPixel(xx, yy, col);
        }
    }
}

void DrawUtils::drawRect(const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h, const uint32_t borderSize, const Color col) {
    drawRectFilled(x, y, w, borderSize, col);
    drawRectFilled(x, y + h - borderSize, w, borderSize, col);
    drawRectFilled(x, y, borderSize, h, col);
    drawRectFilled(x + w - borderSize, y, borderSize, h, col);
}

void DrawUtils::drawBitmap(const uint32_t x, const uint32_t y, const uint32_t target_width, const uint32_t target_height, const uint8_t *data) {
    if (data[0] != 'B' || data[1] != 'M') {
        // invalid header
        return;
    }

    uint32_t dataPos      = __builtin_bswap32(*(uint32_t *) &(data[0x0A]));
    const uint32_t width  = __builtin_bswap32(*(uint32_t *) &(data[0x12]));
    const uint32_t height = __builtin_bswap32(*(uint32_t *) &(data[0x16]));

    if (dataPos == 0) {
        dataPos = 54;
    }

    data += dataPos;

    // TODO flip image since bitmaps are stored upside down

    for (uint32_t yy = y; yy < y + target_height; yy++) {
        for (uint32_t xx = x; xx < x + target_width; xx++) {
            uint32_t i = (((xx - x) * width / target_width) + ((yy - y) * height / target_height) * width) * 3;
            drawPixel(xx, yy, data[i + 2], data[i + 1], data[i], 0xFF);
        }
    }
}

static void png_read_data(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead) {
    void **data = static_cast<void **>(png_get_io_ptr(png_ptr));

    memcpy(outBytes, *data, byteCountToRead);
    *reinterpret_cast<uint8_t **>(data) += byteCountToRead;
}

void DrawUtils::drawPNG(const uint32_t x, const uint32_t y, const uint8_t *data) {
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr) {
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        return;
    }

    png_set_read_fn(png_ptr, (void *) &data, png_read_data);

    png_read_info(png_ptr, info_ptr);

    uint32_t width        = 0;
    uint32_t height       = 0;
    int bitDepth          = 0;
    int colorType         = -1;
    const uint32_t retval = png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, nullptr, nullptr, nullptr);
    if (retval != 1) {
        return;
    }

    const uint32_t bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);
    auto *rowData              = new uint8_t[bytesPerRow];

    for (uint32_t yy = y; yy < y + height; yy++) {
        png_read_row(png_ptr, (png_bytep) rowData, nullptr);

        for (uint32_t xx = x; xx < x + width; xx++) {
            if (colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
                uint32_t i = (xx - x) * 4;
                drawPixel(xx, yy, rowData[i], rowData[i + 1], rowData[i + 2], rowData[i + 3]);
            } else if (colorType == PNG_COLOR_TYPE_RGB) {
                uint32_t i = (xx - x) * 3;
                drawPixel(xx, yy, rowData[i], rowData[i + 1], rowData[i + 2], 0xFF);
            }
        }
    }

    delete[] rowData;
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
}

bool DrawUtils::initFont() {
    void *font    = nullptr;
    uint32_t size = 0;
    OSGetSharedData(OS_SHAREDDATATYPE_FONT_STANDARD, 0, &font, &size);

    if (font && size) {
        pFont.xScale = 20;
        pFont.yScale = 20,
        pFont.flags  = SFT_DOWNWARD_Y;
        pFont.font   = sft_loadmem(font, size);
        if (!pFont.font) {
            return false;
        }
        OSMemoryBarrier();
        return true;
    }
    return false;
}

void DrawUtils::deinitFont() {
    sft_freefont(pFont.font);
    pFont.font = nullptr;
    pFont      = {};
}

void DrawUtils::setFontSize(uint32_t size) {
    pFont.xScale = size;
    pFont.yScale = size;
    SFT_LMetrics metrics;
    sft_lmetrics(&pFont, &metrics);
}

void DrawUtils::setFontColor(const Color col) {
    font_col = col;
}

static void draw_freetype_bitmap(const SFT_Image *bmp, const int32_t x, const int32_t y) {
    int32_t i, j, p, q;

    int32_t x_max = x + bmp->width;
    int32_t y_max = y + bmp->height;

    const auto *src = static_cast<uint8_t *>(bmp->pixels);

    for (i = x, p = 0; i < x_max; i++, p++) {
        for (j = y, q = 0; j < y_max; j++, q++) {
            if (i < 0 || j < 0 || i >= SCREEN_WIDTH || j >= SCREEN_HEIGHT) {
                continue;
            }

            const float opacity = src[q * bmp->width + p] / 255.0f;
            DrawUtils::drawPixel(i, j, font_col.r, font_col.g, font_col.b, font_col.a * opacity);
        }
    }
}

void DrawUtils::print(const uint32_t x, const uint32_t y, const char *string, const bool alignRight) {
    auto *buffer = new wchar_t[strlen(string) + 1];

    size_t num = mbstowcs(buffer, string, strlen(string));
    if (num > 0) {
        buffer[num] = 0;
    } else {
        wchar_t *tmp = buffer;
        while ((*tmp++ = *string++))
            ;
    }

    print(x, y, buffer, alignRight);
    delete[] buffer;
}

void DrawUtils::print(const uint32_t x, const uint32_t y, const wchar_t *string, const bool alignRight) {
    auto penX = static_cast<int32_t>(x);
    auto penY = static_cast<int32_t>(y);

    if (alignRight) {
        penX -= getTextWidth(string);
    }

    uint16_t textureWidth = 0, textureHeight = 0;
    for (; *string; string++) {
        SFT_Glyph gid; //  unsigned long gid;
        if (sft_lookup(&pFont, *string, &gid) >= 0) {
            SFT_GMetrics mtx;
            if (sft_gmetrics(&pFont, gid, &mtx) < 0) {
                DEBUG_FUNCTION_LINE_ERR("Failed to get glyph metrics");
                return;
            }

            if (*string == '\n') {
                penY += mtx.minHeight;
                penX = x;
                continue;
            }

            textureWidth  = (mtx.minWidth + 3) & ~3;
            textureHeight = mtx.minHeight;

            if (textureWidth == 0) {
                textureWidth = 4;
            }
            if (textureHeight == 0) {
                textureHeight = 4;
            }

            SFT_Image img = {
                    .pixels = nullptr,
                    .width  = textureWidth,
                    .height = textureHeight,
            };

            auto buffer = make_unique_nothrow<uint8_t[]>((uint32_t) (img.width * img.height));
            if (!buffer) {
                DEBUG_FUNCTION_LINE_ERR("Failed to allocate memory for glyph");
                return;
            }
            img.pixels = buffer.get();
            if (sft_render(&pFont, gid, img) < 0) {
                DEBUG_FUNCTION_LINE_ERR("Failed to render glyph");
                return;
            } else {
                draw_freetype_bitmap(&img, static_cast<int32_t>(penX + mtx.leftSideBearing), penY + mtx.yOffset);
                penX += static_cast<int32_t>(mtx.advanceWidth);
            }
        }
    }
}

uint32_t DrawUtils::getTextWidth(const char *string) {
    auto *buffer = new wchar_t[strlen(string) + 1];

    if (const size_t num = mbstowcs(buffer, string, strlen(string)); num > 0) {
        buffer[num] = 0;
    } else {
        wchar_t *tmp = buffer;
        while ((*tmp++ = *string++))
            ;
    }

    const uint32_t width = getTextWidth(buffer);
    delete[] buffer;

    return width;
}

uint32_t DrawUtils::getTextWidth(const wchar_t *string) {
    uint32_t width = 0;

    for (; *string; string++) {
        SFT_Glyph gid; //  unsigned long gid;
        if (sft_lookup(&pFont, *string, &gid) >= 0) {
            SFT_GMetrics mtx;
            if (sft_gmetrics(&pFont, gid, &mtx) < 0) {
                DEBUG_FUNCTION_LINE_ERR("bad glyph metrics");
            }
            width += static_cast<int32_t>(mtx.advanceWidth);
        }
    }

    return width;
}
