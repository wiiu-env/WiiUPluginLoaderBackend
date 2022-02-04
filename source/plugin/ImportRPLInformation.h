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

#include "../utils/logger.h"
#include <string>
#include <utility>

class ImportRPLInformation {

public:
    explicit ImportRPLInformation(std::string name, bool isData = false) {
        this->name    = std::move(name);
        this->_isData = isData;
    }

    ~ImportRPLInformation() = default;

    [[nodiscard]] std::string getName() const {
        return name;
    }

    [[nodiscard]] bool isData() const {
        return _isData;
    }

private:
    std::string name;
    bool _isData = false;
};
