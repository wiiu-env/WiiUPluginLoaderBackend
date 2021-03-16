#include "hooks_patcher_static.h"
#include <malloc.h>
#include <wups.h>
#include <vpad/input.h>
#include <coreinit/messagequeue.h>
#include <coreinit/core.h>

#include "../utils/logger.h"
#include "../globals.h"
#include "../hooks.h"

DECL_FUNCTION(void, GX2WaitForVsync, void) {
    CallHook(gPluginInformation, WUPS_LOADER_HOOK_VSYNC);
    real_GX2WaitForVsync();
}

static uint32_t lastData0 = 0;
DECL_FUNCTION(uint32_t, OSReceiveMessage, OSMessageQueue *queue, OSMessage *message, uint32_t flags) {
    int32_t res = real_OSReceiveMessage(queue, message, flags);
    if (queue == OSGetSystemMessageQueue()) {
        if (message != nullptr && res) {
            if (lastData0 != message->args[0]) {
                if (message->args[0] == 0xFACEF000) {
                    CallHook(gPluginInformation, WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND);
                } else if (message->args[0] == 0xD1E0D1E0) {
                   // Implemented via WUMS Hook
                }
            }
            lastData0 = message->args[0];
        }
    }
    return res;
}

DECL_FUNCTION(void, OSReleaseForeground) {
    if (OSGetCoreId() == 1) {
        CallHook(gPluginInformation, WUPS_LOADER_HOOK_RELEASE_FOREGROUND);
    }
    real_OSReleaseForeground();
}

function_replacement_data_t method_hooks_hooks_static[] __attribute__((section(".data"))) = {
        REPLACE_FUNCTION(GX2WaitForVsync,             LIBRARY_GX2,        GX2WaitForVsync),
        REPLACE_FUNCTION(OSReceiveMessage, LIBRARY_COREINIT, OSReceiveMessage),
        REPLACE_FUNCTION(OSReleaseForeground, LIBRARY_COREINIT, OSReleaseForeground)
};

uint32_t method_hooks_size_hooks_static __attribute__((section(".data"))) = sizeof(method_hooks_hooks_static) / sizeof(function_replacement_data_t);