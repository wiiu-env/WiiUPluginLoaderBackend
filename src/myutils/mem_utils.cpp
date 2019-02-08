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
#include <utils/logger.h>
#include <wups.h>
#include <stdarg.h>
#include "dynamic_libs/os_functions.h"
#include "mem_utils.h"
#include "mymemory/memory_mapping.h"


int32_t memHandle __attribute__((section(".data"))) = -1;

void MemoryUtils::init(){
    memHandle = MEMCreateExpHeapEx((void*)MemoryMapping::getVideoMemoryAddress(), MemoryMapping::getVideoMemorySize(), 0);
}

void* MemoryUtils::alloc(uint32_t size, int32_t align){
    return MEMAllocFromExpHeapEx(memHandle,size, align);
}

void MemoryUtils::free(void * ptr){
   MEMFreeToExpHeap(memHandle,ptr);
}
