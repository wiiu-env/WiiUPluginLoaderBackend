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
#include "fs/FSUtils.h"
#include "utils/StringTools.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include <dirent.h>
#include <forward_list>
#include <memory>

std::set<std::shared_ptr<PluginData>> PluginDataFactory::loadDir(std::string_view path) {
    std::set<std::shared_ptr<PluginData>> result;
    struct dirent *dp;
    DIR *dfd;

    if (path.empty()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to load Plugins from dir: Path was empty");
        return result;
    }

    if ((dfd = opendir(path.data())) == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("Couldn't open dir %s", path.data());
        return result;
    }

    while ((dp = readdir(dfd)) != nullptr) {
        if (dp->d_type == DT_DIR) {
            continue;
        }
        if (std::string_view(dp->d_name).starts_with('.') || std::string_view(dp->d_name).starts_with('_') || !std::string_view(dp->d_name).ends_with(".wps")) {
            DEBUG_FUNCTION_LINE_WARN("Skip file %s/%s", path.data(), dp->d_name);
            continue;
        }

        auto full_file_path = string_format("%s/%s", path.data(), dp->d_name);
        DEBUG_FUNCTION_LINE("Loading plugin: %s", full_file_path.c_str());
        auto pluginData = load(full_file_path);
        if (pluginData) {
            result.insert(std::move(pluginData));
        } else {
            auto errMsg = string_format("Failed to load plugin: %s", full_file_path.c_str());
            DEBUG_FUNCTION_LINE_ERR("%s", errMsg.c_str());
            DisplayErrorNotificationMessage(errMsg, 15.0f);
        }
    }

    closedir(dfd);

    return result;
}

std::unique_ptr<PluginData> PluginDataFactory::load(std::string_view filename) {
    std::vector<uint8_t> buffer;
    if (FSUtils::LoadFileToMem(filename, buffer) < 0) {
        DEBUG_FUNCTION_LINE_ERR("Failed to load %s into memory", filename.data());
        return nullptr;
    }

    DEBUG_FUNCTION_LINE_VERBOSE("Loaded file!");
    return load(std::move(buffer), filename);
}

std::unique_ptr<PluginData> PluginDataFactory::load(std::vector<uint8_t> &&buffer, std::string_view source) {
    if (buffer.empty()) {
        return nullptr;
    }

    return make_unique_nothrow<PluginData>(std::move(buffer), source);
}
