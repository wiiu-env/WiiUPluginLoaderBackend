#include "PluginContainer.h"
#include "utils/storage/StorageUtils.h"

#include <utils/buttoncombo/ButtonComboUtils.h>

PluginContainer::PluginContainer(PluginMetaInformation metaInformation, PluginLinkInformation pluginLinkInformation, std::shared_ptr<PluginData> pluginData)
    : mMetaInformation(std::move(metaInformation)),
      mPluginLinkInformation(std::move(pluginLinkInformation)),
      mPluginData(std::move(pluginData)) {
    // Abuse this as a stable handle that references itself and survives std::move
    *mHandle = reinterpret_cast<uint32_t>(mHandle.get());
}

PluginContainer::PluginContainer(PluginContainer &&src) noexcept : mMetaInformation(std::move(src.mMetaInformation)),
                                                                   mPluginLinkInformation(std::move(src.mPluginLinkInformation)),
                                                                   mPluginData(std::move(src.mPluginData)),
                                                                   mHandle(std::move(src.mHandle)),
                                                                   mPluginConfigData(std::move(src.mPluginConfigData)),
                                                                   mStorageRootItem(src.mStorageRootItem),
                                                                   mInitDone(src.mInitDone),
                                                                   mButtonComboManagerHandle(src.mButtonComboManagerHandle) {
    src.mHandle          = {};
    src.mStorageRootItem = {};
    src.mInitDone        = {};
}

PluginContainer &PluginContainer::operator=(PluginContainer &&src) noexcept {
    if (this != &src) {
        this->mMetaInformation          = std::move(src.mMetaInformation);
        this->mPluginLinkInformation    = std::move(src.mPluginLinkInformation);
        this->mPluginData               = std::move(src.mPluginData);
        this->mPluginConfigData         = std::move(src.mPluginConfigData);
        this->mHandle                   = std::move(src.mHandle);
        this->mStorageRootItem          = src.mStorageRootItem;
        this->mInitDone                 = src.mInitDone;
        this->mButtonComboManagerHandle = src.mButtonComboManagerHandle;

        src.mStorageRootItem          = nullptr;
        src.mInitDone                 = false;
        src.mButtonComboManagerHandle = false;
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
    return *mHandle;
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

    const auto res = StorageUtils::API::Internal::OpenStorage(storageId, mStorageRootItem);
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

void PluginContainer::InitButtonComboData() {
    if (getMetaInformation().getWUPSVersion() < WUPSVersion(0, 8, 2)) {
        return;
    }
    mButtonComboManagerHandle = ButtonComboUtils::API::Internal::CreateButtonComboData();
}
void PluginContainer::DeinitButtonComboData() {
    if (getMetaInformation().getWUPSVersion() < WUPSVersion(0, 8, 2)) {
        return;
    }
    ButtonComboUtils::API::Internal::RemoveButtonComboData(mButtonComboManagerHandle);
}

uint32_t PluginContainer::getButtonComboManagerHandle() const {
    return mButtonComboManagerHandle;
}