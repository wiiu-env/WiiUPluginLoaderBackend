#include "globals.h"

MEMHeapHandle pluginDataHeap __attribute__((section(".data"))) = nullptr;
plugin_information_t *gPluginInformation __attribute__((section(".data"))) = nullptr;
plugin_information_on_reload_t gLinkOnReload __attribute__((section(".data")));
module_information_t *gModuleData __attribute__((section(".data"))) = nullptr;

uint32_t gPluginDataHeapSize = 0;