/****************************************************************************
 * Copyright (C) 2018-2021 Maschell
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

#include "WUPSConfigItem.h"
#include <string>
#include <vector>
#include <wups/config.h>

class WUPSConfigCategory {
public:
    explicit WUPSConfigCategory(const std::string &name) {
        this->name = name;
    }

    ~WUPSConfigCategory() {
        for (auto &element : items) {
            delete element;
        }
    }

    /**
        \return Returns the name of this WUPSConfigCategory
    **/
    [[nodiscard]] const std::string &getName() const {
        return this->name;
    }

    /**
        \brief  Adds a given WUPSConfigItem to this WUPSConfigCategory.
                The item will be added to the end of the list.
                This class holds responsibility for deleting the created instance.

        \param item: The item that will be added to this config.

        \return On success, true will be returned.
                On error false will be returned. In this case the caller still has the responsibility
                for deleting the WUPSConfigItem instance.
    **/
    [[nodiscard]] bool addItem(WUPSConfigItem *item) {
        if (item != nullptr) {
            items.push_back(item);
            return true;
        }
        return false;
    }

    /**
        \return Returns a vector with all items.
    **/
    [[nodiscard]] const std::vector<WUPSConfigItem *> &getItems() const {
        return this->items;
    }

private:
    std::string name;
    std::vector<WUPSConfigItem *> items{};
};
