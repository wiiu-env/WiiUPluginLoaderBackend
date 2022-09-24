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
#include "fs/FSUtils.h"
#include "utils/StringTools.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include <dirent.h>
#include <forward_list>
#include <memory>
#include <sys/stat.h>

std::forward_list<std::shared_ptr<PluginData>> PluginDataFactory::loadDir(const std::string &path) {
    std::forward_list<std::shared_ptr<PluginData>> result;
    struct dirent *dp;
    DIR *dfd;

    if (path.empty()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to load Plugins from dir: Path was empty");
        return result;
    }

    if ((dfd = opendir(path.c_str())) == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("Couldn't open dir %s", path.c_str());
        return result;
    }

    while ((dp = readdir(dfd)) != nullptr) {
        if (dp->d_type == DT_DIR) {
            continue;
        }
        if (std::string_view(dp->d_name).starts_with('.') || std::string_view(dp->d_name).starts_with('_') || !std::string_view(dp->d_name).ends_with(".wps")) {
            DEBUG_FUNCTION_LINE_WARN("Skip file %s/%s", path.c_str(), dp->d_name);
            continue;
        }

        auto full_file_path = string_format("%s/%s", path.c_str(), dp->d_name);
        DEBUG_FUNCTION_LINE("Loading plugin: %s", full_file_path.c_str());
        auto pluginData = load(full_file_path);
        if (pluginData) {
            result.push_front(std::move(pluginData.value()));
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to load plugin: %s", full_file_path.c_str());
        }
    }

    closedir(dfd);

    return result;
}

std::optional<std::unique_ptr<PluginData>> PluginDataFactory::load(const std::string &filename) {
    uint8_t *buffer = nullptr;
    uint32_t fsize  = 0;
    if (FSUtils::LoadFileToMem(filename.c_str(), &buffer, &fsize) < 0) {
        DEBUG_FUNCTION_LINE_ERR("Failed to load %s into memory", filename.c_str());
        return {};
    }

    std::vector<uint8_t> result;
    result.resize(fsize);
    memcpy(&result[0], buffer, fsize);
    free(buffer);

    DEBUG_FUNCTION_LINE_VERBOSE("Loaded file!");

    return load(result);
}

std::optional<std::unique_ptr<PluginData>> PluginDataFactory::load(const std::vector<uint8_t> &buffer) {
    if (buffer.empty()) {
        return {};
    }

    auto res = make_unique_nothrow<PluginData>(buffer);
    if (!res) {
        return {};
    }

    return res;
}
