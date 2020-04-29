#pragma once
#include <wups.h>
#include "common/plugin_defines.h"

void CallHook(plugin_information_t * pluginInformation, wups_loader_hook_type_t hook_type);
void CallHookEx(plugin_information_t * pluginInformation, wups_loader_hook_type_t hook_type, int32_t plugin_index_needed);
bool HasHookCallHook(plugin_information_t * pluginInformation, wups_loader_hook_type_t hook_type);
