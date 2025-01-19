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

#include <wums/defines/relocation_defines.h>

#include <elfio/elfio.hpp>

#include <optional>
#include <span>

class PluginData;
class PluginLinkInformation;

class PluginLinkInformationFactory {
public:
    static std::optional<PluginLinkInformation>
    load(const PluginData &pluginData);

private:
    static bool
    linkSection(const ELFIO::elfio &reader, uint32_t section_index, uint32_t destination, uint32_t base_text, uint32_t base_data,
                std::span<relocation_trampoline_entry_t> trampolineData);

    static bool
    addImportRelocationData(PluginLinkInformation &pluginInfo, const ELFIO::elfio &reader, std::span<uint8_t *> destinations);
};
