/****************************************************************************
 * Copyright (C) 2019 Maschell
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

class SectionInfo {

public:
    SectionInfo(std::string name, uint32_t address, uint32_t sectionSize) : name(std::move(name)),
                                                                            address(address),
                                                                            sectionSize(sectionSize) {
    }

    [[nodiscard]] const std::string &getName() const {
        return name;
    }

    [[nodiscard]] uint32_t getAddress() const {
        return address;
    }

    [[nodiscard]] uint32_t getSize() const {
        return sectionSize;
    }

    [[nodiscard]] uint32_t isInSection(uint32_t addr) const {
        return addr >= address && addr < address + sectionSize;
    }

private:
    std::string name;
    uint32_t address{};
    uint32_t sectionSize{};
};
