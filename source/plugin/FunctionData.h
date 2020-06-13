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

#include <wups.h>
#include <string>
#include <function_patcher/fpatching_defines.h>

class FunctionData {

public:
    FunctionData(void *paddress, void *vaddress, const std::string &name, function_replacement_library_type_t library, void *replaceAddr, void *replaceCall) {
        this->paddress = paddress;
        this->vaddress = vaddress;
        this->name = name;
        this->library = library;
        this->replaceAddr = replaceAddr;
        this->replaceCall = replaceCall;
    }

    ~FunctionData() {

    }

    const std::string &getName() const {
        return this->name;
    }

    function_replacement_library_type_t getLibrary() const {
        return this->library;
    }

    const void *getPhysicalAddress() const {
        return paddress;
    }

    const void *getVirtualAddress() const {
        return vaddress;
    }

    const void *getReplaceAddress() const {
        return replaceAddr;
    }

    const void *getReplaceCall() const {
        return replaceCall;
    }

private:
    void *paddress = NULL;
    void *vaddress = NULL;
    std::string name;
    function_replacement_library_type_t library;
    void *replaceAddr = NULL;
    void *replaceCall = NULL;
};

