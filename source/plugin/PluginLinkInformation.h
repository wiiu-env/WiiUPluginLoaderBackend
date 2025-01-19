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

#include "FunctionSymbolData.h"
#include "utils/HeapMemoryFixedSize.h"

#include <iosfwd>
#include <map>
#include <optional>
#include <set>
#include <vector>
#include <wums/defines/relocation_defines.h>

class HeapMemoryFixedSizePool;
class SectionInfo;
class RelocationData;
class FunctionData;
class HookData;

struct FunctionSymbolDataComparator {
    bool operator()(const FunctionSymbolData &lhs,
                    const FunctionSymbolData &rhs) const {
        return lhs < rhs;
    }
};

class PluginLinkInformation {
public:
    static PluginLinkInformation CreateStub();

    PluginLinkInformation(const PluginLinkInformation &) = delete;

    PluginLinkInformation(PluginLinkInformation &&src) noexcept;

    PluginLinkInformation &operator=(PluginLinkInformation &&src) noexcept;

    [[nodiscard]] const std::vector<HookData> &getHookDataList() const;

    [[nodiscard]] const std::vector<FunctionData> &getFunctionDataList() const;

    [[nodiscard]] std::vector<FunctionData> &getFunctionDataList();

    [[nodiscard]] const std::vector<RelocationData> &getRelocationDataList() const;

    [[nodiscard]] const std::map<std::string, SectionInfo> &getSectionInfoList() const;

    [[nodiscard]] std::optional<SectionInfo> getSectionInfo(const std::string &sectionName) const;

    [[nodiscard]] const FunctionSymbolData *getNearestFunctionSymbolData(uint32_t address) const;

    [[nodiscard]] HeapMemoryFixedSizePool::MemorySegmentInfo getTextMemory() const;

    [[nodiscard]] HeapMemoryFixedSizePool::MemorySegmentInfo getDataMemory() const;

    [[nodiscard]] bool hasValidData() const;

    [[nodiscard]] int numberOfSegments() const;

    [[nodiscard]] std::span<relocation_trampoline_entry_t> getTrampData() const;

private:
    PluginLinkInformation() = default;

    void addHookData(const HookData &hook_data);

    void addFunctionData(FunctionData function_data);

    void addRelocationData(RelocationData relocation_data);

    void addFunctionSymbolData(const FunctionSymbolData &symbol_data);

    void addSectionInfo(const SectionInfo &sectionInfo);

    std::vector<HookData> mHookDataList;
    std::vector<FunctionData> mFunctionDataList;
    std::vector<RelocationData> mRelocationDataList;
    std::set<FunctionSymbolData, FunctionSymbolDataComparator> mSymbolDataList;
    std::map<std::string, SectionInfo> mSectionInfoList;

    HeapMemoryFixedSizePool mAllocatedTextAndTrampMemoryAddress;
    HeapMemoryFixedSizePool mAllocatedDataMemoryAddress;

    friend class PluginLinkInformationFactory;
};
