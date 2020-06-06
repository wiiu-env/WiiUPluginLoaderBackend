/****************************************************************************
 * Copyright (C) 2016-2018 Maschell
 * With code from chadderz and dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#ifndef _FUNCTION_PATCHER_HOOKS_H_
#define _FUNCTION_PATCHER_HOOKS_H_

#include <coreinit/dynload.h>
#include <common/plugin_defines.h>
#include <common/replacement_defines.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <wups.h>

#define STATIC_FUNCTION         0
#define DYNAMIC_FUNCTION        1

struct rpl_handling {
    wups_loader_library_type_t library;
    const char rplname[15];
    OSDynLoad_Module handle;
};


void new_PatchInvidualMethodHooks(plugin_info_t *plugin_data);
void new_RestoreInvidualInstructions(plugin_info_t *plugin_data);
uint32_t new_GetAddressOfFunction(const char *functionName, wups_loader_library_type_t library);
int32_t new_isDynamicFunction(uint32_t physicalAddress);
void new_resetLibs();

#ifdef __cplusplus
}
#endif

#endif /* _FUNCTION_PATCHER_HOOKS_H_ */
