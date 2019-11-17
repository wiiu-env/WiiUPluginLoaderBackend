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
#include "utils/mem_utils.h"
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

        CallHookEx(WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB,plugin_index);
        CallHookEx(WUPS_LOADER_HOOK_APPLICATION_START,plugin_index);
        new_PatchInvidualMethodHooks(&gbl_replacement_data.plugin_data[plugin_index]);
        CallHookEx(WUPS_LOADER_HOOK_FUNCTIONS_PATCHED,plugin_index);
    }
}

void RestorePatches() {
    for(int32_t plugin_index=gbl_replacement_data.number_used_plugins-1; plugin_index>=0; plugin_index--) {
        new_RestoreInvidualInstructions(&gbl_replacement_data.plugin_data[plugin_index]);
    }
    RestoreInvidualInstructions(method_hooks_hooks, method_hooks_size_hooks);
    RestoreInvidualInstructions(method_hooks_hooks_static, method_hooks_size_hooks_static);
}

void RestoreEverything() {
    CallHook(WUPS_LOADER_HOOK_RELEASE_FOREGROUND);
    CallHook(WUPS_LOADER_HOOK_APPLICATION_END);
    CallHook(WUPS_LOADER_HOOK_DEINIT_PLUGIN);

    CallHook(WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB);
    CallHook(WUPS_LOADER_HOOK_FINI_WUT_STDCPP);
    CallHook(WUPS_LOADER_HOOK_FINI_WUT_NEWLIB);
    CallHook(WUPS_LOADER_HOOK_FINI_WUT_MALLOC);

    // Restore patches as the patched functions could change.
    RestorePatches();
    DynamicLinkingHelper::getInstance()->clearAll();
    gInBackground = false;
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

    CallHook(WUPS_LOADER_HOOK_INIT_WUT_MALLOC);
    CallHook(WUPS_LOADER_HOOK_INIT_WUT_NEWLIB);
    CallHook(WUPS_LOADER_HOOK_INIT_WUT_STDCPP);

    CallHook(WUPS_LOADER_HOOK_INIT_VID_MEM);
    CallHook(WUPS_LOADER_HOOK_INIT_KERNEL);
    CallHook(WUPS_LOADER_HOOK_INIT_OVERLAY);
    ConfigUtils::loadConfigFromSD();
    CallHook(WUPS_LOADER_HOOK_INIT_PLUGIN);
}

extern "C" void doStart(int argc, char **argv);
// We need to wrap it to make sure the main function is called AFTER our code.
// The compiler tries to optimize this otherwise and calling the main function earlier
extern "C" int _start(int argc, char **argv) {
    doStart(argc,argv);
    return ( (int (*)(int, char **))(*(unsigned int*)0x1005E040) )(argc, argv);
}

extern "C" void doStart(int argc, char **argv) {
    coreinit_handle = 0;
    InitOSFunctionPointers();

    if(gInBackground) {
        CallHook(WUPS_LOADER_HOOK_APPLET_START);
        return;
    }

    gInBackground = true;

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

    log_init();

    if(!MemoryMapping::isMemoryMapped()) {
        MemoryMapping::setupMemoryMapping();
        memset((void*)&gbl_replacement_data,0,sizeof(gbl_replacement_data));
        DCFlushRange((void*)&gbl_replacement_data,sizeof(gbl_replacement_data));
        ICInvalidateRange((void*)&gbl_replacement_data,sizeof(gbl_replacement_data));

        memset((void*)&gbl_dyn_linking_data,0,sizeof(gbl_dyn_linking_data));
        DCFlushRange((void*)&gbl_dyn_linking_data,sizeof(gbl_dyn_linking_data));
        ICInvalidateRange((void*)&gbl_dyn_linking_data,sizeof(gbl_dyn_linking_data));

        // Switch to custom heap
        //Does not work =(
        //initMemory();
        //SplashScreen(1, "Memory mapping was completed!", 0,0);

        memset(gbl_to_link_and_load_data,0, sizeof(gbl_to_link_and_load_data));

        // Init space
        DynamicLinkingHelper::getInstance()->clearAll();

        // Init IPC
        uint32_t* ipcFunction = (uint32_t*)(MEMORY_START_PLUGIN_SPACE);
        *ipcFunction = (uint32_t) &ipc_ioctl;
        DCFlushRange(ipcFunction,4);
        ICInvalidateRange(ipcFunction,4);

        // Do patches so memory mapping works fine with some functions.
        PatchInvidualMethodHooks(method_hooks_hooks_static, method_hooks_size_hooks_static, method_calls_hooks_static);

        // mount sd card.
        mount_sd_fat("sd");

        DEBUG_FUNCTION_LINE("Mounted SD\n");

        PluginLoader * pluginLoader = new PluginLoader((void*)PLUGIN_LOCATION_START_ADDRESS, (void*)PLUGIN_LOCATION_END_ADDRESS);
        std::vector<PluginInformation *> pluginList = pluginLoader->getPluginInformation("sd:/wiiu/autoboot_plugins/");
        pluginLoader->loadAndLinkPlugins(pluginList);
        pluginLoader->clearPluginInformation(pluginList);
        delete pluginLoader;
        afterLoadAndLink();
    } else {
        DEBUG_FUNCTION_LINE("Mapping was already done\n");
        //unmount_sd_fat("sd");
        mount_sd_fat("sd");

        //SplashScreen(5, "Memory mapping was already done!!", 0,0);
        //readAndPrintSegmentRegister(NULL,NULL);
        //MemoryMapping::writeTestValuesToMemory();
        //MemoryMapping::readTestValuesFromMemory();
    }

    if(gbl_to_link_and_load_data[0].name[0] != 0) {
        ResolveRelocations();
        CallHook(WUPS_LOADER_HOOK_DEINIT_PLUGIN);

        // Restore patches as the patched functions could change.
        RestorePatches();
        DynamicLinkingHelper::getInstance()->clearAll();

        PluginLoader * pluginLoader = new PluginLoader((void*)PLUGIN_LOCATION_START_ADDRESS, (void*)PLUGIN_LOCATION_END_ADDRESS);
        std::vector<PluginInformation *> pluginList;
        for(int i = 0; gbl_to_link_and_load_data[i].name[0] != 0; i++) {
            PluginInformation * info = PluginInformation::loadPluginInformation(gbl_to_link_and_load_data[i].name);
            if(info != NULL) {
                pluginList.push_back(info);
            }
        }
        pluginLoader->loadAndLinkPlugins(pluginList);
        pluginLoader->clearPluginInformation(pluginList);
        delete pluginLoader;
        afterLoadAndLink();
        memset(gbl_to_link_and_load_data,0, sizeof(gbl_to_link_and_load_data));
    }

    ResolveRelocations();

    MemoryUtils::init();

    DEBUG_FUNCTION_LINE("Apply patches.\n");
    ApplyPatchesAndCallHookStartingApp();
    DEBUG_FUNCTION_LINE("Patches applied. Running application.\n");

}
