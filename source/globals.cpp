#include "globals.h"

StoredBuffer gStoredTVBuffer __attribute__((section(".data")))  = {};
StoredBuffer gStoredDRCBuffer __attribute__((section(".data"))) = {};

std::vector<std::unique_ptr<PluginContainer>> gLoadedPlugins __attribute__((section(".data")));
relocation_trampoline_entry_t *gTrampData __attribute__((section(".data"))) = nullptr;

std::forward_list<std::shared_ptr<PluginData>> gLoadedData __attribute__((section(".data")));
std::forward_list<std::shared_ptr<PluginData>> gLoadOnNextLaunch __attribute__((section(".data")));
std::mutex gLoadedDataMutex __attribute__((section(".data")));
std::map<std::string, OSDynLoad_Module> gUsedRPLs __attribute__((section(".data")));
std::vector<void *> gAllocatedAddresses __attribute__((section(".data")));

bool gNotificationModuleLoaded __attribute__((section(".data"))) = false;