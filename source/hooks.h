#pragma once

#include <wups/hooks.h>

#include <functional>
#include <vector>

class PluginContainer;

void CallHook(const std::vector<PluginContainer> &plugins, wups_loader_hook_type_t hook_type, const std::function<bool(const PluginContainer &)> &pred);

void CallHook(const std::vector<PluginContainer> &plugins, wups_loader_hook_type_t hook_type);

void CallHook(const PluginContainer &plugin, wups_loader_hook_type_t hook_type);