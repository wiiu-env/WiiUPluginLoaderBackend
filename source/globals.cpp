#include "globals.h"

#include "plugin/FunctionData.h"
#include "plugin/HookData.h"
#include "plugin/PluginConfigData.h"
#include "plugin/PluginContainer.h"
#include "plugin/PluginData.h"
#include "plugin/PluginLinkInformation.h"
#include "plugin/PluginLoadWrapper.h"
#include "plugin/RelocationData.h"
#include "plugin/SectionInfo.h"

StoredBuffer gStoredTVBuffer  = {};
StoredBuffer gStoredDRCBuffer = {};

std::vector<PluginContainer> gLoadedPlugins;
std::vector<PluginContainer> gPluginsToBeDeInitialized;

std::set<std::shared_ptr<PluginData>, PluginDataSharedPtrComparator> gLoadedData;
std::vector<PluginLoadWrapper> gLoadOnNextLaunch;
std::recursive_mutex gLoadedDataMutex;
std::map<std::string, OSDynLoad_Module> gUsedRPLs;
std::vector<void *> gAllocatedAddresses;

bool gNotificationModuleLoaded = false;

OSThread *gOnlyAcceptFromThread = nullptr;

bool gConfigMenuShouldClose = false;
bool gConfigMenuOpened      = false;
