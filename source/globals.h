#pragma once
#include "plugin/PluginContainer.h"
#include "utils/ConfigUtils.h"
#include "version.h"
#include <coreinit/dynload.h>
#include <forward_list>
#include <memory>
#include <mutex>
#include <vector>
#include <wums/defines/relocation_defines.h>

#define VERSION      "v0.2.4"
#define VERSION_FULL VERSION VERSION_EXTRA

extern StoredBuffer gStoredTVBuffer;
extern StoredBuffer gStoredDRCBuffer;

#define TRAMP_DATA_SIZE 1024
extern relocation_trampoline_entry_t *gTrampData;
extern std::vector<std::unique_ptr<PluginContainer>> gLoadedPlugins;

extern std::forward_list<std::shared_ptr<PluginData>> gLoadedData;
extern std::forward_list<std::shared_ptr<PluginData>> gLoadOnNextLaunch;
extern std::mutex gLoadedDataMutex;
extern std::map<std::string, OSDynLoad_Module> gUsedRPLs;
extern std::vector<void *> gAllocatedAddresses;
