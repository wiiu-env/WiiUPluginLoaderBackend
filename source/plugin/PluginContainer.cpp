#include "PluginContainer.h"

#include "plugin/ButtonComboManager.h"
#include "plugin/FunctionData.h"
#include "plugin/HookData.h"
#include "plugin/PluginConfigData.h"
#include "plugin/PluginData.h"
#include "plugin/PluginLinkInformation.h"
#include "plugin/RelocationData.h"
#include "plugin/SectionInfo.h"

#include "plugin/ButtonComboManager.h"
#include "utils/buttoncombo/ButtonComboUtils.h"
#include "utils/logger.h"
#include "utils/storage/StorageUtils.h"

#include <optional>

PluginContainer::PluginContainer(PluginMetaInformation metaInformation, PluginLinkInformation pluginLinkInformation, std::shared_ptr<PluginData> pluginData, std::optional<PluginMetaInformation::HeapTrackingOptions> heapTrackingOptions)
    : mMetaInformation(std::move(metaInformation)),
      mPluginLinkInformation(std::move(pluginLinkInformation)),
      mPluginData(std::move(pluginData)) {
    // Abuse this as a stable handle that references itself and survives std::move
    *mHandle = reinterpret_cast<uint32_t>(mHandle.get());

    auto trackingOptions = mMetaInformation.getHeapTrackingOptions();
    if (heapTrackingOptions) {
        trackingOptions = *heapTrackingOptions;
    }

    if (const bool res = useTrackingPluginHeapMemoryAllocator(trackingOptions); !res) {
        DEBUG_FUNCTION_LINE_WARN("Failed to set heap tracking options for \"%s\"", mMetaInformation.getName().c_str());
    }
}

PluginContainer::PluginContainer(PluginContainer &&src) noexcept : mMetaInformation(std::move(src.mMetaInformation)),
                                                                   mPluginLinkInformation(std::move(src.mPluginLinkInformation)),
                                                                   mTrackingHeapAllocatorOpt(std::move(src.mTrackingHeapAllocatorOpt)),
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
        this->mTrackingHeapAllocatorOpt = std::move(src.mTrackingHeapAllocatorOpt);
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


std::vector<ButtonComboInfo> PluginContainer::GetButtonComboData() const {
    if (getMetaInformation().getWUPSVersion() < WUPSVersion(0, 8, 2)) {
        return {};
    }
    return ButtonComboUtils::API::Internal::GetButtonComboData(mButtonComboManagerHandle);
}

uint32_t PluginContainer::getButtonComboManagerHandle() const {
    return mButtonComboManagerHandle;
}

bool PluginContainer::useTrackingPluginHeapMemoryAllocator(PluginMetaInformation::HeapTrackingOptions options) {
    if (options != PluginMetaInformation::TRACK_HEAP_OPTIONS_NONE) {
        if (!this->mTrackingHeapAllocatorOpt) {
            uint32_t stackTraceDepth = 0;
            if (options == PluginMetaInformation::TRACK_HEAP_OPTIONS_TRACK_SIZE_AND_COLLECT_STACK_TRACES) {
                stackTraceDepth = 8;
            }
            this->mTrackingHeapAllocatorOpt = TrackingPluginHeapMemoryAllocator::Create(this->mMetaInformation.getName(), stackTraceDepth);
        }

        return this->mTrackingHeapAllocatorOpt.has_value();
    }
    this->mTrackingHeapAllocatorOpt.reset();
    return true;
}

bool PluginContainer::isUsingTrackingPluginHeapMemoryAllocator() const {
    return mTrackingHeapAllocatorOpt.has_value();
}

const IPluginHeapMemoryAllocator &PluginContainer::getMemoryAllocator() const {
    if (mTrackingHeapAllocatorOpt) {
        return *mTrackingHeapAllocatorOpt;
    }
    return DefaultPluginHeapMemoryAllocator::gDefaultPluginHeapMemoryAllocator;
}

const TrackingPluginHeapMemoryAllocator *PluginContainer::getTrackingMemoryAllocator() const {
    if (mTrackingHeapAllocatorOpt) {
        return &(mTrackingHeapAllocatorOpt.value());
    }
    return nullptr;
}

size_t PluginContainer::getMemoryFootprint() const {
    size_t totalSize = sizeof(*this);

    if (mHandle) {
        totalSize += sizeof(uint32_t);
    }

    if (mPluginData) {
        totalSize += mPluginData->getMemoryFootprint();
    }

    if (mPluginConfigData.has_value()) {
        size_t configFootprint = mPluginConfigData->getMemoryFootprint();
        if (configFootprint > sizeof(PluginConfigData)) {
            totalSize += (configFootprint - sizeof(PluginConfigData));
        }
    }
    {
        size_t metaFootprint = mMetaInformation.getMemoryFootprint();
        if (metaFootprint > sizeof(PluginMetaInformation)) {
            totalSize += (metaFootprint - sizeof(PluginMetaInformation));
        }
    }
    {
        size_t linkFootprint = mPluginLinkInformation.getMemoryFootprint();
        if (linkFootprint > sizeof(PluginLinkInformation)) {
            totalSize += (linkFootprint - sizeof(PluginLinkInformation));
        }
    }

    return totalSize;
}