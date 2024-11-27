#pragma once

#include "utils/config/ConfigDefines.h"
#include "version.h"
#include <coreinit/dynload.h>
#include <coreinit/thread.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include <wums/defines/relocation_defines.h>

#define MODULE_VERSION      "v0.3.4"
#define MODULE_VERSION_FULL MODULE_VERSION MODULE_VERSION_EXTRA

class PluginDataSharedPtrComparator;
class PluginData;
class PluginContainer;

extern StoredBuffer gStoredTVBuffer;
extern StoredBuffer gStoredDRCBuffer;

#define TRAMP_DATA_SIZE 1024
extern std::vector<relocation_trampoline_entry_t> gTrampData;
extern std::vector<PluginContainer> gLoadedPlugins;

extern std::set<std::shared_ptr<PluginData>, PluginDataSharedPtrComparator> gLoadedData;
extern std::vector<std::shared_ptr<PluginData>> gLoadOnNextLaunch;
extern std::mutex gLoadedDataMutex;
extern std::map<std::string, OSDynLoad_Module> gUsedRPLs;
extern std::vector<void *> gAllocatedAddresses;

extern bool gNotificationModuleLoaded;

extern OSThread *gOnlyAcceptFromThread;

extern bool gConfigMenuShouldClose;