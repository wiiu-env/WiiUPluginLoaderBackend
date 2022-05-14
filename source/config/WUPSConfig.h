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

#include "WUPSConfigCategory.h"
#include "utils/logger.h"
#include <optional>
#include <string>
#include <vector>
#include <wups/config.h>

class WUPSConfig {
public:
    explicit WUPSConfig(const std::string &name) {
        this->name = name;
    }

    ~WUPSConfig() {
        for (auto &element : categories) {
            delete element;
        }
    }

    /**
        \return Returns the name of this WUPSConfig
    **/
    [[nodiscard]] const std::string &getName() const {
        return this->name;
    }

    /**
        \brief  Creates a new WUPSCategory add its to this WUPSConfig.
                The category will be added to the end of the list.
                This class holds responsibility for deleting the created instance.

        \param categoryName: The name of the category that will be created.

        \return On success, the created and inserted category will be returned.
    **/
    std::optional<WUPSConfigCategory *> addCategory(const std::string &categoryName) {
        auto curCat = new (std::nothrow) WUPSConfigCategory(categoryName);
        if (curCat == nullptr) {
            return {};
        }
        categories.push_back(curCat);
        return curCat;
    }

    /**
        \brief  Adds a given WUPSConfigCategory to this WUPSConfig.
                The category will be added to the end of the list.
                This class holds responsibility for deleting the created instance.

        \param category: The category that will be added to this config.

        \return On success, the inserted category will be returned.
                On error nullptr will be returned. In this case the caller still has the responsibility
                for deleting the WUPSConfigCategory instance.
    **/
    WUPSConfigCategory *addCategory(WUPSConfigCategory *category) {
        categories.push_back(category);
        return category;
    }

    /**
        \return Returns a vector with all categories.
    **/
    const std::vector<WUPSConfigCategory *> &getCategories() {
        return this->categories;
    }

private:
    std::string name;
    std::vector<WUPSConfigCategory *> categories = {};
};
