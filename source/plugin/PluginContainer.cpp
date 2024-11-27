#include "PluginContainer.h"
#include "utils/storage/StorageUtils.h"

PluginContainer::PluginContainer(PluginMetaInformation metaInformation, PluginInformation pluginInformation, std::shared_ptr<PluginData> pluginData)
    : mMetaInformation(std::move(metaInformation)),
      mPluginInformation(std::move(pluginInformation)),
      mPluginData(std::move(pluginData)) {
}

PluginContainer::PluginContainer(PluginContainer &&src) noexcept : mMetaInformation(std::move(src.mMetaInformation)),
                                                                   mPluginInformation(std::move(src.mPluginInformation)),
                                                                   mPluginData(std::move(src.mPluginData)),
                                                                   mPluginConfigData(std::move(src.mPluginConfigData)),
                                                                   mStorageRootItem(src.mStorageRootItem),
                                                                   mInitDone(src.mInitDone)

{
    src.mStorageRootItem = {};
    src.mInitDone        = {};
}

PluginContainer &PluginContainer::operator=(PluginContainer &&src) noexcept {
    if (this != &src) {
        this->mMetaInformation   = std::move(src.mMetaInformation);
        this->mPluginInformation = std::move(src.mPluginInformation);
        this->mPluginData        = std::move(src.mPluginData);
        this->mPluginConfigData  = std::move(src.mPluginConfigData);
        this->mStorageRootItem   = src.mStorageRootItem;
        this->mInitDone          = src.mInitDone;

        src.mStorageRootItem = nullptr;
        src.mInitDone        = false;
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
    return reinterpret_cast<uint32_t>(this);
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
    auto res = StorageUtils::API::Internal::OpenStorage(storageId, mStorageRootItem);
    if (res != WUPS_STORAGE_ERROR_SUCCESS) {
        mStorageRootItem = nullptr;
    }
    return res;
}

WUPSStorageError PluginContainer::CloseStorage() {
    if (getMetaInformation().getWUPSVersion() < WUPSVersion(0, 8, 0)) {
        return WUPS_STORAGE_ERROR_SUCCESS;
    }
    if (mStorageRootItem == nullptr) {
        return WUPS_STORAGE_ERROR_SUCCESS;
    }
    return StorageUtils::API::Internal::CloseStorage(mStorageRootItem);
}

wups_storage_root_item PluginContainer::getStorageRootItem() const {
    return mStorageRootItem;
}

void PluginContainer::setInitDone(const bool val) {
    mInitDone = val;
}

bool PluginContainer::isInitDone() const {
    return mInitDone;
}