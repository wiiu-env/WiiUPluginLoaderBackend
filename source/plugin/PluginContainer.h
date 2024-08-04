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

#include "PluginConfigData.h"
#include "PluginData.h"
#include "PluginLinkInformation.h"
#include "PluginMetaInformation.h"

#include <memory>
#include <optional>
#include <wups/storage.h>

class PluginContainer {
public:
    PluginContainer(PluginMetaInformation metaInformation, PluginLinkInformation pluginLinkInformation, std::shared_ptr<PluginData> pluginData);

    PluginContainer(const PluginContainer &) = delete;

    PluginContainer(PluginContainer &&src);

    PluginContainer &operator=(PluginContainer &&src);

    [[nodiscard]] const PluginMetaInformation &getMetaInformation() const;

    [[nodiscard]] const PluginLinkInformation &getPluginLinkInformation() const;

    [[nodiscard]] PluginLinkInformation &getPluginLinkInformation();

    [[nodiscard]] std::shared_ptr<PluginData> getPluginDataCopy() const;

    [[nodiscard]] uint32_t getHandle() const;

    [[nodiscard]] const std::optional<PluginConfigData> &getConfigData() const;

    [[nodiscard]] bool isLinkedAndLoaded() const;

    void setConfigData(const PluginConfigData &pluginConfigData);

    WUPSStorageError OpenStorage();

    WUPSStorageError CloseStorage();

    [[nodiscard]] wups_storage_root_item getStorageRootItem() const;

    void setInitDone(bool val);

    [[nodiscard]] bool isInitDone() const;

private:
    PluginMetaInformation mMetaInformation;
    PluginLinkInformation mPluginLinkInformation;
    std::shared_ptr<PluginData> mPluginData;

    std::optional<PluginConfigData> mPluginConfigData = std::nullopt;
    wups_storage_root_item mStorageRootItem           = nullptr;
    bool mInitDone                                    = false;
};
