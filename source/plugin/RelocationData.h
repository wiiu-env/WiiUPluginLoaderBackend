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

#include "ImportRPLInformation.h"
#include <cstdint>
#include <memory>
#include <string>

class RelocationData {

public:
    RelocationData(char type, size_t offset, int32_t addend, void *destination, std::string name, std::shared_ptr<ImportRPLInformation> rplInfo);

    RelocationData(const RelocationData &o2);

    virtual ~RelocationData();

    [[nodiscard]] char getType() const;

    [[nodiscard]] size_t getOffset() const;

    [[nodiscard]] int32_t getAddend() const;

    [[nodiscard]] const void *getDestination() const;

    [[nodiscard]] const std::string &getName() const;

    [[nodiscard]] const ImportRPLInformation &getImportRPLInformation() const;

private:
    char mType;
    size_t mOffset;
    int32_t mAddend;
    void *mDestination;
    std::string mName;
    std::shared_ptr<ImportRPLInformation> mRPLInfo;
};
