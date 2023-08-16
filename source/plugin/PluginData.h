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

#include <coreinit/memexpheap.h>
#include <malloc.h>
#include <memory>
#include <optional>
#include <vector>

class PluginData {
public:
    explicit PluginData(const std::vector<uint8_t> &buffer, std::string source);

    uint32_t getHandle() {
        return (uint32_t) this;
    }

    size_t length = 0;
    std::unique_ptr<uint8_t[]> buffer;
    std::string mSource;
};
