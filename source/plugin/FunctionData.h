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

class FunctionData {

public:
    FunctionData(void *paddress,
                 void *vaddress,
                 std::string_view name,
                 function_replacement_library_type_t library,
                 void *replaceAddr,
                 void *replaceCall,
                 FunctionPatcherTargetProcess targetProcess);

    ~FunctionData();

    [[nodiscard]] const std::string &getName() const;

    [[nodiscard]] function_replacement_library_type_t getLibrary() const;

    [[nodiscard]] const void *getPhysicalAddress() const;

    [[nodiscard]] const void *getVirtualAddress() const;

    [[nodiscard]] const void *getReplaceAddress() const;

    [[nodiscard]] const void *getReplaceCall() const;

    [[nodiscard]] FunctionPatcherTargetProcess getTargetProcess() const;

    bool AddPatch();

    bool RemovePatch();

private:
    void *mPAddress = nullptr;
    void *mVAddress = nullptr;
    std::string mName;
    function_replacement_library_type_t mLibrary;
    FunctionPatcherTargetProcess mTargetProcess;
    void *mReplaceAddr = nullptr;
    void *mReplaceCall = nullptr;

    PatchedFunctionHandle mHandle = 0;
};
