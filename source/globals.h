#pragma once

#include <wums.h>

#include "plugin/PluginContainer.h"
#include "common/plugin_defines.h"

extern plugin_information_t *gPluginInformation;
extern MEMHeapHandle pluginDataHeap;
extern plugin_information_on_reload_t gLinkOnReload;
extern module_information_t *gModuleData;

extern "C" void __init_wut();
extern "C" void __fini_wut();