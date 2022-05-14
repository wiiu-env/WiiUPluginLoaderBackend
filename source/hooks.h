#pragma once

#include "plugin/PluginContainer.h"
#include <memory>
#include <vector>
#include <wups/hooks.h>

void CallHook(const std::vector<std::unique_ptr<PluginContainer>> &plugins, wups_loader_hook_type_t hook_type);

void CallHook(const std::unique_ptr<PluginContainer> &plugin, wups_loader_hook_type_t hook_type);