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
    bool operator()(const std::unique_ptr<FunctionSymbolData> &lhs,
                    const std::unique_ptr<FunctionSymbolData> &rhs) const {
        return *lhs < *rhs;
    }
};

class PluginInformation {
public:
    void addHookData(std::unique_ptr<HookData> hook_data) {
        mHookDataList.push_back(std::move(hook_data));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<HookData>> &getHookDataList() const {
        return mHookDataList;
    }

    void addFunctionData(std::unique_ptr<FunctionData> function_data) {
        mFunctionDataList.push_back(std::move(function_data));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<FunctionData>> &getFunctionDataList() const {
        return mFunctionDataList;
    }

    void addRelocationData(std::unique_ptr<RelocationData> relocation_data) {
        mRelocationDataList.push_back(std::move(relocation_data));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<RelocationData>> &getRelocationDataList() const {
        return mRelocationDataList;
    }

    void addFunctionSymbolData(std::unique_ptr<FunctionSymbolData> symbol_data) {
        mSymbolDataList.insert(std::move(symbol_data));
    }

    void addSectionInfo(std::unique_ptr<SectionInfo> sectionInfo) {
        mSectionInfoList[sectionInfo->getName()] = std::move(sectionInfo);
    }

    [[nodiscard]] const std::map<std::string, std::unique_ptr<SectionInfo>> &getSectionInfoList() const {
        return mSectionInfoList;
    }

    [[nodiscard]] std::optional<SectionInfo> getSectionInfo(const std::string &sectionName) const {
        if (getSectionInfoList().contains(sectionName)) {
            return *mSectionInfoList.at(sectionName);
        }
        return std::nullopt;
    }

    void setTrampolineId(uint8_t trampolineId) {
        this->mTrampolineId = trampolineId;
    }

    [[nodiscard]] uint8_t getTrampolineId() const {
        return mTrampolineId;
    }

    [[nodiscard]] std::optional<FunctionSymbolData> getNearestFunctionSymbolData(uint32_t address) const {
        FunctionSymbolData *result = nullptr;

        bool foundHit = false;
        for (auto &cur : mSymbolDataList) {
            if (foundHit && address < (uint32_t) cur->getAddress()) {
                break;
            }
            if (address >= (uint32_t) cur->getAddress()) {
                result   = cur.get();
                foundHit = true;
            }
        }
        if (!foundHit) {
            return std::nullopt;
        }

        return *result;
    }

    [[nodiscard]] const HeapMemoryFixedSize &getTextMemory() const {
        return mAllocatedTextMemoryAddress;
    }

    [[nodiscard]] const HeapMemoryFixedSize &getDataMemory() const {
        return mAllocatedDataMemoryAddress;
    }

private:
    std::vector<std::unique_ptr<HookData>> mHookDataList;
    std::vector<std::unique_ptr<FunctionData>> mFunctionDataList;
    std::vector<std::unique_ptr<RelocationData>> mRelocationDataList;
    std::set<std::unique_ptr<FunctionSymbolData>, FunctionSymbolDataComparator> mSymbolDataList;
    std::map<std::string, std::unique_ptr<SectionInfo>> mSectionInfoList;

    uint8_t mTrampolineId = 0;

    HeapMemoryFixedSize mAllocatedTextMemoryAddress;
    HeapMemoryFixedSize mAllocatedDataMemoryAddress;

    friend class PluginInformationFactory;
};
