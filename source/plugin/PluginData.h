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

#include <optional>
#include <vector>
#include <malloc.h>
#include <coreinit/memexpheap.h>

#include "elfio/elfio.hpp"

using namespace ELFIO;

enum eMemoryTypes {
    eMemTypeMEM2,
    eMemTypeExpHeap
};

class PluginData {
public:
    const std::optional<elfio *> &getReader() const {
        return reader;
    }

    ~PluginData() {
        if (nReader != NULL) {
            delete nReader;
            nReader = NULL;
        }
    }

    void freeMemory();

    void *getBuffer() const {
        return this->buffer;
    }

    PluginData(const PluginData &obj);

    PluginData() {
    }

    void *buffer;
    MEMHeapHandle heapHandle;
    eMemoryTypes memoryType;
    size_t length;

private:
    PluginData(std::vector<uint8_t> buffer);

    PluginData(std::vector<uint8_t> input, MEMHeapHandle heapHandle, eMemoryTypes memoryType);

    void loadReader();

    static std::optional<PluginData> createFromExistingData(const void *buffer, MEMHeapHandle heapHandle, eMemoryTypes memoryType, const size_t length);

    std::optional<elfio *> reader;

    elfio *nReader = NULL;

    friend class PluginDataFactory;

    friend class PluginContainer;

    friend class PluginDataPersistence;
};
