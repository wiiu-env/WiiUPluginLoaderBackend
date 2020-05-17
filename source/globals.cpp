#include "globals.h"

MEMHeapHandle pluginDataHeap __attribute__((section(".data"))) = 0;
plugin_information_t *gPluginInformation __attribute__((section(".data"))) = NULL;

plugin_information_on_reload_t gLinkOnReload __attribute__((section(".data")));