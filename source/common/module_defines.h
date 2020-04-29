/****************************************************************************
 * Copyright (C) 2018-2019 Maschell
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

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "dynamic_linking_defines.h"
#include "relocation_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAXIMUM_MODULE_PATH_NAME_LENGTH                     256
#define MAXIMUM_MODULE_NAME_LENGTH                          51

#define DYN_LINK_RELOCATION_LIST_LENGTH                     500

struct module_information_single_t {
    char                            path[MAXIMUM_MODULE_PATH_NAME_LENGTH] = "";     // Path where the module is stored
    dyn_linking_relocation_entry_t  linking_entries[DYN_LINK_RELOCATION_LIST_LENGTH];
    int32_t                         priority;                                       // Priority of this module
    uint32_t                        bssAddr;
    uint32_t                        bssSize;
    uint32_t                        sbssAddr;
    uint32_t                        sbssSize;
    uint32_t                        entrypoint;
    uint32_t                        startAddress;
    uint32_t                        endAddress;
};

#define MAXIMUM_MODULES                                     8

struct module_information_t {
    int32_t                         number_used_modules = 0;                        // Number of used function. Maximum is MAXIMUM_MODULES
    dyn_linking_relocation_data_t   linking_data;
    relocation_trampolin_entry_t    trampolines[DYN_LINK_TRAMPOLIN_LIST_LENGTH];
    module_information_single_t     module_data[MAXIMUM_MODULES];
};

#ifdef __cplusplus
}
#endif
