#pragma once
#include <vector>
#include <wups/reent_internal.h>

class PluginContainer;

bool wups_backend_get_context(const void *pluginId, void **outPtr);

bool wups_backend_register_context(const void *pluginId, void *reentPtr, void (*cleanupFn)(void *));

void ClearReentDataForPlugins(const std::vector<PluginContainer> &plugins);

void MarkReentNodesForDeletion();

void ClearDanglingReentPtr();
