#include "PluginContainer.h"

PluginContainer::PluginContainer(PluginMetaInformation metaInformation, PluginInformation pluginInformation, std::shared_ptr<PluginData> pluginData)
    : mMetaInformation(std::move(metaInformation)),
      mPluginInformation(std::move(pluginInformation)),
      mPluginData(std::move(pluginData)) {
}

PluginContainer::PluginContainer(PluginContainer &&src) : mMetaInformation(std::move(src.mMetaInformation)),
                                                          mPluginInformation(std::move(src.mPluginInformation)),
                                                          mPluginData(std::move(src.mPluginData)),
                                                          mPluginConfigData(std::move(src.mPluginConfigData)),
                                                          storageRootItem(src.storageRootItem)

{
    src.storageRootItem = {};
}

PluginContainer &PluginContainer::operator=(PluginContainer &&src) {
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

const PluginMetaInformation &PluginContainer::getMetaInformation() const {
    return this->mMetaInformation;
}

const PluginInformation &PluginContainer::getPluginInformation() const {
    return this->mPluginInformation;
}

PluginInformation &PluginContainer::getPluginInformation() {
    return this->mPluginInformation;
}

std::shared_ptr<PluginData> PluginContainer::getPluginDataCopy() const {
    return mPluginData;
}

uint32_t PluginContainer::getHandle() const {
    return (uint32_t) this;
}

const std::optional<PluginConfigData> &PluginContainer::getConfigData() const {
    return mPluginConfigData;
}

void PluginContainer::setConfigData(const PluginConfigData &pluginConfigData) {
    mPluginConfigData = pluginConfigData;
}

WUPSStorageError PluginContainer::OpenStorage() {
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

WUPSStorageError PluginContainer::CloseStorage() {
    if (getMetaInformation().getWUPSVersion() < WUPSVersion(0, 8, 0)) {
        return WUPS_STORAGE_ERROR_SUCCESS;
    }
    if (storageRootItem == nullptr) {
        return WUPS_STORAGE_ERROR_SUCCESS;
    }
    return StorageUtils::API::Internal::CloseStorage(storageRootItem);
}
