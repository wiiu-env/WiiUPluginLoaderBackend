/****************************************************************************
 * Copyright (C) 2015 Dimok
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
#include <malloc.h>
#include <string.h>
#include "dynamic_libs/coreinit.h"
#include "memory_mapping.h"
#include "memory.h"

//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//! Memory functions
//! This is the only place where those are needed so lets keep them more or less private
//!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
extern uint32_t * pMEMAllocFromDefaultHeapEx;
extern uint32_t * pMEMAllocFromDefaultHeap;
extern uint32_t * pMEMFreeToDefaultHeap;

int32_t memHandle __attribute__((section(".data"))) = -1;

void initMemory() {
    memHandle = MEMCreateExpHeapEx((void*)MemoryMapping::getHeapAddress(), MemoryMapping::getHeapSize(), 0);
}



//!-------------------------------------------------------------------------------------------
//! wraps
//!-------------------------------------------------------------------------------------------
extern "C" void *__wrap_malloc(size_t size) {
    if(memHandle == -1) {
        return ((void * (*)(size_t))(*pMEMAllocFromDefaultHeap))(size);
    }
    // pointer to a function resolve
    return MEMAllocFromExpHeapEx(memHandle, size, 0x40);
}

extern "C" void *__wrap_memalign(size_t align, size_t size) {
    if(memHandle == -1) {
        return ((void * (*)(size_t, size_t))(*pMEMAllocFromDefaultHeapEx))(size, align);
    }
    if (align < 4)
        align = 4;

    // pointer to a function resolve
    return MEMAllocFromExpHeapEx(memHandle, size, align);
}

extern "C" void __wrap_free(void *p) {
    if(memHandle == -1) {
        //OSFatal("__wrap_free");
        if(p != 0) {
            ((void (*)(void *))(*pMEMFreeToDefaultHeap))(p);
        }
        return;
    }
    // pointer to a function resolve
    if(p != 0) {
        MEMFreeToExpHeap(memHandle,p);
    }
}

extern "C" void *__wrap_calloc(size_t n, size_t size) {
    void *p = __wrap_malloc(n * size);
    if (p != 0) {
        memset(p, 0, n * size);
    }
    return p;
}

extern "C" size_t __wrap_malloc_usable_size(void *p) {
    //! TODO: this is totally wrong and needs to be addressed
    return 0x7FFFFFFF;
}

extern "C" void *__wrap_realloc(void *ptr, size_t size) {
    void *newPtr;

    if (!ptr) {
        newPtr = __wrap_malloc(size);
        if (!newPtr) {
            goto error;
        }
    } else {
        newPtr = __wrap_malloc(size);
        if (!newPtr) {
            goto error;
        }

        memcpy(newPtr, ptr, size);

        __wrap_free(ptr);
    }

    return newPtr;
error:
    return NULL;
}

//!-------------------------------------------------------------------------------------------
//! reent versions
//!-------------------------------------------------------------------------------------------
extern "C" void *__wrap__malloc_r(struct _reent *r, size_t size) {
    return __wrap_malloc(size);
}

extern "C" void *__wrap__calloc_r(struct _reent *r, size_t n, size_t size) {
    return __wrap_calloc(n, size);
}

extern "C" void *__wrap__memalign_r(struct _reent *r, size_t align, size_t size) {
    return __wrap_memalign(align, size);
}

extern "C" void __wrap__free_r(struct _reent *r, void *p) {
    __wrap_free(p);
}

extern "C" size_t __wrap__malloc_usable_size_r(struct _reent *r, void *p) {
    return __wrap_malloc_usable_size(p);
}

extern "C" void *__wrap__realloc_r(struct _reent *r, void *p, size_t size) {
    return __wrap_realloc(p, size);
}
