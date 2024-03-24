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
#include "PluginInformation.h"
#include "PluginMetaInformation.h"
#include "utils/storage/StorageUtils.h"
#include <memory>
#include <utility>
#include <wups/config_api.h>

class PluginContainer {
public:
    PluginContainer(PluginMetaInformation metaInformation, PluginInformation pluginInformation, std::shared_ptr<PluginData> pluginData)
        : mMetaInformation(std::move(metaInformation)),
          mPluginInformation(std::move(pluginInformation)),
          mPluginData(std::move(pluginData)) {
    }


    PluginContainer(const PluginContainer &) = delete;


    PluginContainer(PluginContainer &&src) : mMetaInformation(std::move(src.mMetaInformation)),
                                             mPluginInformation(std::move(src.mPluginInformation)),
                                             mPluginData(std::move(src.mPluginData)),
                                             mPluginConfigData(std::move(src.mPluginConfigData)),
                                             storageRootItem(src.storageRootItem)

    {
        src.storageRootItem = {};
    }

    PluginContainer &operator=(PluginContainer &&src) {
        if (this != &src) {
            this->mMetaInformation   = src.mMetaInformation;
            this->mPluginInformation = std::move(src.mPluginInformation);
            this->mPluginData        = std::move(src.mPluginData);
            this->mPluginConfigData  = std::move(src.mPluginConfigData);
            this->storageRootItem    = src.storageRootItem;

            storageRootItem = nullptr;
        }
        return *this;
    }


    [[nodiscard]] const PluginMetaInformation &getMetaInformation() const {
        return this->mMetaInformation;
    }

    [[nodiscard]] const PluginInformation &getPluginInformation() const {
        return this->mPluginInformation;
    }
    [[nodiscard]]  PluginInformation &getPluginInformation()  {
        return this->mPluginInformation;
    }

    [[nodiscard]] std::shared_ptr<PluginData> getPluginDataCopy() const {
        return mPluginData;
    }

    [[nodiscard]] uint32_t getHandle() const {
        return (uint32_t) this;
    }

    [[nodiscard]] const std::optional<PluginConfigData> &getConfigData() const {
        return mPluginConfigData;
    }

    void setConfigData(const PluginConfigData &pluginConfigData) {
        mPluginConfigData = pluginConfigData;
    }

    WUPSStorageError OpenStorage() {
        if (getMetaInformation().getWUPSVersion() < WUPSVersion(0, 8, 0)) {
            return WUPS_STORAGE_ERROR_SUCCESS;
        }
        auto &storageId = getMetaInformation().getStorageId();
        if (storageId.empty()) {
            return WUPS_STORAGE_ERROR_SUCCESS;
        }
        auto res = StorageUtils::API::Internal::OpenStorage(storageId, storageRootItem);
        if (res != WUPS_STORAGE_ERROR_SUCCESS) {
            storageRootItem = nullptr;
        }
        return res;
    }

    WUPSStorageError CloseStorage() {
        if (getMetaInformation().getWUPSVersion() < WUPSVersion(0, 8, 0)) {
            return WUPS_STORAGE_ERROR_SUCCESS;
        }
        if (storageRootItem == nullptr) {
            return WUPS_STORAGE_ERROR_SUCCESS;
        }
        return StorageUtils::API::Internal::CloseStorage(storageRootItem);
    }

    [[nodiscard]] wups_storage_root_item getStorageRootItem() const {
        return storageRootItem;
    }

private:
    PluginMetaInformation mMetaInformation;
    PluginInformation mPluginInformation;
    std::shared_ptr<PluginData> mPluginData;

    std::optional<PluginConfigData> mPluginConfigData;
    wups_storage_root_item storageRootItem = nullptr;
};
