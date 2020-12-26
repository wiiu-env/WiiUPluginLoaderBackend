/****************************************************************************
 * Copyright (C) 2020 Maschell
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

#include "PluginData.h"
#include "PluginMetaInformation.h"
#include "PluginInformation.h"

class PluginContainer {
public:
    PluginContainer(const PluginContainer &other) {
        this->pluginData = other.pluginData;
        this->pluginInformation = other.pluginInformation;
        this->metaInformation = other.metaInformation;
    }

    PluginContainer() = default;

    [[nodiscard]] const PluginMetaInformation &getMetaInformation() const {
        return this->metaInformation;
    }

    void setMetaInformation(const PluginMetaInformation &metaInfo) {
        this->metaInformation = metaInfo;
    }

    [[nodiscard]] const PluginInformation &getPluginInformation() const {
        return pluginInformation;
    }

    void setPluginInformation(const PluginInformation &_pluginInformation) {
        this->pluginInformation = _pluginInformation;
    }

    [[nodiscard]] const PluginData &getPluginData() const {
        return pluginData;
    }

    void setPluginData(const PluginData &_pluginData) {
        this->pluginData = _pluginData;
    }

    PluginData pluginData;
    PluginMetaInformation metaInformation;
    PluginInformation pluginInformation;
};
