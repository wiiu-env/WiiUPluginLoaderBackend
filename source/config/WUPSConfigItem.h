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
#include <wups/config.h>

namespace WUPSConfigAPIBackend {
    class WUPSConfigItem {
    public:
        explicit WUPSConfigItem(std::string displayName);
        virtual ~WUPSConfigItem();

        [[nodiscard]] virtual std::string getCurrentValueDisplay() const = 0;

        [[nodiscard]] virtual std::string getCurrentValueSelectedDisplay() const = 0;

        virtual void onSelected(bool isSelected) const = 0;

        virtual void onButtonPressed(WUPSConfigButtons) const;

        [[nodiscard]] virtual bool isMovementAllowed() const = 0;

        virtual void restoreDefault() const = 0;

        virtual void onCloseCallback() = 0;

        [[nodiscard]] const std::string &getDisplayName() const;

        virtual void setConfigId(const std::string &);

        virtual const std::string &getConfigId();

        void setDisplayName(std::string displayName);

        virtual void onInput(WUPSConfigSimplePadData) const;

        virtual void onInputEx(WUPSConfigComplexPadData) const;

    protected:
        std::string mDisplayName;
        std::string mStubConfigId;
    };
} // namespace WUPSConfigAPIBackend
