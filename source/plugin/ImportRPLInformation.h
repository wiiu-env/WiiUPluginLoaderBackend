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
    explicit ImportRPLInformation(std::string name) {
        this->name = std::move(name);
    }

    ~ImportRPLInformation() = default;

    [[nodiscard]] const std::string &getName() const {
        return name;
    }

    [[nodiscard]] std::string getRPLName() const {
        return name.substr(strlen(".dimport_"));
    }

    [[nodiscard]] bool isData() const {
        return name.starts_with(".dimport_");
    }

private:
    std::string name;
};
