/****************************************************************************
 * Copyright (C) 2018-2020 Maschell
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

#include "utils/logger.h"
#include <function_patcher/fpatching_defines.h>
#include <function_patcher/function_patching.h>
#include <string>

class FunctionData {

public:
    FunctionData(void *paddress, void *vaddress, std::string_view name, function_replacement_library_type_t library, void *replaceAddr, void *replaceCall,
                 FunctionPatcherTargetProcess targetProcess) {
        this->paddress      = paddress;
        this->vaddress      = vaddress;
        this->name          = name;
        this->library       = library;
        this->targetProcess = targetProcess;
        this->replaceAddr   = replaceAddr;
        this->replaceCall   = replaceCall;
    }

    ~FunctionData() {
        if (handle != 0) {
            DEBUG_FUNCTION_LINE_WARN("Destroying FunctionData while it was still patched. This should never happen.");
            RemovePatch();
        }
    }

    [[nodiscard]] const std::string &getName() const {
        return this->name;
    }

    [[nodiscard]] function_replacement_library_type_t getLibrary() const {
        return this->library;
    }

    [[nodiscard]] const void *getPhysicalAddress() const {
        return paddress;
    }

    [[nodiscard]] const void *getVirtualAddress() const {
        return vaddress;
    }

    [[nodiscard]] const void *getReplaceAddress() const {
        return replaceAddr;
    }

    [[nodiscard]] const void *getReplaceCall() const {
        return replaceCall;
    }

    [[nodiscard]] FunctionPatcherTargetProcess getTargetProcess() const {
        return targetProcess;
    }

    bool AddPatch() {
        if (handle == 0) {
            function_replacement_data_t functionData = {
                    .version       = FUNCTION_REPLACEMENT_DATA_STRUCT_VERSION,
                    .type          = FUNCTION_PATCHER_REPLACE_BY_LIB_OR_ADDRESS,
                    .physicalAddr  = reinterpret_cast<uint32_t>(this->paddress),
                    .virtualAddr   = reinterpret_cast<uint32_t>(this->vaddress),
                    .replaceAddr   = reinterpret_cast<uint32_t>(this->replaceAddr),
                    .replaceCall   = static_cast<uint32_t *>(this->replaceCall),
                    .targetProcess = this->targetProcess,
                    .ReplaceInRPL  = {
                             .function_name = this->name.c_str(),
                             .library       = this->library,
                    }};

            if (FunctionPatcher_AddFunctionPatch(&functionData, &handle, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to add patch for function (\"%s\" PA:%08X VA:%08X)", this->name.c_str(), this->paddress, this->vaddress);
                return false;
            }
        } else {
            DEBUG_FUNCTION_LINE("Function patch has already been added.");
        }
        return true;
    }

    bool RemovePatch() {
        if (handle != 0) {
            if (FunctionPatcher_RemoveFunctionPatch(handle) != FUNCTION_PATCHER_RESULT_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to remove patch for function");
                return false;
            }
            handle = 0;
        } else {
            DEBUG_FUNCTION_LINE_VERBOSE("Was not patched.");
        }

        return true;
    }

private:
    void *paddress = nullptr;
    void *vaddress = nullptr;
    std::string name;
    function_replacement_library_type_t library;
    FunctionPatcherTargetProcess targetProcess;
    void *replaceAddr = nullptr;
    void *replaceCall = nullptr;

    PatchedFunctionHandle handle = 0;
};
