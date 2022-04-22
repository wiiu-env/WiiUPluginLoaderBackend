/****************************************************************************
 * Copyright (C) 2021 Maschell
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

class FunctionSymbolData {

public:
    FunctionSymbolData(const FunctionSymbolData &o2) = default;

    FunctionSymbolData(std::string &name, void *address, uint32_t size) : name(name),
                                                                          address(address),
                                                                          size(size) {
    }

    bool operator<(const FunctionSymbolData &rhs) const {
        return (uint32_t) address < (uint32_t) rhs.address;
    }

    virtual ~FunctionSymbolData() = default;

    [[nodiscard]] const std::string &getName() const {
        return name;
    }

    [[nodiscard]] void *getAddress() const {
        return address;
    }

    [[nodiscard]] uint32_t getSize() const {
        return size;
    }

private:
    std::string name;
    void *address;
    uint32_t size;
};