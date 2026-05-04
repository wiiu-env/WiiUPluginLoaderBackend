#include "DrawUtils.h"

#include "dc.h"
#include "globals.h"
#include "logger.h"
#include "utils.h"

#include <png.h>

#include <memory/mappedmemory.h>

#include <avm/tv.h>
#include <coreinit/cache.h>
#include <coreinit/memory.h>
#include <coreinit/screen.h>
#include <padscore/kpad.h>

#include <algorithm>
#include <cstdlib>
#include <unordered_map>
#include <vector>

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


struct CachedGlyph {
    SFT_GMetrics metrics;
    std::vector<uint8_t> pixels;
    uint16_t width;
    uint16_t height;
};

static std::unordered_map<uint64_t, CachedGlyph> mGlyphCache;

static const CachedGlyph &getOrCacheGlyph(wchar_t character) {
    const uint64_t cacheKey = (static_cast<uint64_t>(pFont.xScale) << 32) | static_cast<uint64_t>(character);

    if (const auto it = mGlyphCache.find(cacheKey); it != mGlyphCache.end()) {
        return it->second;
    }

    CachedGlyph newGlyph;
    newGlyph.width                   = 0;
    newGlyph.height                  = 0;
    newGlyph.metrics.advanceWidth    = 0;
    newGlyph.metrics.minHeight       = 0;
    newGlyph.metrics.leftSideBearing = 0;
    newGlyph.metrics.yOffset         = 0;

    SFT_Glyph gid;
    if (sft_lookup(&pFont, character, &gid) >= 0) {
        if (sft_gmetrics(&pFont, gid, &newGlyph.metrics) >= 0) {
            uint16_t texWidth  = (newGlyph.metrics.minWidth + 3) & ~3;
            uint16_t texHeight = newGlyph.metrics.minHeight;
            if (texWidth == 0) texWidth = 4;
            if (texHeight == 0) texHeight = 4;

            newGlyph.width  = texWidth;
            newGlyph.height = texHeight;

            newGlyph.pixels.resize(texWidth * texHeight, 0);

            SFT_Image img = {
                    .pixels = newGlyph.pixels.data(),
                    .width  = texWidth,
                    .height = texHeight,
            };

            sft_render(&pFont, gid, img);
        }
    }

    mGlyphCache[cacheKey] = std::move(newGlyph);
    return mGlyphCache[cacheKey];
}

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

static void FastMemset32(void *dest, uint32_t value, uint32_t size) {
    // Handle Leading Alignment
    if ((reinterpret_cast<uint32_t>(dest) & 31) != 0) {
        while ((reinterpret_cast<uint32_t>(dest) & 31) != 0 && size >= 4) {
            *static_cast<uint32_t *>(dest) = value;
            dest                           = static_cast<uint8_t *>(dest) + 4;
            size -= 4;
        }
    }

    auto *d             = static_cast<uint32_t *>(dest);
    uint32_t num_blocks = size / 32;
    uint32_t remainder  = (size % 32) / 4;

    if (value == 0) { // Fast Path if value is 0 (DCZeroRange)
        for (uint32_t i = 0; i < num_blocks; i++) {
            __asm__ volatile("dcbz 0, %0"
                             :
                             : "r"(d));
            d += 8;
        }
    } else { // Save as floats to cut numer of instructions in half
        union {
            double f;
            uint32_t u[2];
        } cast;
        cast.u[0]      = value;
        cast.u[1]      = value;
        double f_color = cast.f;

        for (uint32_t i = 0; i < num_blocks; i++) {
            // Claim/Zero the 32bit cache line
            __asm__ volatile("dcbz 0, %0"
                             :
                             : "r"(d));

            // Overwrite with the actual value using 64-bit FPU stores
            __asm__ volatile(
                    "stfd %[col], 0(%[ptr])\n"
                    "stfd %[col], 8(%[ptr])\n"
                    "stfd %[col], 16(%[ptr])\n"
                    "stfd %[col], 24(%[ptr])\n"
                    :
                    : [col] "f"(f_color), [ptr] "r"(d)
                    : "memory");
            d += 8;
        }
    }

    // Handle Trailing Remainder
    for (uint32_t i = 0; i < remainder; i++) {
        d[i] = value;
    }
}

void DrawUtils::clear(const Color col) {
    const uint32_t drcOffset = mIsBackBuffer ? (mDRCSize / 2) : 0;
    const uint32_t tvOffset  = mIsBackBuffer ? (mTVSize / 2) : 0;
    uint32_t val             = col.color;
    if (val == 0x000000FF) {
        val = 0;
    }
    if (mDRCBuffer) {
        FastMemset32(mDRCBuffer + drcOffset, val, mDRCSize / 2);
    }
    if (mTVBuffer) {
        FastMemset32(mTVBuffer + tvOffset, val, mTVSize / 2);
    }
}

void DrawUtils::drawPixel(const uint32_t x, const uint32_t y, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
    if (a == 0) {
        return;
    }

    // put pixel in the drc buffer
    uint32_t i              = (x + y * DRC_WIDTH) * 4;
    const uint32_t drcLimit = mDRCSize / 2;
    if (i + 3 < drcLimit) {
        if (mIsBackBuffer) {
            i += drcLimit;
        }
        if (a == 0xFF) {
            mDRCBuffer[i]     = r;
            mDRCBuffer[i + 1] = g;
            mDRCBuffer[i + 2] = b;
        } else {
            const uint32_t inv_a = 255 - a;
            mDRCBuffer[i]        = (r * a + mDRCBuffer[i] * inv_a) >> 8;
            mDRCBuffer[i + 1]    = (g * a + mDRCBuffer[i + 1] * inv_a) >> 8;
            mDRCBuffer[i + 2]    = (b * a + mDRCBuffer[i + 2] * inv_a) >> 8;
        }
    }

    // Corrected scaling for TV to eliminate gaps
    const auto startX = static_cast<uint32_t>(x * mUsedTVScale);
    const auto startY = static_cast<uint32_t>(y * mUsedTVScale);

    // Calculate the end boundary by scaling the NEXT logical coordinate
    const auto endX = static_cast<uint32_t>((x + 1) * mUsedTVScale);
    const auto endY = static_cast<uint32_t>((y + 1) * mUsedTVScale);

    const uint32_t tvLimit          = mTVSize / 2;
    const uint32_t backBufferOffset = mIsBackBuffer ? tvLimit : 0;

    for (uint32_t yy = startY; yy < endY; yy++) {
        uint32_t rowOffset = (yy * DrawUtils::mUsedTVWidth) * 4 + backBufferOffset;
        for (uint32_t xx = startX; xx < endX; xx++) {
            uint32_t j = (xx * 4) + rowOffset;
            if (j + 3 < (mIsBackBuffer ? mTVSize : tvLimit)) {
                if (a == 0xFF) {
                    mTVBuffer[j]     = r;
                    mTVBuffer[j + 1] = g;
                    mTVBuffer[j + 2] = b;
                } else {
                    const uint32_t inv_a = 255 - a;
                    mTVBuffer[j]         = (r * a + mTVBuffer[j] * inv_a) >> 8;
                    mTVBuffer[j + 1]     = (g * a + mTVBuffer[j + 1] * inv_a) >> 8;
                    mTVBuffer[j + 2]     = (b * a + mTVBuffer[j + 2] * inv_a) >> 8;
                }
            }
        }
    }
}

void DrawUtils::drawRectFilled(const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h, const Color col) {
    if (col.a == 0) return;

    // Use fast path for opaque rectangles
    if (col.a == 0xFF) {
        const uint32_t color32 = (col.r << 24) | (col.g << 16) | (col.b << 8) | 0xFF;

        uint32_t drcOffset = mIsBackBuffer ? (mDRCSize / 2) : 0;
        auto *drc32        = reinterpret_cast<uint32_t *>(mDRCBuffer + drcOffset);
        for (uint32_t yy = y; yy < y + h; yy++) {
            std::fill_n(drc32 + x + (yy * DRC_WIDTH), w, color32);
        }

        uint32_t tvOffset      = mIsBackBuffer ? (mTVSize / 2) : 0;
        auto *tv32             = reinterpret_cast<uint32_t *>(mTVBuffer + tvOffset);
        const auto startX      = static_cast<uint32_t>(x * mUsedTVScale);
        const auto startY      = static_cast<uint32_t>(y * mUsedTVScale);
        const auto endX        = static_cast<uint32_t>((x + w) * mUsedTVScale);
        const auto endY        = static_cast<uint32_t>((y + h) * mUsedTVScale);
        const uint32_t scaledW = endX - startX;

        for (uint32_t yy = startY; yy < endY; yy++) {
            std::fill_n(tv32 + startX + (yy * mUsedTVWidth), scaledW, color32);
        }
    } else {
        // Fallback to per-pixel for transparency
        for (uint32_t yy = y; yy < y + h; yy++) {
            for (uint32_t xx = x; xx < x + w; xx++) {
                drawPixel(xx, yy, col);
            }
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
    mGlyphCache.clear();
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
    int32_t j, q;

    int32_t x_max = x + bmp->width;
    int32_t y_max = y + bmp->height;

    const auto *src = static_cast<uint8_t *>(bmp->pixels);

    for (j = y, q = 0; j < y_max; j++, q++) {
        if (j < 0 || j >= SCREEN_HEIGHT) {
            continue;
        }

        for (int32_t i = x, p = 0; i < x_max; i++, p++) {
            if (i < 0 || i >= SCREEN_WIDTH) {
                continue;
            }

            const uint32_t alpha = src[q * bmp->width + p];
            if (alpha == 0) {
                continue;
            }

            DrawUtils::drawPixel(i, j, font_col.r, font_col.g, font_col.b, (font_col.a * alpha) >> 8);
        }
    }
}

void DrawUtils::print(const uint32_t x, const uint32_t y, const char *string, const bool alignRight) {
    if (!string || *string == '\0') return;

    const size_t len = strlen(string);
    wchar_t stackBuffer[256];
    wchar_t *buffer = stackBuffer;

    if (len >= 256) {
        buffer = new wchar_t[len + 1];
    }

    size_t num = mbstowcs(buffer, string, len);
    if (num > 0) {
        buffer[num] = 0;
    } else {
        wchar_t *dest   = buffer;
        const char *src = string;
        while ((*dest++ = *src++))
            ;
    }

    print(x, y, buffer, alignRight);

    if (buffer != stackBuffer) {
        delete[] buffer;
    }
}

void DrawUtils::print(const uint32_t x, const uint32_t y, const wchar_t *string, const bool alignRight) {
    auto penX = static_cast<int32_t>(x);
    auto penY = static_cast<int32_t>(y);

    if (alignRight) {
        penX -= getTextWidth(string);
    }

    for (; *string; string++) {
        const CachedGlyph &glyph = getOrCacheGlyph(*string);

        if (*string == '\n') {
            penY += glyph.metrics.minHeight;
            penX = x;
            continue;
        }

        if (glyph.width > 0 && glyph.height > 0) {
            SFT_Image img = {
                    .pixels = (void *) glyph.pixels.data(),
                    .width  = glyph.width,
                    .height = glyph.height,
            };
            draw_freetype_bitmap(&img, static_cast<int32_t>(penX + glyph.metrics.leftSideBearing), penY + glyph.metrics.yOffset);
        }

        penX += static_cast<int32_t>(glyph.metrics.advanceWidth);
    }
}

uint32_t DrawUtils::getTextWidth(const char *string) {
    if (!string || *string == '\0') return 0;

    const size_t len = strlen(string);

    // Stack buffer for typical strings
    wchar_t stackBuffer[256];
    wchar_t *buffer = stackBuffer;

    // Fall back to heap ONLY if the string is longer than our stack buffer
    if (len >= 256) {
        buffer = new wchar_t[len + 1];
    }

    if (const size_t num = mbstowcs(buffer, string, len); num > 0) {
        buffer[num] = 0;
    } else {
        // Fallback: simple copy if mbstowcs fails
        wchar_t *dest   = buffer;
        const char *src = string;
        while ((*dest++ = *src++))
            ;
    }

    const uint32_t width = getTextWidth(buffer);

    // Clean up only if we actually used the heap
    if (buffer != stackBuffer) {
        delete[] buffer;
    }

    return width;
}

uint32_t DrawUtils::getTextWidth(const wchar_t *string) {
    uint32_t width = 0;
    for (; *string; string++) {
        const CachedGlyph &glyph = getOrCacheGlyph(*string);
        width += static_cast<int32_t>(glyph.metrics.advanceWidth);
    }
    return width;
}

void DrawUtils::RenderScreen(const std::function<void()> &callback) {
    gOnlyAcceptFromThread         = OSGetCurrentThread();
    bool wasHomeButtonMenuEnabled = OSIsHomeButtonMenuEnabled();

    // Save copy of DC reg values
    auto tvRender1 = DCReadReg32(SCREEN_TV, D1GRPH_CONTROL_REG);
    auto tvRender2 = DCReadReg32(SCREEN_TV, D1GRPH_ENABLE_REG);
    auto tvPitch1  = DCReadReg32(SCREEN_TV, D1GRPH_PITCH_REG);
    auto tvPitch2  = DCReadReg32(SCREEN_TV, D1OVL_PITCH_REG);

    auto drcRender1 = DCReadReg32(SCREEN_DRC, D1GRPH_CONTROL_REG);
    auto drcRender2 = DCReadReg32(SCREEN_DRC, D1GRPH_ENABLE_REG);
    auto drcPitch1  = DCReadReg32(SCREEN_DRC, D1GRPH_PITCH_REG);
    auto drcPitch2  = DCReadReg32(SCREEN_DRC, D1OVL_PITCH_REG);

    OSScreenInit();

    uint32_t screen_buf0_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t screen_buf1_size = OSScreenGetBufferSizeEx(SCREEN_DRC);
    void *screenbuffer0       = MEMAllocFromMappedMemoryForGX2Ex(screen_buf0_size, 0x100);
    void *screenbuffer1       = MEMAllocFromMappedMemoryForGX2Ex(screen_buf1_size, 0x100);

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

    FastMemset32(screenbuffer0, (COLOR_BACKGROUND).color, screen_buf0_size);
    FastMemset32(screenbuffer1, (COLOR_BACKGROUND).color, screen_buf1_size);

    OSScreenSetBufferEx(SCREEN_TV, screenbuffer0);
    OSScreenSetBufferEx(SCREEN_DRC, screenbuffer1);

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

    callback();

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