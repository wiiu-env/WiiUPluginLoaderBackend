#pragma once
#include <vector>
#include <wups/reent_internal.h>

class PluginContainer;

void *wups_backend_get_context(const void *pluginId, wups_loader_init_reent_errors_t_ *outError);

void *wups_backend_set_sentinel();

void wups_backend_restore_head(void *oldHead);

bool wups_backend_register_context(const void *pluginId, void *reentPtr, void (*cleanupFn)(void *), void *oldHead);

void ClearReentDataForPlugins(const std::vector<PluginContainer> &plugins);

void ClearDanglingReentPtr();
