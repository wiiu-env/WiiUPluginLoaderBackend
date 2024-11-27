#include "globals.h"
#include "plugin/PluginContainer.h"
#include "plugin/PluginData.h"

StoredBuffer gStoredTVBuffer  = {};
StoredBuffer gStoredDRCBuffer = {};

std::vector<PluginContainer> gLoadedPlugins;
std::vector<relocation_trampoline_entry_t> gTrampData;

std::set<std::shared_ptr<PluginData>, PluginDataSharedPtrComparator> gLoadedData;
std::vector<std::shared_ptr<PluginData>> gLoadOnNextLaunch;
std::mutex gLoadedDataMutex;
std::map<std::string, OSDynLoad_Module> gUsedRPLs;
std::vector<void *> gAllocatedAddresses;

bool gNotificationModuleLoaded = false;

OSThread *gOnlyAcceptFromThread = nullptr;

bool gConfigMenuShouldClose = false;