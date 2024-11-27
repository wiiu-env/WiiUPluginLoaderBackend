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

class PluginMetaInformation {
public:
    [[nodiscard]] const std::string &getName() const;

    [[nodiscard]] const std::string &getAuthor() const;

    [[nodiscard]] const std::string &getVersion() const;

    [[nodiscard]] const std::string &getLicense() const;

    [[nodiscard]] const std::string &getBuildTimestamp() const;

    [[nodiscard]] const std::string &getDescription() const;

    [[nodiscard]] const WUPSVersion &getWUPSVersion() const;

    [[nodiscard]] const std::string &getStorageId() const;

    [[nodiscard]] size_t getSize() const;

private:
    PluginMetaInformation();

    void setName(std::string name);

    void setAuthor(std::string author);

    void setVersion(std::string version);

    void setLicense(std::string license);

    void setBuildTimestamp(std::string buildTimestamp);

    void setDescription(std::string description);

    void setWUPSVersion(uint16_t major, uint16_t minor, uint16_t revision);

    void setWUPSVersion(const WUPSVersion &wupsVersion);

    void setSize(size_t size);

    void setStorageId(std::string storageId);

    std::string mName;
    std::string mAuthor;
    std::string mVersion;
    std::string mLicense;
    std::string mBuildTimestamp;
    std::string mDescription;
    std::string mStorageId;
    size_t mSize             = {};
    WUPSVersion mWUPSVersion = WUPSVersion(0, 0, 0);

    friend class PluginMetaInformationFactory;

    friend class PluginContainer;
};
