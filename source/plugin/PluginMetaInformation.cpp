#include "PluginMetaInformation.h"

#include <cstdint>

[[nodiscard]] const std::string &PluginMetaInformation::getName() const {
    return mName;
}

[[nodiscard]] const std::string &PluginMetaInformation::getAuthor() const {
    return mAuthor;
}

[[nodiscard]] const std::string &PluginMetaInformation::getVersion() const {
    return mVersion;
}

[[nodiscard]] const std::string &PluginMetaInformation::getLicense() const {
    return mLicense;
}

[[nodiscard]] const std::string &PluginMetaInformation::getBuildTimestamp() const {
    return mBuildTimestamp;
}

[[nodiscard]] const std::string &PluginMetaInformation::getDescription() const {
    return mDescription;
}

[[nodiscard]] const WUPSVersion &PluginMetaInformation::getWUPSVersion() const {
    return this->mWUPSVersion;
}

[[nodiscard]] const std::string &PluginMetaInformation::getStorageId() const {
    return mStorageId;
}

[[nodiscard]] size_t PluginMetaInformation::getSize() const {
    return mSize;
}

PluginMetaInformation::PluginMetaInformation() = default;

void PluginMetaInformation::setName(std::string name) {
    mName = std::move(name);
}

void PluginMetaInformation::setAuthor(std::string author) {
    mAuthor = std::move(author);
}

void PluginMetaInformation::setVersion(std::string version) {
    mVersion = std::move(version);
}

void PluginMetaInformation::setLicense(std::string license) {
    mLicense = std::move(license);
}

void PluginMetaInformation::setBuildTimestamp(std::string buildTimestamp) {
    mBuildTimestamp = std::move(buildTimestamp);
}

void PluginMetaInformation::setDescription(std::string description) {
    mDescription = std::move(description);
}

void PluginMetaInformation::setWUPSVersion(const uint16_t major, const uint16_t minor, const uint16_t revision) {
    mWUPSVersion = WUPSVersion(major, minor, revision);
}

void PluginMetaInformation::setWUPSVersion(const WUPSVersion &wupsVersion) {
    mWUPSVersion = wupsVersion;
}

void PluginMetaInformation::setSize(const size_t size) {
    mSize = size;
}

void PluginMetaInformation::setStorageId(std::string storageId) {
    mStorageId = std::move(storageId);
}