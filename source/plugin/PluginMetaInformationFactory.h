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

#include "PluginData.h"
#include "PluginMetaInformation.h"
#include "elfio/elfio.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

enum PluginParseErrors {
    PLUGIN_PARSE_ERROR_NONE,
    PLUGIN_PARSE_ERROR_UNKNOWN,
    PLUGIN_PARSE_ERROR_INCOMPATIBLE_VERSION,
    PLUGIN_PARSE_ERROR_BUFFER_EMPTY,
    PLUGIN_PARSE_ERROR_ELFIO_PARSE_FAILED,
    PLUGIN_PARSE_ERROR_IO_ERROR,
};

class PluginMetaInformationFactory {
public:
    static std::optional<std::unique_ptr<PluginMetaInformation>> loadPlugin(const std::shared_ptr<PluginData> &pluginData, PluginParseErrors &error);

    static std::optional<std::unique_ptr<PluginMetaInformation>> loadPlugin(const std::string &filePath, PluginParseErrors &error);

    static std::optional<std::unique_ptr<PluginMetaInformation>> loadPlugin(char *buffer, size_t size, PluginParseErrors &error);

    static std::optional<std::unique_ptr<PluginMetaInformation>> loadPlugin(const ELFIO::elfio &reader, PluginParseErrors &error);
};
