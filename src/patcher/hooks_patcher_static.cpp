#include "utils/logger.h"
#include "utils/function_patcher.h"
#include "common/retain_vars.h"
#include "hooks_patcher.h"
#include "utils/overlay_helper.h"
#include "utils/ConfigUtils.h"
#include <malloc.h>
#include "utils.h"
#include "memory/memory_mapping.h"
#include "utils/mem_utils.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/coreinit.h"
#include "fs/sd_fat_devoptab.h"
//#include "utils/texture_utils.h"

DECL_FUNCTION(uint32_t, __OSPhysicalToEffectiveCached, uint32_t phyiscalAddress) {
    uint32_t result = real___OSPhysicalToEffectiveCached(phyiscalAddress);
    if(result == 0) {
        result = MemoryMapping::PhysicalToEffective(phyiscalAddress);
        //DEBUG_FUNCTION_LINE("__OSPhysicalToEffectiveCached in %08X out %08X\n",phyiscalAddress,result);
    }
    return result;
}

DECL_FUNCTION(uint32_t, __OSPhysicalToEffectiveUncached, uint32_t phyiscalAddress) {
    uint32_t result = real___OSPhysicalToEffectiveUncached(phyiscalAddress);
    if(result == 0) {
        result = MemoryMapping::PhysicalToEffective(phyiscalAddress);
        //DEBUG_FUNCTION_LINE("__OSPhysicalToEffectiveUncached in %08X out %08X\n",phyiscalAddress,result);
        return result;
    }
    return result;
}


DECL_FUNCTION(uint32_t, OSEffectiveToPhysical, uint32_t virtualAddress) {
    uint32_t result = real_OSEffectiveToPhysical(virtualAddress);
    if(result == 0) {
        result = MemoryMapping::EffectiveToPhysical(virtualAddress);
        //DEBUG_FUNCTION_LINE("OSEffectiveToPhysical in %08X out %08X\n",virtualAddress,result);
        return result;
    }
    return result;
}

DECL_FUNCTION(int32_t, OSIsAddressValid, uint32_t virtualAddress) {
    int32_t result = real_OSIsAddressValid(virtualAddress);
    if(result == 0) {
        result = (MemoryMapping::EffectiveToPhysical(virtualAddress) > 0);
        //DEBUG_FUNCTION_LINE("OSIsAddressValid in %08X out %d\n",virtualAddress,result);
        return result;
    }
    return result;
}

DECL(void, GX2SetTVBuffer, void *buffer, uint32_t buffer_size, int32_t tv_render_mode, int32_t format, int32_t buffering_mode) {
    tv_store.buffer = buffer;
    tv_store.buffer_size = buffer_size;
    tv_store.mode = tv_render_mode;
    tv_store.surface_format = format;
    tv_store.buffering_mode = buffering_mode;

    return real_GX2SetTVBuffer(buffer,buffer_size,tv_render_mode,format,buffering_mode);
}

DECL(void, GX2SetDRCBuffer, void *buffer, uint32_t buffer_size, int32_t drc_mode, int32_t surface_format, int32_t buffering_mode) {
    drc_store.buffer = buffer;
    drc_store.buffer_size = buffer_size;
    drc_store.mode = drc_mode;
    drc_store.surface_format = surface_format;
    drc_store.buffering_mode = buffering_mode;

    return real_GX2SetDRCBuffer(buffer,buffer_size,drc_mode,surface_format,buffering_mode);
}

DECL(void, GX2WaitForVsync, void) {
    CallHook(WUPS_LOADER_HOOK_VSYNC);
    real_GX2WaitForVsync();
}

uint8_t vpadPressCooldown = 0xFF;

uint8_t angleX_counter = 0;
float angleX_delta = 0.0f;
float angleX_last = 0.0f;
uint8_t angleX_frameCounter = 0;

void checkMagic(VPADData *buffer) {
    // buffer->angle stores the rotations per axis since the app started.
    // Each full rotation add/subtracts 1.0f (depending on the direction).

    // Check for rotation every only 5 frames.
    angleX_frameCounter++;
    if(angleX_frameCounter >= 5) {
        // Get how much the gamepad rotated within the last 5 frames.
        float diff_angle = -(buffer->angle.x - angleX_last);
        // We want the gamepad to make (on average) at least 0.16% (1/6) of a full rotation per 5 frames (for 6 times in a row).
        float target_diff = (0.16f);
        // Calculate if rotated enough in this step (including the delta from the last step).
        float total_diff = (diff_angle + angleX_delta) - target_diff;
        if(total_diff > 0.0f) {
            // The rotation in this step was enough.
            angleX_counter++;
            // When the gamepad rotated ~0.16% for 6 times in a row we made a full rotation!
            if(angleX_counter > 5) {
                ConfigUtils::openConfigMenu();
                // reset stuff.
                angleX_counter = 0;
                angleX_delta = 0.0f;
            } else {
                // Save difference as it will be added on the next check.
                angleX_delta = total_diff;
            }
        } else {
            // reset counter if it stopped rotating.
            angleX_counter = 0;
        }
        angleX_frameCounter = 0;
        angleX_last = buffer->angle.x;
    }
}

DECL(int32_t, VPADRead, int32_t chan, VPADData *buffer, uint32_t buffer_size, int32_t *error) {
    int32_t result = real_VPADRead(chan, buffer, buffer_size, error);

    if(result > 0 && (buffer[0].btns_h == (VPAD_BUTTON_PLUS | VPAD_BUTTON_R | VPAD_BUTTON_L)) && vpadPressCooldown == 0 && OSIsHomeButtonMenuEnabled()) {
        if(MemoryMapping::isMemoryMapped()) {
            MemoryMapping::readTestValuesFromMemory();
        } else {
            DEBUG_FUNCTION_LINE("Memory was not mapped. To test the memory please exit the plugin loader by pressing MINUS\n");
        }
        vpadPressCooldown = 0x3C;
    }

    if(result > 0 && (buffer[0].btns_h == (VPAD_BUTTON_L | VPAD_BUTTON_DOWN | VPAD_BUTTON_MINUS)) && vpadPressCooldown == 0 && OSIsHomeButtonMenuEnabled()) {
        ConfigUtils::openConfigMenu();
        vpadPressCooldown = 0x3C;
    } else if(result > 0 && OSIsHomeButtonMenuEnabled()) {
        checkMagic(buffer);
    }

    if(vpadPressCooldown > 0) {
        vpadPressCooldown--;
    }
    return result;
}
/*
void setupContextState() {
    g_vid_ownContextState = (GX2ContextState*)memalign(
                                GX2_CONTEXT_STATE_ALIGNMENT,
                                sizeof(GX2ContextState)
                            );
    if(g_vid_ownContextState == NULL) {
        OSFatal("VideoSquoosher: Failed to alloc g_vid_ownContextState\n");
    }
    GX2SetupContextStateEx(g_vid_ownContextState, 1);

    GX2SetContextState(g_vid_ownContextState);
    GX2SetColorBuffer(&g_vid_main_cbuf, GX2_RENDER_TARGET_0);
    //GX2SetDepthBuffer(&tvDepthBuffer);
    GX2SetContextState(g_vid_originalContextSave);
    DEBUG_FUNCTION_LINE("Setup contest state done\n");
}

void initTextures() {
    GX2InitColorBuffer(&g_vid_main_cbuf,
                       GX2_SURFACE_DIM_2D,
                       1280, 720, 1,
                       GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                       GX2_AA_MODE_1X
                      );

    if (g_vid_main_cbuf.surface.image_size) {
        g_vid_main_cbuf.surface.image_data = MemoryUtils::alloc(
                g_vid_main_cbuf.surface.image_size,
                g_vid_main_cbuf.surface.align
                                             );
        if(g_vid_main_cbuf.surface.image_data == NULL) {
            OSFatal("Failed to alloc g_vid_main_cbuf\n");
        }
        DEBUG_FUNCTION_LINE("Allocated %dx%d g_vid_main_cbuf %08X\n",
                            g_vid_main_cbuf.surface.width,
                            g_vid_main_cbuf.surface.height,
                            g_vid_main_cbuf.surface.image_data);
    } else {
        DEBUG_FUNCTION_LINE("GX2InitTexture failed for g_vid_main_cbuf!\n");
    }

    GX2InitTexture(&g_vid_drcTex,
                   854, 480, 1, 0,
                   GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                   GX2_SURFACE_DIM_2D,
                   GX2_TILE_MODE_LINEAR_ALIGNED
                  );
    g_vid_drcTex.surface.use = (GX2_SURFACE_USE_COLOR_BUFFER | GX2_SURFACE_USE_TEXTURE);

    if (g_vid_drcTex.surface.image_size) {

        g_vid_drcTex.surface.image_data = MemoryUtils::alloc(
                                              g_vid_drcTex.surface.image_size,
                                              g_vid_drcTex.surface.align);

        if(g_vid_drcTex.surface.image_data == NULL) {
            OSFatal("VideoSquoosher: Failed to alloc g_vid_drcTex\n");
        }
        GX2Invalidate(GX2_INVALIDATE_CPU, g_vid_drcTex.surface.image_data, g_vid_drcTex.surface.image_size);
        DEBUG_FUNCTION_LINE("VideoSquoosher: allocated %dx%d g_vid_drcTex %08X\n",
                            g_vid_drcTex.surface.width,
                            g_vid_drcTex.surface.height,
                            g_vid_drcTex.surface.image_data);

    } else {
        DEBUG_FUNCTION_LINE("VideoSquoosher: GX2InitTexture failed for g_vid_drcTex!\n");
    }

    GX2InitTexture(&g_vid_tvTex,
                   1280, 720, 1, 0,
                   GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
                   GX2_SURFACE_DIM_2D,
                   GX2_TILE_MODE_LINEAR_ALIGNED
                  );
    g_vid_tvTex.surface.use =
        (GX2_SURFACE_USE_COLOR_BUFFER | GX2_SURFACE_USE_TEXTURE);

    DCFlushRange(&g_vid_tvTex, sizeof(GX2Texture));

    if (g_vid_tvTex.surface.image_size) {
        g_vid_tvTex.surface.image_data = MemoryUtils::alloc(
                                             g_vid_tvTex.surface.image_size,
                                             g_vid_tvTex.surface.align
                                         );
        if(g_vid_tvTex.surface.image_data == NULL) {
            OSFatal("VideoSquoosher: Failed to alloc g_vid_tvTex\n");
        }
        GX2Invalidate(GX2_INVALIDATE_CPU, g_vid_tvTex.surface.image_data, g_vid_tvTex.surface.image_size);
        DEBUG_FUNCTION_LINE("VideoSquoosher: allocated %dx%d g_vid_tvTex %08X\n",
                            g_vid_tvTex.surface.width,
                            g_vid_tvTex.surface.height,
                            g_vid_tvTex.surface.image_data);
    } else {
        DEBUG_FUNCTION_LINE("VideoSquoosher: GX2InitTexture failed for g_vid_tvTex!\n");
    }

    GX2InitSampler(&g_vid_sampler,
                   GX2_TEX_CLAMP_CLAMP,
                   GX2_TEX_XY_FILTER_BILINEAR
                  );
}

DECL_FUNCTION(void, GX2SetContextState, GX2ContextState * curContext) {
    if(gAppStatus == WUPS_APP_STATUS_FOREGROUND) {
        g_vid_originalContextSave = curContext;
    }
    real_GX2SetContextState(curContext);
}

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, GX2ColorBuffer* cbuf, int32_t target) {
    bool hasDRCHook = HasHookCallHook(WUPS_LOADER_HOOK_VID_DRC_DRAW);
    bool hasTVHook = HasHookCallHook(WUPS_LOADER_HOOK_VID_TV_DRAW);
    if(gAppStatus != WUPS_APP_STATUS_FOREGROUND || !g_NotInLoader || (!hasDRCHook && !hasTVHook)) {
        return real_GX2CopyColorBufferToScanBuffer(cbuf,target);
    }

    if (!g_vid_drcTex.surface.image_data) {
        initTextures();
    }

    if(g_vid_ownContextState == NULL) {
        setupContextState();
    }

    if(target == 1) {
        TextureUtils::copyToTexture(cbuf,&g_vid_tvTex);
        if(!hasTVHook) {
            return real_GX2CopyColorBufferToScanBuffer(cbuf,target);
        }
    } else if(target == 4) {
        TextureUtils::copyToTexture(cbuf,&g_vid_drcTex);
        if(!hasDRCHook) {
            return real_GX2CopyColorBufferToScanBuffer(cbuf,target);
        }
    }

    GX2SetContextState(g_vid_ownContextState);
    GX2ClearColor(&g_vid_main_cbuf, 1.0f, 1.0f, 1.0f, 1.0f);
    GX2SetContextState(g_vid_ownContextState);

    GX2SetViewport(
        0.0f, 0.0f,
        g_vid_main_cbuf.surface.width, g_vid_main_cbuf.surface.height,
        0.0f, 1.0f
    );
    GX2SetScissor(
        0, 0,
        g_vid_main_cbuf.surface.width, g_vid_main_cbuf.surface.height
    );

    if(target == 1) {
        //drawTexture(&g_vid_tvTex, &g_vid_sampler, 0, 0, 1280, 720, 1.0f);
        CallHook(WUPS_LOADER_HOOK_VID_TV_DRAW);
    } else if(target == 4) {
        //drawTexture(&g_vid_drcTex, &g_vid_sampler, 0, 0, 1280, 720, 1.0f);
        CallHook(WUPS_LOADER_HOOK_VID_DRC_DRAW);
    }

    GX2SetContextState(g_vid_originalContextSave);

    return real_GX2CopyColorBufferToScanBuffer(&g_vid_main_cbuf,target);
}*/

static uint32_t lastData0 = 0;

DECL(uint32_t, OSReceiveMessage, OSMessageQueue *queue, OSMessage *message, uint32_t flags) {
    if(flags == 0x15154848) {
        CallHook(WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND);
        CallHook(WUPS_LOADER_HOOK_APPLICATION_END);
        CallHook(WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB);
        gInBackground = false;
        DCFlushRange(&gInBackground,4);
        return false;
    }
    int32_t res =  real_OSReceiveMessage(queue, message, flags);
    if(queue == OSGetSystemMessageQueue()) {
        if(message != NULL) {
            if(lastData0 != message->data0) {
                if(message->data0 == 0xFACEF000) {
                    CallHook(WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND);
                } else if(message->data0 == 0xD1E0D1E0) {
                    CallHook(WUPS_LOADER_HOOK_APPLICATION_END);
                    CallHook(WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB);
                    gInBackground = false;
                    DCFlushRange(&gInBackground,4);
                    unmount_sd_fat("sd");
                }
            }
            lastData0 = message->data0;
        }
    }
    return res;
}

DECL(void, OSReleaseForeground) {
    if(OSGetCoreId() == 1) {
        CallHook(WUPS_LOADER_HOOK_RELEASE_FOREGROUND);
    }
    real_OSReleaseForeground();
}

hooks_magic_t method_hooks_hooks_static[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(GX2SetTVBuffer,                  LIB_GX2,        STATIC_FUNCTION),
    MAKE_MAGIC(GX2SetDRCBuffer,                 LIB_GX2,        STATIC_FUNCTION),
    MAKE_MAGIC(GX2WaitForVsync,                 LIB_GX2,        STATIC_FUNCTION),
    //MAKE_MAGIC(GX2CopyColorBufferToScanBuffer,  LIB_GX2,        STATIC_FUNCTION),
    //MAKE_MAGIC(GX2SetContextState,              LIB_GX2,        STATIC_FUNCTION),
    MAKE_MAGIC(VPADRead,                        LIB_VPAD,       STATIC_FUNCTION),
    MAKE_MAGIC(OSIsAddressValid,                LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(__OSPhysicalToEffectiveUncached, LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(__OSPhysicalToEffectiveCached,   LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSEffectiveToPhysical,           LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSReceiveMessage,                  LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSReleaseForeground,               LIB_CORE_INIT,  STATIC_FUNCTION)
};

uint32_t method_hooks_size_hooks_static __attribute__((section(".data"))) = sizeof(method_hooks_hooks_static) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile uint32_t method_calls_hooks_static[sizeof(method_hooks_hooks_static) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

