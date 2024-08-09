/****************************************************************************
 * Copyright (C) 2018 Maschell
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

#include "PluginDataFactory.h"
#include "NotificationsUtils.h"
#include "PluginLoadWrapper.h"
#include "fs/FSUtils.h"
#include "utils/StringTools.h"
#include "utils/logger.h"

#include <memory>
#include <set>
#include <sys/dirent.h>

std::vector<PluginLoadWrapper> PluginDataFactory::loadDir(const std::string_view path, const std::set<std::string> &inactivePluginsFilenames) {
    std::vector<PluginLoadWrapper> result;

    for (const auto &full_file_path : getPluginFilePaths(path)) {
        std::string fileName = StringTools::FullpathToFilename(full_file_path.c_str());

        DEBUG_FUNCTION_LINE("Loading plugin: %s", full_file_path.c_str());

        if (auto pluginData = load(full_file_path)) {
            bool shouldBeLoadedAndLinked = true;

            if (inactivePluginsFilenames.contains(fileName)) {
                shouldBeLoadedAndLinked = false;
            }
            result.emplace_back(std::move(pluginData), shouldBeLoadedAndLinked);
        } else {
            auto errMsg = string_format("Failed to load plugin: %s", full_file_path.c_str());
            DEBUG_FUNCTION_LINE_ERR("%s", errMsg.c_str());
            DisplayErrorNotificationMessage(errMsg, 15.0f);
        }
    }

    return result;
}

std::unique_ptr<PluginData> PluginDataFactory::load(const std::string_view path) {
    std::vector<uint8_t> buffer;
    if (FSUtils::LoadFileToMem(path, buffer) < 0) {
        DEBUG_FUNCTION_LINE_ERR("Failed to load %s into memory", path.data());
        return nullptr;
    }

    DEBUG_FUNCTION_LINE_VERBOSE("Loaded file!");
    return load(std::move(buffer), path);
}

std::unique_ptr<PluginData> PluginDataFactory::load(std::vector<uint8_t> &&buffer, std::string_view source) {
    if (buffer.empty()) {
        return nullptr;
    }

    return make_unique_nothrow<PluginData>(std::move(buffer), source);
}
