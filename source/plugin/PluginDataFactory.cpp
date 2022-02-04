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
#include "../utils/StringTools.h"
#include "../utils/logger.h"
#include <dirent.h>
#include <fcntl.h>
#include <memory>
#include <sys/stat.h>


std::vector<std::shared_ptr<PluginData>> PluginDataFactory::loadDir(const std::string &path, MEMHeapHandle heapHandle) {
    std::vector<std::shared_ptr<PluginData>> result;
    struct dirent *dp;
    DIR *dfd;

    if (path.empty()) {
        DEBUG_FUNCTION_LINE("Path was empty");
        return result;
    }

    if ((dfd = opendir(path.c_str())) == nullptr) {
        DEBUG_FUNCTION_LINE("Couldn't open dir %s", path.c_str());
        return result;
    }

    while ((dp = readdir(dfd)) != nullptr) {
        struct stat stbuf {};
        std::string full_file_path = StringTools::strfmt("%s/%s", path.c_str(), dp->d_name);
        StringTools::RemoveDoubleSlashs(full_file_path);
        if (stat(full_file_path.c_str(), &stbuf) == -1) {
            DEBUG_FUNCTION_LINE("Unable to stat file: %s", full_file_path.c_str());
            continue;
        }

        if ((stbuf.st_mode & S_IFMT) == S_IFDIR) { // Skip directories
            continue;
        } else {
            DEBUG_FUNCTION_LINE("Loading plugin: %s", full_file_path.c_str());
            auto pluginData = load(full_file_path, heapHandle);
            if (pluginData) {
                result.push_back(pluginData.value());
            }
        }
    }
    if (dfd != nullptr) {
        closedir(dfd);
    }

    return result;
}

std::optional<std::shared_ptr<PluginData>> PluginDataFactory::load(const std::string &filename, MEMHeapHandle heapHandle) {
    // Not going to explicitly check these.
    // The use of gcount() below will compensate for a failure here.
    std::ifstream is(filename, std::ios::binary);

    is.seekg(0, std::ios::end);
    std::streampos length = is.tellg();
    is.seekg(0, std::ios::beg);

    // reading into a 0x40 aligned buffer increases reading speed.
    char *data = (char *) memalign(0x40, length);
    if (!data) {
        is.close();
        DEBUG_FUNCTION_LINE("Failed to alloc memory for holding the plugin");
        return {};
    }
    is.read(data, length);

    std::vector<uint8_t> result;
    result.resize(length);
    memcpy(&result[0], data, length);
    free(data);
    is.close();

    DEBUG_FUNCTION_LINE_VERBOSE("Loaded file!");

    return load(result, heapHandle);
}

std::optional<std::shared_ptr<PluginData>> PluginDataFactory::load(std::vector<uint8_t> &buffer, MEMHeapHandle heapHandle) {
    if (buffer.empty()) {
        return std::nullopt;
    }

    return std::shared_ptr<PluginData>(new PluginData(buffer, heapHandle, eMemoryTypes::eMemTypeMEM2));
}
