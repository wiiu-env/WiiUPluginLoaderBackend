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

    PluginInformation(PluginInformation &&src);

    PluginInformation &operator=(PluginInformation &&src);

    [[nodiscard]] const std::vector<HookData> &getHookDataList() const;

    [[nodiscard]] const std::vector<FunctionData> &getFunctionDataList() const;

    [[nodiscard]] std::vector<FunctionData> &getFunctionDataList();

    [[nodiscard]] const std::vector<RelocationData> &getRelocationDataList() const;

    [[nodiscard]] const std::map<std::string, SectionInfo> &getSectionInfoList() const;

    [[nodiscard]] std::optional<SectionInfo> getSectionInfo(const std::string &sectionName) const;

    [[nodiscard]] uint8_t getTrampolineId() const;

    [[nodiscard]] const FunctionSymbolData *getNearestFunctionSymbolData(uint32_t address) const;

    [[nodiscard]] const HeapMemoryFixedSize &getTextMemory() const;

    [[nodiscard]] const HeapMemoryFixedSize &getDataMemory() const;

private:
    PluginInformation() = default;

    void addHookData(HookData hook_data);

    void addFunctionData(FunctionData function_data);

    void addRelocationData(RelocationData relocation_data);

    void addFunctionSymbolData(const FunctionSymbolData &symbol_data);

    void addSectionInfo(const SectionInfo &sectionInfo);

    void setTrampolineId(uint8_t trampolineId);

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
