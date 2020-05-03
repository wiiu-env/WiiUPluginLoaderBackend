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

#include <map>
#include <optional>
#include <string>
#include <vector>
#include "PluginMetaInformation.h"
#include "RelocationData.h"
#include "HookData.h"
#include "FunctionData.h"
#include "SectionInfo.h"

class PluginInformation {
public:
    PluginInformation(){
    }

    virtual ~PluginInformation() {
    }

    void addHookData(const HookData& hook_data) {
        hook_data_list.push_back(hook_data);
    }

    const std::vector<HookData>& getHookDataList() const {
        return hook_data_list;
    }

    void addFunctionData(const FunctionData &function_data) {
        function_data_list.push_back(function_data);
    }

    const std::vector<FunctionData>& getFunctionDataList() const {
        return function_data_list;
    }

    void addRelocationData(const RelocationData &relocation_data) {
        relocation_data_list.push_back(relocation_data);
    }

    const std::vector<RelocationData>& getRelocationDataList() const {
        return relocation_data_list;
    }

    void addSectionInfo(const SectionInfo& sectionInfo) {
        section_info_list[sectionInfo.getName()] = sectionInfo;
    }

    const std::map<std::string, SectionInfo>& getSectionInfoList() const {
        return section_info_list;
    }

    std::optional<SectionInfo> getSectionInfo(const std::string& sectionName) const {
        if(getSectionInfoList().count(sectionName) > 0) {
            return section_info_list.at(sectionName);
        }
        return std::nullopt;
    }

    void setTrampolinId(uint8_t trampolinId) {
        this->trampolinId = trampolinId;
    }

    uint8_t getTrampolinId() const {
        return trampolinId;
    }

private:

    std::vector<HookData> hook_data_list;
    std::vector<FunctionData> function_data_list;
    std::vector<RelocationData> relocation_data_list;
    std::map<std::string, SectionInfo> section_info_list;

    uint8_t trampolinId = 0;

    void* allocatedTextMemoryAddress = 0;
    void* allocatedDataMemoryAddress = 0;

    friend class PluginInformationFactory;
    friend class PluginContainerPersistence;
};
