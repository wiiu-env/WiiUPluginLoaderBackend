#include "PluginInformation.h"

PluginInformation::PluginInformation(PluginInformation &&src) : mHookDataList(std::move(src.mHookDataList)),
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

PluginInformation &PluginInformation::operator=(PluginInformation &&src) {
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

void PluginInformation::addHookData(const HookData &hook_data) {
    mHookDataList.push_back(hook_data);
}

const std::vector<HookData> &PluginInformation::getHookDataList() const {
    return mHookDataList;
}

void PluginInformation::addFunctionData(FunctionData function_data) {
    mFunctionDataList.push_back(std::move(function_data));
}

const std::vector<FunctionData> &PluginInformation::getFunctionDataList() const {
    return mFunctionDataList;
}

std::vector<FunctionData> &PluginInformation::getFunctionDataList() {
    return mFunctionDataList;
}

void PluginInformation::addRelocationData(RelocationData relocation_data) {
    mRelocationDataList.push_back(std::move(relocation_data));
}

const std::vector<RelocationData> &PluginInformation::getRelocationDataList() const {
    return mRelocationDataList;
}

void PluginInformation::addFunctionSymbolData(const FunctionSymbolData &symbol_data) {
    mSymbolDataList.insert(symbol_data);
}

void PluginInformation::addSectionInfo(const SectionInfo &sectionInfo) {
    mSectionInfoList.insert(std::pair(sectionInfo.getName(), sectionInfo));
}

const std::map<std::string, SectionInfo> &PluginInformation::getSectionInfoList() const {
    return mSectionInfoList;
}

std::optional<SectionInfo> PluginInformation::getSectionInfo(const std::string &sectionName) const {
    if (getSectionInfoList().contains(sectionName)) {
        return mSectionInfoList.at(sectionName);
    }
    return std::nullopt;
}

void PluginInformation::setTrampolineId(const uint8_t trampolineId) {
    this->mTrampolineId = trampolineId;
}

uint8_t PluginInformation::getTrampolineId() const {
    return mTrampolineId;
}

const FunctionSymbolData *PluginInformation::getNearestFunctionSymbolData(const uint32_t address) const {
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

const HeapMemoryFixedSize &PluginInformation::getTextMemory() const {
    return mAllocatedTextMemoryAddress;
}

const HeapMemoryFixedSize &PluginInformation::getDataMemory() const {
    return mAllocatedDataMemoryAddress;
}
