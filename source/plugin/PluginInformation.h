/****************************************************************************
 * Copyright (C) 2018 Maschell
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

#include "FunctionData.h"
#include "FunctionSymbolData.h"
#include "HookData.h"
#include "PluginMetaInformation.h"
#include "RelocationData.h"
#include "SectionInfo.h"
#include "utils/HeapMemoryFixedSize.h"
#include "utils/utils.h"
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <vector>

struct FunctionSymbolDataComparator {
    bool operator()(const FunctionSymbolData &lhs,
                    const FunctionSymbolData &rhs) const {
        return lhs < rhs;
    }
};

class PluginInformation {
public:
    PluginInformation(const PluginInformation &) = delete;


    PluginInformation(PluginInformation &&src) : mHookDataList(std::move(src.mHookDataList)),
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

    PluginInformation &operator=(PluginInformation &&src) {
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


    void addHookData(HookData hook_data) {
        mHookDataList.push_back(hook_data);
    }

    [[nodiscard]] const std::vector<HookData> &getHookDataList() const {
        return mHookDataList;
    }

    void addFunctionData(FunctionData function_data) {
        mFunctionDataList.push_back(std::move(function_data));
    }

    [[nodiscard]] const std::vector<FunctionData> &getFunctionDataList() const {
        return mFunctionDataList;
    }

    [[nodiscard]] std::vector<FunctionData> &getFunctionDataList() {
        return mFunctionDataList;
    }

    void addRelocationData(RelocationData relocation_data) {
        mRelocationDataList.push_back(std::move(relocation_data));
    }

    [[nodiscard]] const std::vector<RelocationData> &getRelocationDataList() const {
        return mRelocationDataList;
    }

    void addFunctionSymbolData(const FunctionSymbolData &symbol_data) {
        mSymbolDataList.insert(symbol_data);
    }

    void addSectionInfo(const SectionInfo &sectionInfo) {
        mSectionInfoList.insert(std::pair(sectionInfo.getName(), sectionInfo));
    }

    [[nodiscard]] const std::map<std::string, SectionInfo> &getSectionInfoList() const {
        return mSectionInfoList;
    }

    [[nodiscard]] std::optional<SectionInfo> getSectionInfo(const std::string &sectionName) const {
        if (getSectionInfoList().contains(sectionName)) {
            return mSectionInfoList.at(sectionName);
        }
        return std::nullopt;
    }

    void setTrampolineId(uint8_t trampolineId) {
        this->mTrampolineId = trampolineId;
    }

    [[nodiscard]] uint8_t getTrampolineId() const {
        return mTrampolineId;
    }

    [[nodiscard]] const FunctionSymbolData *getNearestFunctionSymbolData(uint32_t address) const {
        const FunctionSymbolData *result = nullptr;

        bool foundHit = false;
        for (auto &cur : mSymbolDataList) {
            if (foundHit && address < (uint32_t) cur.getAddress()) {
                break;
            }
            if (address >= (uint32_t) cur.getAddress()) {
                result   = &cur;
                foundHit = true;
            }
        }
        if (!foundHit) {
            return nullptr;
        }

        return result;
    }

    [[nodiscard]] const HeapMemoryFixedSize &getTextMemory() const {
        return mAllocatedTextMemoryAddress;
    }

    [[nodiscard]] const HeapMemoryFixedSize &getDataMemory() const {
        return mAllocatedDataMemoryAddress;
    }

private:
    PluginInformation(){

    }
    std::vector<HookData> mHookDataList;
    std::vector<FunctionData> mFunctionDataList;
    std::vector<RelocationData> mRelocationDataList;
    std::set<FunctionSymbolData, FunctionSymbolDataComparator> mSymbolDataList;
    std::map<std::string, SectionInfo> mSectionInfoList;

    uint8_t mTrampolineId = 0;

    HeapMemoryFixedSize mAllocatedTextMemoryAddress;
    HeapMemoryFixedSize mAllocatedDataMemoryAddress;

    friend class PluginInformationFactory;
};
