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
#include <span>
#include <utility>
#include <vector>

class PluginData {
public:
    explicit PluginData(std::vector<uint8_t> &&buffer, std::string_view source) : mBuffer(std::move(buffer)), mSource(source) {
    }

    explicit PluginData(std::span<uint8_t> buffer, std::string_view source) : mBuffer(buffer.begin(), buffer.end()), mSource(source) {
    }

    [[nodiscard]] uint32_t getHandle() const;

    [[nodiscard]] std::span<uint8_t const> getBuffer() const;

    [[nodiscard]] const std::string &getSource() const;

private:
    std::vector<uint8_t> mBuffer;
    std::string mSource;
};
