#pragma once
#include "plugin/PluginContainer.h"
#include "utils/config/ConfigUtils.h"
#include "version.h"
#include <coreinit/dynload.h>
#include <forward_list>
#include <memory>
#include <mutex>
#include <vector>
#include <wums/defines/relocation_defines.h>

#define VERSION      "v0.3.1"
#define VERSION_FULL VERSION VERSION_EXTRA

extern StoredBuffer gStoredTVBuffer;
extern StoredBuffer gStoredDRCBuffer;

#define TRAMP_DATA_SIZE 1024
extern std::vector<relocation_trampoline_entry_t> gTrampData;
extern std::vector<PluginContainer> gLoadedPlugins;

extern std::set<std::shared_ptr<PluginData>> gLoadedData;
extern std::set<std::shared_ptr<PluginData>> gLoadOnNextLaunch;
extern std::mutex gLoadedDataMutex;
extern std::map<std::string, OSDynLoad_Module> gUsedRPLs;
extern std::vector<void *> gAllocatedAddresses;

extern bool gNotificationModuleLoaded;

extern OSThread *gOnlyAcceptFromThread;

extern bool gConfigMenuShouldClose;