#include "PluginContainer.h"

PluginContainer::PluginContainer(PluginMetaInformation metaInformation, PluginLinkInformation pluginLinkInformation, std::shared_ptr<PluginData> pluginData)
    : mMetaInformation(std::move(metaInformation)),
      mPluginLinkInformation(std::move(pluginLinkInformation)),
      mPluginData(std::move(pluginData)) {
}

PluginContainer::PluginContainer(PluginContainer &&src) : mMetaInformation(std::move(src.mMetaInformation)),
                                                          mPluginLinkInformation(std::move(src.mPluginLinkInformation)),
                                                          mPluginData(std::move(src.mPluginData)),
                                                          mPluginConfigData(std::move(src.mPluginConfigData)),
                                                          storageRootItem(src.storageRootItem)

{
    src.storageRootItem = {};
}

PluginContainer &PluginContainer::operator=(PluginContainer &&src) {
    if (this != &src) {
        this->mMetaInformation       = src.mMetaInformation;
        this->mPluginLinkInformation = std::move(src.mPluginLinkInformation);
        this->mPluginData            = std::move(src.mPluginData);
        this->mPluginConfigData      = std::move(src.mPluginConfigData);
        this->storageRootItem        = src.storageRootItem;

        src.storageRootItem = nullptr;
    }
    return *this;
}

const PluginMetaInformation &PluginContainer::getMetaInformation() const {
    return this->mMetaInformation;
}


const PluginLinkInformation &PluginContainer::getPluginLinkInformation() const {
    return this->mPluginLinkInformation;
}

PluginLinkInformation &PluginContainer::getPluginLinkInformation() {
    return this->mPluginLinkInformation;
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

bool PluginContainer::isLinkedAndLoaded() const {
    return mPluginLinkInformation.hasValidData();
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
