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
#include "PluginInformation.h"
#include "PluginMetaInformation.h"
#include <memory>
#include <utility>

class PluginContainer {
public:
    PluginContainer(std::unique_ptr<PluginMetaInformation> metaInformation, std::unique_ptr<PluginInformation> pluginInformation, std::shared_ptr<PluginData> pluginData)
        : metaInformation(std::move(metaInformation)),
          pluginInformation(std::move(pluginInformation)),
          pluginData(std::move(pluginData)) {
    }

    [[nodiscard]] const std::unique_ptr<PluginMetaInformation> &getMetaInformation() const {
        return this->metaInformation;
    }

    [[nodiscard]] const std::unique_ptr<PluginInformation> &getPluginInformation() const {
        return pluginInformation;
    }

    [[nodiscard]] const std::shared_ptr<PluginData> &getPluginData() const {
        return pluginData;
    }

    uint32_t getHandle() {
        return (uint32_t) this;
    }

    const std::unique_ptr<PluginMetaInformation> metaInformation;
    const std::unique_ptr<PluginInformation> pluginInformation;
    const std::shared_ptr<PluginData> pluginData;
};
