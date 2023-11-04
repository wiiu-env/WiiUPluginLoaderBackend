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
#include <vector>

#include "utils/StringTools.h"
#include "utils/logger.h"
#include <wups/config.h>

class WUPSConfigItem {
public:
    /**
        Sets the display name of this WUPSConfigItem
        This is the value which will be shown in the configuration menu.
    **/
    virtual void setDisplayName(std::string_view _displayName) {
        this->displayName = _displayName;
    }

    /**
        \return Returns the display name of this WUPSConfigItem
    **/
    virtual const std::string &getDisplayName() {
        return this->displayName;
    }

    /**
        Sets the config ID name of this WUPSConfigItem.
        This config ID is used to persist the configuration values and needs
        to be unique in the context of this WUPSConfig.
        Items in different categories are NOT allowed to have the config ID.
    **/
    virtual void setConfigID(std::string_view _configID) {
        this->configID = _configID;
    }

    /**
        \return Returns the configID of this WUPSConfigItem.
    **/
    [[nodiscard]] virtual const std::string &getConfigID() const {
        return this->configID;
    }

    /**
        Returns a string that displays the current value.
        This string is shown next to the display name when the cursor is NOT on this item
    **/
    [[nodiscard]] std::string getCurrentValueDisplay() const {
        if (this->callbacks.getCurrentValueDisplay != nullptr) {
            char buf[80];
            int res = this->callbacks.getCurrentValueDisplay(context, buf, sizeof(buf));
            if (res == 0) {
                return buf;
            } else {
                return string_format("[ERROR %d]", res);
            }
        }
        DEBUG_FUNCTION_LINE_ERR("NOT IMPLEMENTED");
        return "NOT_IMPLEMENTED";
    }

    /**
        Returns a string that displays the current value when selected.
        This string is shown next to the display name when the cursor IS on this item
    **/
    [[nodiscard]] std::string getCurrentValueSelectedDisplay() const {
        if (this->callbacks.getCurrentValueSelectedDisplay != nullptr) {
            char buf[80];
            int res = this->callbacks.getCurrentValueSelectedDisplay(context, buf, sizeof(buf));
            if (res == 0) {
                return buf;
            } else {
                return string_format("[ERROR %d]", res);
            }
        }
        DEBUG_FUNCTION_LINE_ERR("NOT IMPLEMENTED");
        return "NOT_IMPLEMENTED";
    }

    /**
        Is called when the cursor enters or leaves the item.
        When the cursor enters the item, "isSelected" will be true.
        When the cursor leaves the item, "isSelected" will be false.
    **/
    void onSelected(bool isSelected) const {
        if (this->callbacks.onSelected != nullptr) {
            this->callbacks.onSelected(context, isSelected);
            return;
        }
        DEBUG_FUNCTION_LINE_ERR("NOT IMPLEMENTED");
    }

    /**
        Is called when a button is pressed while the cursor on this item.
        See the WUPSConfigButtons enum for possible values.
    **/
    void onButtonPressed(WUPSConfigButtons buttons) const {
        if (this->callbacks.onButtonPressed != nullptr) {
            this->callbacks.onButtonPressed(context, buttons);
            return;
        }
        DEBUG_FUNCTION_LINE_ERR("NOT IMPLEMENTED");
    }

    /**
        When the cursor is on this item, the configuration menu asks this item
        if it's allowed to leave it.
        If it returns true, the item can be leaved.
        It it returns false, leaves is not allowed.
    **/
    [[nodiscard]] bool isMovementAllowed() const {
        if (this->callbacks.isMovementAllowed != nullptr) {
            return this->callbacks.isMovementAllowed(context);
        }
        DEBUG_FUNCTION_LINE_ERR("NOT IMPLEMENTED");
        return false;
    }

    /**
        Restores the default value
    **/
    void restoreDefault() {
        if (this->callbacks.restoreDefault != nullptr) {
            this->callbacks.restoreDefault(context);
            return;
        }
        DEBUG_FUNCTION_LINE_ERR("NOT IMPLEMENTED");
    }

    /**
        Call callback with with current value.
        This function will be called whenever this item should call it's (optional) given
        callback with the current value.
        Returns true if a valid callback could be called
        Returns false if no callback was called (e.g. callback was nullptr)
    **/
    bool callCallback() {
        if (this->callbacks.callCallback != nullptr) {
            return this->callbacks.callCallback(context);
        }
        return false;
    }

    bool isDirty() {
        return defaultValue != getCurrentValueDisplay();
    }

    WUPSConfigItem(std::string_view _configID, std::string_view _displayName, WUPSConfigCallbacks_t callbacks, void *_context) {
        this->configID     = _configID;
        this->displayName  = _displayName;
        this->context      = _context;
        this->callbacks    = callbacks;
        this->defaultValue = getCurrentValueDisplay();
    }

    virtual ~WUPSConfigItem() {
        if (this->callbacks.onDelete != nullptr) {
            this->callbacks.onDelete(context);
            return;
        }
        DEBUG_FUNCTION_LINE_ERR("NOT IMPLEMENTED");
    };

private:
    void *context;
    std::string displayName;
    std::string configID;
    std::string defaultValue;
    WUPSConfigCallbacks_t callbacks{};
};
