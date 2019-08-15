#include <utils/logger.h>
#include <utils/function_patcher.h>
#include "common/retain_vars.h"
#include "hooks_patcher.h"
#include "utils/overlay_helper.h"
#include "utils/ConfigUtils.h"
#include "utils.h"
#include "fs/sd_fat_devoptab.h"
//#include <video/shaders/Texture2DShader.h>

DECL(uint32_t, ProcUIProcessMessages, uint32_t u) {
    uint32_t res = real_ProcUIProcessMessages(u);
    // Only continue if we are in the "right" application.
    if(res != gAppStatus && OSGetTitleID() == gGameTitleID) {
        DEBUG_FUNCTION_LINE("App status changed from %d to %d \n",gAppStatus,res);
        gAppStatus = res;
        CallHook(WUPS_LOADER_HOOK_APP_STATUS_CHANGED);
        if(gAppStatus == WUPS_APP_STATUS_CLOSED) {
            CallHook(WUPS_LOADER_HOOK_ENDING_APPLICATION);
            ConfigUtils::saveConfigToSD();
            unmount_sd_fat("sd");
            //DeInit();
            //Texture2DShader::destroyInstance();
        }
    }

    return res;
}

hooks_magic_t method_hooks_hooks[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(ProcUIProcessMessages,           LIB_PROC_UI,    DYNAMIC_FUNCTION),
};


uint32_t method_hooks_size_hooks __attribute__((section(".data"))) = sizeof(method_hooks_hooks) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile uint32_t method_calls_hooks[sizeof(method_hooks_hooks) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

