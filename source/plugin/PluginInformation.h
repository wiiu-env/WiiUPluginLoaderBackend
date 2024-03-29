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
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <vector>

struct FunctionSymbolDataComparator {
    bool operator()(const std::shared_ptr<FunctionSymbolData> &lhs,
                    const std::shared_ptr<FunctionSymbolData> &rhs) const {
        return (uint32_t) lhs->getAddress() < (uint32_t) rhs->getAddress();
    }
};

class PluginInformation {
public:
    void addHookData(std::unique_ptr<HookData> hook_data) {
        hook_data_list.push_back(std::move(hook_data));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<HookData>> &getHookDataList() const {
        return hook_data_list;
    }

    void addFunctionData(std::unique_ptr<FunctionData> function_data) {
        function_data_list.push_back(std::move(function_data));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<FunctionData>> &getFunctionDataList() const {
        return function_data_list;
    }

    void addRelocationData(std::unique_ptr<RelocationData> relocation_data) {
        relocation_data_list.push_back(std::move(relocation_data));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<RelocationData>> &getRelocationDataList() const {
        return relocation_data_list;
    }


    void addFunctionSymbolData(std::shared_ptr<FunctionSymbolData> symbol_data) {
        symbol_data_list.insert(std::move(symbol_data));
    }

    [[nodiscard]] const std::set<std::shared_ptr<FunctionSymbolData>, FunctionSymbolDataComparator> &getFunctionSymbolDataList() const {
        return symbol_data_list;
    }

    void addSectionInfo(std::shared_ptr<SectionInfo> sectionInfo) {
        section_info_list[sectionInfo->getName()] = std::move(sectionInfo);
    }

    [[nodiscard]] const std::map<std::string, std::shared_ptr<SectionInfo>> &getSectionInfoList() const {
        return section_info_list;
    }

    [[nodiscard]] std::optional<std::shared_ptr<SectionInfo>> getSectionInfo(const std::string &sectionName) const {
        if (getSectionInfoList().contains(sectionName)) {
            return section_info_list.at(sectionName);
        }
        return std::nullopt;
    }

    void setTrampolineId(uint8_t _trampolineId) {
        this->trampolineId = _trampolineId;
    }

    [[nodiscard]] uint8_t getTrampolineId() const {
        return trampolineId;
    }

    [[nodiscard]] std::optional<std::shared_ptr<FunctionSymbolData>> getNearestFunctionSymbolData(uint32_t address) const {
        std::shared_ptr<FunctionSymbolData> result;

        bool foundHit = false;
        for (auto &cur : symbol_data_list) {
            if (foundHit && address < (uint32_t) cur->getAddress()) {
                break;
            }
            if (address >= (uint32_t) cur->getAddress()) {
                result   = cur;
                foundHit = true;
            }
        }
        if (result) {
            return result;
        }

        return {};
    }

    void *getTextMemoryAddress() {
        return allocatedTextMemoryAddress.get();
    }

    void *getDataMemoryAddress() {
        return allocatedDataMemoryAddress.get();
    }

private:
    std::vector<std::unique_ptr<HookData>> hook_data_list;
    std::vector<std::unique_ptr<FunctionData>> function_data_list;
    std::vector<std::unique_ptr<RelocationData>> relocation_data_list;
    std::set<std::shared_ptr<FunctionSymbolData>, FunctionSymbolDataComparator> symbol_data_list;
    std::map<std::string, std::shared_ptr<SectionInfo>> section_info_list;

    uint8_t trampolineId = 0;

    std::unique_ptr<uint8_t[]> allocatedTextMemoryAddress;
    std::unique_ptr<uint8_t[]> allocatedDataMemoryAddress;

    friend class PluginInformationFactory;
};
