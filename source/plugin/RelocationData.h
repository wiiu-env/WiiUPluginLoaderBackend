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

#pragma once

#include <string>
#include "ImportRPLInformation.h"

class RelocationData {

public:
    RelocationData(const char type, size_t offset, int32_t addend, void *destination, std::string name, ImportRPLInformation rplInfo) :
            type(type),
            offset(offset),
            addend(addend),
            destination(destination),
            name(name),
            rplInfo(rplInfo) {
    }

    RelocationData(const RelocationData &o2) :
            type(o2.type),
            offset(o2.offset),
            addend(o2.addend),
            destination(o2.destination),
            name(o2.name),
            rplInfo(o2.rplInfo) {
    }

    virtual ~RelocationData() {
    }

    const char getType() const {
        return type;
    }

    const size_t getOffset() const {
        return offset;
    }

    const int32_t getAddend() const {
        return addend;
    }

    const void *getDestination() const {
        return destination;
    }

    const std::string &getName() const {
        return name;
    }

    const ImportRPLInformation &getImportRPLInformation() const {
        return rplInfo;
    }

private:
    char type;
    size_t offset;
    int32_t addend;
    void *destination;
    std::string name;
    ImportRPLInformation rplInfo;
};
