#pragma once

#include <wums.h>

#include "plugin/PluginContainer.h"
#include "common/plugin_defines.h"
#include "utils/ConfigUtils.h"

extern plugin_information_t *gPluginInformation;
extern MEMHeapHandle gPluginDataHeap;
extern MEMHeapHandle gPluginInformationHeap;
extern uint32_t gPluginDataHeapSize;
extern uint32_t gPluginInformationHeapSize;
extern plugin_information_on_reload_t gLinkOnReload;
extern module_information_t *gModuleData;
extern relocation_trampoline_entry_t *gTrampolineData;
extern uint32_t gTrampolineDataSize;
extern StoredBuffer storedTVBuffer;
extern StoredBuffer storedDRCBuffer;

#define PLUGIN_DATA_HEAP_SIZE (8 * 1024 * 1024)
#define NUMBER_OF_TRAMPS 1024