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
    PluginMetaInformation(const PluginMetaInformation& other);

    const std::string getName() const {
        return name;
    }

    const std::string getAuthor() const {
        return this->author;
    }

    const std::string getVersion() const {
        return this->version;
    }

    const std::string getLicense() const {
        return this->license;
    }

    const std::string getBuildTimestamp() const {
        return this->buildtimestamp;
    }

    const std::string getDescription() const {
        return this->description;
    }

    const size_t getSize() const {
        return this->size;
    }

private:
    PluginMetaInformation() {
    }

    void setName(const std::string &name) {
        this->name = name;
    }

    void setAuthor(const std::string &author) {
        this->author = author;
    }

    void setVersion(const std::string &version) {
        this->version = version;
    }

    void setLicense(const std::string &license) {
        this->license = license;
    }

    void setBuildTimestamp(const std::string &buildtimestamp) {
        this->buildtimestamp = buildtimestamp;
    }

    void setDescription(const std::string &description) {
        this->description = description;
    }

    void setSize(size_t size) {
        this->size = size;
    }

    std::string name;
    std::string author;
    std::string version;
    std::string license;
    std::string buildtimestamp;
    std::string description;
    size_t size;

    friend class PluginMetaInformationFactory;

    friend class PluginContainerPersistence;

    friend class PluginContainer;
};
