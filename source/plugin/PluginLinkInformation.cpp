#include "PluginLinkInformation.h"

#include "FunctionData.h"
#include "HookData.h"
#include "RelocationData.h"
#include "SectionInfo.h"
#include "utils/logger.h"

PluginLinkInformation::PluginLinkInformation(PluginLinkInformation &&src) : mHookDataList(std::move(src.mHookDataList)),
                                                                            mFunctionDataList(std::move(src.mFunctionDataList)),
                                                                            mRelocationDataList(std::move(src.mRelocationDataList)),
                                                                            mSymbolDataList(std::move(src.mSymbolDataList)),
                                                                            mSectionInfoList(std::move(src.mSectionInfoList)),
                                                                            mAllocatedTextAndTrampMemoryAddress(std::move(src.mAllocatedTextAndTrampMemoryAddress)),
                                                                            mAllocatedDataMemoryAddress(std::move(src.mAllocatedDataMemoryAddress))

{
}

PluginLinkInformation &PluginLinkInformation::operator=(PluginLinkInformation &&src) noexcept {
    if (this != &src) {
        this->mHookDataList                       = std::move(src.mHookDataList);
        this->mFunctionDataList                   = std::move(src.mFunctionDataList);
        this->mRelocationDataList                 = std::move(src.mRelocationDataList);
        this->mSymbolDataList                     = std::move(src.mSymbolDataList);
        this->mSectionInfoList                    = std::move(src.mSectionInfoList);
        this->mAllocatedTextAndTrampMemoryAddress = std::move(src.mAllocatedTextAndTrampMemoryAddress);
        this->mAllocatedDataMemoryAddress         = std::move(src.mAllocatedDataMemoryAddress);
    }
    return *this;
}

void PluginLinkInformation::addHookData(const HookData &hook_data) {
    mHookDataList.push_back(hook_data);
}

const std::vector<HookData> &PluginLinkInformation::getHookDataList() const {
    return mHookDataList;
}

void PluginLinkInformation::addFunctionData(FunctionData function_data) {
    mFunctionDataList.push_back(std::move(function_data));
}

const std::vector<FunctionData> &PluginLinkInformation::getFunctionDataList() const {
    return mFunctionDataList;
}

std::vector<FunctionData> &PluginLinkInformation::getFunctionDataList() {
    return mFunctionDataList;
}

void PluginLinkInformation::addRelocationData(RelocationData relocation_data) {
    mRelocationDataList.push_back(std::move(relocation_data));
}

const std::vector<RelocationData> &PluginLinkInformation::getRelocationDataList() const {
    return mRelocationDataList;
}

void PluginLinkInformation::addFunctionSymbolData(const FunctionSymbolData &symbol_data) {
    mSymbolDataList.insert(symbol_data);
}

void PluginLinkInformation::addSectionInfo(const SectionInfo &sectionInfo) {
    mSectionInfoList.insert(std::pair(sectionInfo.getName(), sectionInfo));
}

const std::map<std::string, SectionInfo> &PluginLinkInformation::getSectionInfoList() const {
    return mSectionInfoList;
}

std::optional<SectionInfo> PluginLinkInformation::getSectionInfo(const std::string &sectionName) const {
    if (getSectionInfoList().contains(sectionName)) {
        return mSectionInfoList.at(sectionName);
    }
    return std::nullopt;
}

const FunctionSymbolData *PluginLinkInformation::getNearestFunctionSymbolData(uint32_t address) const {
    const FunctionSymbolData *result = nullptr;

    bool foundHit = false;
    for (auto &cur : mSymbolDataList) {
        if (foundHit && address < reinterpret_cast<uint32_t>(cur.getAddress())) {
            break;
        }
        if (address >= reinterpret_cast<uint32_t>(cur.getAddress())) {
            result   = &cur;
            foundHit = true;
        }
    }
    if (!foundHit) {
        return nullptr;
    }

    return result;
}

HeapMemoryFixedSizePool::MemorySegmentInfo PluginLinkInformation::getTextMemory() const {
    return mAllocatedTextAndTrampMemoryAddress[0]; // 0 is .text data
}

HeapMemoryFixedSizePool::MemorySegmentInfo PluginLinkInformation::getDataMemory() const {
    return mAllocatedDataMemoryAddress[0]; // 0 is .data data
}

PluginLinkInformation PluginLinkInformation::CreateStub() {
    return {};
}

bool PluginLinkInformation::hasValidData() const {
    return mAllocatedDataMemoryAddress && mAllocatedTextAndTrampMemoryAddress && mAllocatedTextAndTrampMemoryAddress.numberOfSegments() == 2;
}
std::span<relocation_trampoline_entry_t> PluginLinkInformation::getTrampData() const {
    if (mAllocatedTextAndTrampMemoryAddress && mAllocatedTextAndTrampMemoryAddress.numberOfSegments() < 2) {
        DEBUG_FUNCTION_LINE_ERR("Failed to return trampoline data. Memory is either not valid or has not enough segments");
        return {};
    }
    const auto &entry = mAllocatedTextAndTrampMemoryAddress[1]; // 1 is tramp data
    return std::span(static_cast<relocation_trampoline_entry_t *>(entry.data()), entry.size() / sizeof(relocation_trampoline_entry_t));
}
