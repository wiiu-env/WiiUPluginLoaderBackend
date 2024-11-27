#pragma once

#include "plugin/PluginContainer.h"
#include <vector>
#include <wups/hooks.h>

void CallHook(const std::vector<PluginContainer> &plugins, wups_loader_hook_type_t hook_type);

void CallHook(const PluginContainer &plugin, wups_loader_hook_type_t hook_type);