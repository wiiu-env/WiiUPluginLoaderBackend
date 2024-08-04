#include "PluginLinkInformation.h"

PluginLinkInformation::PluginLinkInformation(PluginLinkInformation &&src) : mHookDataList(std::move(src.mHookDataList)),
                                                                            mFunctionDataList(std::move(src.mFunctionDataList)),
                                                                            mRelocationDataList(std::move(src.mRelocationDataList)),
                                                                            mSymbolDataList(std::move(src.mSymbolDataList)),
                                                                            mSectionInfoList(std::move(src.mSectionInfoList)),
                                                                            mTrampolineId(src.mTrampolineId),
                                                                            mAllocatedTextMemoryAddress(std::move(src.mAllocatedTextMemoryAddress)),
                                                                            mAllocatedDataMemoryAddress(std::move(src.mAllocatedDataMemoryAddress))

{
    src.mTrampolineId = {};
}

PluginLinkInformation &PluginLinkInformation::operator=(PluginLinkInformation &&src) {
    if (this != &src) {
        this->mHookDataList               = std::move(src.mHookDataList);
        this->mFunctionDataList           = std::move(src.mFunctionDataList);
        this->mRelocationDataList         = std::move(src.mRelocationDataList);
        this->mSymbolDataList             = std::move(src.mSymbolDataList);
        this->mSectionInfoList            = std::move(src.mSectionInfoList);
        this->mTrampolineId               = src.mTrampolineId;
        this->mAllocatedTextMemoryAddress = std::move(src.mAllocatedTextMemoryAddress);
        this->mAllocatedDataMemoryAddress = std::move(src.mAllocatedDataMemoryAddress);
        src.mTrampolineId                 = {};
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


void PluginLinkInformation::setTrampolineId(const uint8_t trampolineId) {
    this->mTrampolineId = trampolineId;
}

uint8_t PluginLinkInformation::getTrampolineId() const {
    return mTrampolineId;
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

const HeapMemoryFixedSize &PluginLinkInformation::getTextMemory() const {
    return mAllocatedTextMemoryAddress;
}

const HeapMemoryFixedSize &PluginLinkInformation::getDataMemory() const {
    return mAllocatedDataMemoryAddress;
}

PluginLinkInformation PluginLinkInformation::CreateStub() {
    return {};
}

bool PluginLinkInformation::hasValidData() const {
    return mAllocatedDataMemoryAddress.size() > 0 && mAllocatedTextMemoryAddress.size() > 0;
}
