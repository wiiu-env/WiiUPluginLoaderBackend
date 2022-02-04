/****************************************************************************
 * Copyright (C) 2019 Maschell
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

#include "../elfio/elfio.hpp"
#include "PluginContainer.h"
#include "PluginInformation.h"
#include <coreinit/memheap.h>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <wums/defines/relocation_defines.h>

class PluginInformationFactory {
public:
    static std::optional<std::shared_ptr<PluginInformation>>
    load(const std::shared_ptr<PluginData> &pluginData, MEMHeapHandle heaphandle, relocation_trampoline_entry_t *trampoline_data, uint32_t trampoline_data_length, uint8_t trampolineId);

    static bool
    linkSection(const elfio &reader, uint32_t section_index, uint32_t destination, uint32_t base_text, uint32_t base_data, relocation_trampoline_entry_t *trampoline_data,
                uint32_t trampoline_data_length,
                uint8_t trampolineId);

    static std::vector<std::shared_ptr<RelocationData>> getImportRelocationData(const elfio &reader, uint8_t **destinations);
};
