#include <string.h>
#include <stdint.h>
#include "dynamic_libs/coreinit.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "kernel/kernel_utils.h"
#include "memory/memory_mapping.h"
#include "memory/memory.h"
#include "utils/ConfigUtils.h"
#include "utils/function_patcher.h"
#include "utils/logger.h"
#include "utils/ipc.h"
#include "utils.h"
#include "system/CThread.h"
#include "common/retain_vars.h"
#include "plugin/PluginLoader.h"
#include "plugin/DynamicLinkingHelper.h"

#include <fs/sd_fat_devoptab.h>

#include "patcher/function_patcher.h"
#include "patcher/hooks_patcher.h"
#include "patcher/hooks_patcher_static.h"

int SplashScreen(int32_t time,const char * message,uint8_t pos,uint32_t button) {
    //Call the Screen initilzation function.
    OSScreenInit();


    int result = 0;
    // Prepare screen
    int32_t screen_buf0_size = 0;

    // Init screen and screen buffers
    OSScreenInit();
    screen_buf0_size = OSScreenGetBufferSizeEx(0);
    OSScreenSetBufferEx(0, (void *)0xF4000000);
    OSScreenSetBufferEx(1, (void *)(0xF4000000 + screen_buf0_size));

    OSScreenEnableEx(0, 1);
    OSScreenEnableEx(1, 1);

    // Clear screens
    OSScreenClearBufferEx(0, 0);
    OSScreenClearBufferEx(1, 0);

    // Flip buffers
    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);

    OSScreenPutFontEx(0, 0, pos, message);
    OSScreenPutFontEx(1, 0, pos, message);

    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);

    int32_t tickswait = time * 1000*1000;
    int32_t times = 1000;
    int32_t sleepingtime = tickswait / 1000;
    int32_t i=0;

    void(*OSSleepTicks)(uint64_t x);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSSleepTicks", &OSSleepTicks);

    while(i<times) {
        i++;

        OSSleepTicks(OSMicrosecondsToTicks(sleepingtime));
    }
    return 1;
}

void readAndPrintSegmentRegister(CThread *thread, void *arg);


void ApplyPatchesAndCallHookStartingApp() {
    PatchInvidualMethodHooks(method_hooks_hooks_static, method_hooks_size_hooks_static, method_calls_hooks_static);
    PatchInvidualMethodHooks(method_hooks_hooks, method_hooks_size_hooks, method_calls_hooks);
    for(int32_t plugin_index=0; plugin_index<gbl_replacement_data.number_used_plugins; plugin_index++) {
        CallHookEx(WUPS_LOADER_HOOK_STARTING_APPLICATION,plugin_index);
        new_PatchInvidualMethodHooks(&gbl_replacement_data.plugin_data[plugin_index]);
        CallHookEx(WUPS_LOADER_HOOK_FUNCTIONS_PATCHED,plugin_index);
    }
}

void RestorePatches() {
    for(int32_t plugin_index=gbl_replacement_data.number_used_plugins-1; plugin_index>=0; plugin_index--) {
        DEBUG_FUNCTION_LINE("Restoring function for plugin: %d\n",plugin_index);
        new_RestoreInvidualInstructions(&gbl_replacement_data.plugin_data[plugin_index]);
    }
    RestoreInvidualInstructions(method_hooks_hooks, method_hooks_size_hooks);
    RestoreInvidualInstructions(method_hooks_hooks_static, method_hooks_size_hooks_static);
}

void RestoreEverything() {
    CallHook(WUPS_LOADER_HOOK_DEINIT_PLUGIN);
    uint32_t old = gAppStatus;
    gAppStatus = 3;
    CallHook(WUPS_LOADER_HOOK_APP_STATUS_CHANGED);
    gAppStatus = old;
    CallHook(WUPS_LOADER_HOOK_ENDING_APPLICATION);
    // Restore patches as the patched functions could change.
    RestorePatches();
    DynamicLinkingHelper::getInstance()->clearAll();
}


void ResolveRelocations() {
    std::vector<dyn_linking_relocation_entry_t *> relocations = DynamicLinkingHelper::getInstance()->getAllValidDynamicLinkingRelocations();
    DEBUG_FUNCTION_LINE("Found relocation information for %d functions\n",relocations.size());

    if(!DynamicLinkingHelper::getInstance()->fillRelocations(relocations)) {
        OSFatal("fillRelocations failed.");
    }

}

void afterLoadAndLink() {
    ResolveRelocations();

    CallHook(WUPS_LOADER_HOOK_INIT_VID_MEM);
    CallHook(WUPS_LOADER_HOOK_INIT_KERNEL);
    CallHook(WUPS_LOADER_HOOK_INIT_FS);
    CallHook(WUPS_LOADER_HOOK_INIT_OVERLAY);
    ConfigUtils::loadConfigFromSD();
    CallHook(WUPS_LOADER_HOOK_INIT_PLUGIN);
}


extern "C" int _start(int argc, char **argv) {
    if(gAppStatus == 2) {
        //"No, we don't want to patch stuff again.");
        return ( (int (*)(int, char **))(*(unsigned int*)0x1005E040) )(argc, argv);
    }
    coreinit_handle = 0;
    InitOSFunctionPointers();
    InitSocketFunctionPointers();
    InitFSFunctionPointers();
    InitVPadFunctionPointers();
    InitGX2FunctionPointers();

    // is called once.
    kernelInitialize();

    gGameTitleID = OSGetTitleID();

    memset(&tv_store,0,sizeof(tv_store));
    memset(&drc_store,0,sizeof(drc_store));


    if(!MemoryMapping::isMemoryMapped()) {
        MemoryMapping::setupMemoryMapping();
        // Switch to custom heap

        //Does not work =(
        //initMemory();
        //SplashScreen(1, "Memory mapping was completed!", 0,0);


        // Init space
        DynamicLinkingHelper::getInstance()->clearAll();

        // Init IPC
        uint32_t* ipcFunction = (uint32_t*)(MEMORY_START_PLUGIN_SPACE);
        *ipcFunction = (uint32_t) &ipc_ioctl;
        DCFlushRange(ipcFunction,4);
        ICInvalidateRange(ipcFunction,4);

        log_init();

        DEBUG_FUNCTION_LINE("log init done\n");

        DEBUG_FUNCTION_LINE("Patch own stuff\n");

        // Do patches so memmory mapping works fine with some functions.
        PatchInvidualMethodHooks(method_hooks_hooks_static, method_hooks_size_hooks_static, method_calls_hooks_static);

        // mount sd card.
        mount_sd_fat("sd");

        PluginLoader * pluginLoader = new PluginLoader((void*)PLUGIN_LOCATION_START_ADDRESS, (void*)PLUGIN_LOCATION_END_ADDRESS);

        std::vector<PluginInformation *> pluginList = pluginLoader->getPluginInformation("sd:/wiiu/autoboot_plugins/");
        pluginLoader->loadAndLinkPlugins(pluginList);
        pluginLoader->clearPluginInformation(pluginList);
        delete pluginLoader;

        afterLoadAndLink();
    } else {
        log_init();
        DEBUG_FUNCTION_LINE("Mapping was already done\n");
        //unmount_sd_fat("sd");
        mount_sd_fat("sd");

        //SplashScreen(5, "Memory mapping was already done!!", 0,0);
        //readAndPrintSegmentRegister(NULL,NULL);
        //MemoryMapping::writeTestValuesToMemory();
        //MemoryMapping::readTestValuesFromMemory();
    }

    std::vector<dyn_linking_relocation_entry_t *> relocations = DynamicLinkingHelper::getInstance()->getAllValidDynamicLinkingRelocations();
    DEBUG_FUNCTION_LINE("Found relocation information for %d functions\n",relocations.size());

    if(!DynamicLinkingHelper::getInstance()->fillRelocations(relocations)) {
        OSFatal("fillRelocations failed.");
    }

    DEBUG_FUNCTION_LINE("Apply patches.\n");
    ApplyPatchesAndCallHookStartingApp();

    return ( (int (*)(int, char **))(*(unsigned int*)0x1005E040) )(argc, argv);
}
