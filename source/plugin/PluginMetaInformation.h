/****************************************************************************
 * Copyright (C) 2019,2020 Maschell
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

#include "WUPSVersion.h"
#include <string>
#include <vector>

class PluginMetaInformation {
public:
    [[nodiscard]] const std::string &getName() const {
        return name;
    }

    [[nodiscard]] const std::string &getAuthor() const {
        return this->author;
    }

    [[nodiscard]] const std::string &getVersion() const {
        return this->version;
    }

    [[nodiscard]] const std::string &getLicense() const {
        return this->license;
    }

    [[nodiscard]] const std::string &getBuildTimestamp() const {
        return this->buildtimestamp;
    }

    [[nodiscard]] const std::string &getDescription() const {
        return this->description;
    }

    [[nodiscard]] const WUPSVersion &getWUPSVersion() const {
        return this->wupsversion;
    }

    [[nodiscard]] const std::string &getStorageId() const {
        return this->storageId;
    }

    [[nodiscard]] size_t getSize() const {
        return this->size;
    }

private:
    PluginMetaInformation() = default;

    void setName(std::string _name) {
        this->name = std::move(_name);
    }

    void setAuthor(std::string _author) {
        this->author = std::move(_author);
    }

    void setVersion(std::string _version) {
        this->version = std::move(_version);
    }

    void setLicense(std::string _license) {
        this->license = std::move(_license);
    }

    void setBuildTimestamp(std::string _buildtimestamp) {
        this->buildtimestamp = std::move(_buildtimestamp);
    }

    void setDescription(std::string _description) {
        this->description = std::move(_description);
    }

    void setWUPSVersion(uint16_t major, uint16_t minor, uint16_t revision) {
        this->wupsversion = WUPSVersion(major, minor, revision);
    }

    void setWUPSVersion(WUPSVersion &_wupsversion) {
        this->wupsversion = _wupsversion;
    }

    void setSize(size_t _size) {
        this->size = _size;
    }

    void setStorageId(std::string _storageId) {
        this->storageId = std::move(_storageId);
    }

    std::string name;
    std::string author;
    std::string version;
    std::string license;
    std::string buildtimestamp;
    std::string description;
    std::string storageId;
    size_t size{};
    WUPSVersion wupsversion = WUPSVersion(0, 0, 0);

    friend class PluginMetaInformationFactory;

    friend class PluginContainer;
};
