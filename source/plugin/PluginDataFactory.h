/****************************************************************************
 * Copyright (C) 2020 Maschell
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
#include "PluginData.h"
#include "PluginLoadWrapper.h"

#include <memory>
#include <set>
#include <vector>

class PluginDataFactory {
public:
    static std::vector<PluginLoadWrapper> loadDir(std::string_view path, const std::set<std::string> &inactivePluginsFilenames);

    static std::unique_ptr<PluginData> load(std::string_view path);

    static std::unique_ptr<PluginData> load(std::vector<uint8_t> &&buffer, std::string_view source);
};
