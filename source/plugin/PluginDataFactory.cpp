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
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "PluginDataFactory.h"
#include "utils/logger.h"
#include "utils/StringTools.h"


std::vector<PluginData> PluginDataFactory::loadDir(const std::string &path, MEMHeapHandle heapHandle) {
    std::vector<PluginData> result;
    struct dirent *dp;
    DIR *dfd = NULL;

    if (path.empty()) {
        DEBUG_FUNCTION_LINE("Path was empty\n");
        return result;
    }

    if ((dfd = opendir(path.c_str())) == NULL) {
        DEBUG_FUNCTION_LINE("Couldn't open dir %s\n", path.c_str());
        return result;
    }

    while ((dp = readdir(dfd)) != NULL) {
        struct stat stbuf;
        std::string full_file_path = StringTools::strfmt("%s/%s", path.c_str(), dp->d_name);
        StringTools::RemoveDoubleSlashs(full_file_path);
        if (stat(full_file_path.c_str(), &stbuf) == -1) {
            DEBUG_FUNCTION_LINE("Unable to stat file: %s\n", full_file_path.c_str());
            continue;
        }

        if ((stbuf.st_mode & S_IFMT) == S_IFDIR) { // Skip directories
            continue;
        } else {
            DEBUG_FUNCTION_LINE("Found file: %s\n", full_file_path.c_str());
            auto pluginData = load(full_file_path, heapHandle);
            if (pluginData) {
                result.push_back(pluginData.value());
            }
        }
    }
    if (dfd != NULL) {
        closedir(dfd);
    }

    return result;
}

std::optional<PluginData> PluginDataFactory::load(const std::string &filename, MEMHeapHandle heapHandle) {
    // open the file:
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        DEBUG_FUNCTION_LINE("Failed to open %s", filename.c_str());
        return std::nullopt;
    }
    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);
    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> vBuffer;
    vBuffer.reserve(fileSize);

    // read the data:
    vBuffer.insert(vBuffer.begin(),
                   std::istream_iterator<uint8_t>(file),
                   std::istream_iterator<uint8_t>());

    DEBUG_FUNCTION_LINE("Loaded file");

    return load(vBuffer, heapHandle);
}

std::optional<PluginData> PluginDataFactory::load(std::vector<uint8_t> &buffer, MEMHeapHandle heapHandle) {
    if (buffer.empty()) {
        return std::nullopt;
    }

    PluginData pluginData(buffer, heapHandle, eMemoryTypes::eMemTypeExpHeap);
    return pluginData;
}
