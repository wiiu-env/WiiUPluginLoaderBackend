#include "globals.h"

MEMHeapHandle gPluginDataHeap __attribute__((section(".data")))            = nullptr;
MEMHeapHandle gPluginInformationHeap __attribute__((section(".data")))     = nullptr;
plugin_information_t *gPluginInformation __attribute__((section(".data"))) = nullptr;
plugin_information_on_reload_t gLinkOnReload __attribute__((section(".data")));
module_information_t *gModuleData __attribute__((section(".data")))              = nullptr;
relocation_trampoline_entry_t *gTrampolineData __attribute__((section(".data"))) = nullptr;

uint32_t gPluginDataHeapSize __attribute__((section(".data")))        = 0;
uint32_t gPluginInformationHeapSize __attribute__((section(".data"))) = 0;
uint32_t gTrampolineDataSize __attribute__((section(".data")))        = 0;

StoredBuffer storedTVBuffer __attribute__((section(".data")))  = {};
StoredBuffer storedDRCBuffer __attribute__((section(".data"))) = {};
