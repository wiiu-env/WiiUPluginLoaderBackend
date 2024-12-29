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

#include <memory>
#include <string>
#include <vector>

namespace WUPSConfigAPIBackend {
    class WUPSConfigItem;
    class WUPSConfigCategory {
    public:
        explicit WUPSConfigCategory(std::string_view name);

        virtual ~WUPSConfigCategory();

        /**
           \return Returns the name of this WUPSConfigCategory
        **/
        [[nodiscard]] const std::string &getName() const;

        /**
       \brief  Adds a given WUPSConfigItem to this WUPSConfigCategory.
               The item will be added to the end of the list.
               This class holds responsibility for deleting the created instance.

       \param item: The item that will be added to this config.

       \return On success, true will be returned.
               On error false will be returned. In this case the caller still has the responsibility
               for deleting the WUPSConfigItem instance.
        **/
        [[nodiscard]] bool addItem(std::unique_ptr<WUPSConfigItem> &item);

        /**
        \return Returns a vector with all items.
        **/
        [[nodiscard]] const std::vector<std::unique_ptr<WUPSConfigItem>> &getItems() const;

        /**
        \return Returns a vector with all categories.
        **/
        [[nodiscard]] const std::vector<std::unique_ptr<WUPSConfigCategory>> &getCategories() const;

        /**
          \brief  Adds a given WUPSConfigCategory to this WUPSConfigCategory.
                  The category will be added to the end of the list.
                  This class holds responsibility for deleting the created instance.

          \param category: The category that will be added to this config.

          \return On success, true will be returned.
                  On error false will be returned. In this case the caller still has the responsibility
                  for deleting the WUPSConfigCategory instance.
        **/
        [[nodiscard]] bool addCategory(std::unique_ptr<WUPSConfigCategory> &category);

    private:
        std::string mName;
        std::vector<std::unique_ptr<WUPSConfigItem>> mItems;
        std::vector<std::unique_ptr<WUPSConfigCategory>> mCategories;
    };
} // namespace WUPSConfigAPIBackend