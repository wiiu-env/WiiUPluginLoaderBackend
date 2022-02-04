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

#include <function_patcher/fpatching_defines.h>
#include <string>
#include <wups.h>

class FunctionData {

public:
    FunctionData(void *paddress, void *vaddress, const std::string &name, function_replacement_library_type_t library, void *replaceAddr, void *replaceCall,
                 FunctionPatcherTargetProcess targetProcess) {
        this->paddress      = paddress;
        this->vaddress      = vaddress;
        this->name          = name;
        this->library       = library;
        this->targetProcess = targetProcess;
        this->replaceAddr   = replaceAddr;
        this->replaceCall   = replaceCall;
    }

    ~FunctionData() = default;

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

private:
    void *paddress = nullptr;
    void *vaddress = nullptr;
    std::string name;
    function_replacement_library_type_t library;
    FunctionPatcherTargetProcess targetProcess;
    void *replaceAddr = nullptr;
    void *replaceCall = nullptr;
};
