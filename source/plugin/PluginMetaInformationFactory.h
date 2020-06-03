/****************************************************************************
 * Copyright (C) 2019,2020 Maschell
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

#include <optional>
#include <string>
#include <vector>
#include "PluginMetaInformation.h"
#include "PluginData.h"

class PluginMetaInformationFactory {
public:
    static std::optional<PluginMetaInformation> loadPlugin(const PluginData &pluginData);

    static std::optional<PluginMetaInformation> loadPlugin(const std::string filePath);

    static std::optional<PluginMetaInformation> loadPlugin(char *buffer, size_t size);

    static std::optional<PluginMetaInformation> loadPlugin(const elfio& reader);
};
