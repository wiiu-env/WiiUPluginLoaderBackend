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

#include <string>
#include <vector>

class PluginMetaInformation {
public:
    PluginMetaInformation(const PluginMetaInformation &other);

    [[nodiscard]] std::string getName() const {
        return name;
    }

    [[nodiscard]] std::string getAuthor() const {
        return this->author;
    }

    [[nodiscard]] std::string getVersion() const {
        return this->version;
    }

    [[nodiscard]] std::string getLicense() const {
        return this->license;
    }

    [[nodiscard]] std::string getBuildTimestamp() const {
        return this->buildtimestamp;
    }

    [[nodiscard]] std::string getDescription() const {
        return this->description;
    }

    [[nodiscard]] size_t getSize() const {
        return this->size;
    }

private:
    PluginMetaInformation() = default;

    void setName(const std::string &_name) {
        this->name = _name;
    }

    void setAuthor(const std::string &_author) {
        this->author = _author;
    }

    void setVersion(const std::string &_version) {
        this->version = _version;
    }

    void setLicense(const std::string &_license) {
        this->license = _license;
    }

    void setBuildTimestamp(const std::string &_buildtimestamp) {
        this->buildtimestamp = _buildtimestamp;
    }

    void setDescription(const std::string &_description) {
        this->description = _description;
    }

    void setSize(size_t _size) {
        this->size = _size;
    }

    std::string name;
    std::string author;
    std::string version;
    std::string license;
    std::string buildtimestamp;
    std::string description;
    size_t size{};

    friend class PluginMetaInformationFactory;

    friend class PluginContainerPersistence;

    friend class PluginContainer;
};
