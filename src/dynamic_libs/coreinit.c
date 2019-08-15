#include "coreinit.h"

uint32_t coreinit_handle __attribute__((section(".data"))) = 0;

EXPORT_DECL(void, DCInvalidateRange, void *buffer, uint32_t length);
EXPORT_DECL(void, DCFlushRange, const void *addr, uint32_t length);
EXPORT_DECL(void, DCStoreRange, const void *addr, uint32_t length);
EXPORT_DECL(void, ICInvalidateRange, const void *addr, uint32_t length);
EXPORT_DECL(void*, OSEffectiveToPhysical, uint32_t);
EXPORT_DECL(void*, OSSleepTicks, uint64_t);

EXPORT_DECL(int32_t, OSCreateThread, OSThread *thread, int32_t (*callback)(int32_t, void*), int32_t argc, void *args, uint32_t stack, uint32_t stack_size, int32_t priority, uint32_t attr);
EXPORT_DECL(int32_t, OSResumeThread, OSThread *thread);
EXPORT_DECL(int32_t, OSSuspendThread, OSThread *thread);
EXPORT_DECL(int32_t, OSIsThreadTerminated, OSThread *thread);
EXPORT_DECL(int32_t, OSIsThreadSuspended, OSThread *thread);
EXPORT_DECL(int32_t, OSSetThreadPriority, OSThread * thread, int32_t priority);
EXPORT_DECL(int32_t, OSJoinThread, OSThread * thread, int32_t * ret_val);
EXPORT_DECL(void, OSDetachThread, OSThread * thread);
EXPORT_DECL(OSThread *,OSGetCurrentThread,void);
EXPORT_DECL(const char *,OSGetThreadName,OSThread * thread);
EXPORT_DECL(void,OSGetActiveThreadLink,OSThread * thread, void* link);
EXPORT_DECL(uint32_t,OSGetThreadAffinity,OSThread * thread);
EXPORT_DECL(int32_t,OSGetThreadPriority,OSThread * thread);
EXPORT_DECL(void,OSSetThreadName,OSThread * thread, const char *name);
EXPORT_DECL(int32_t, OSGetCoreId, void);
EXPORT_DECL(int32_t, MEMCreateExpHeapEx, void *heap, uint32_t size, uint16_t flags);
EXPORT_DECL(void*, MEMAllocFromExpHeapEx, int32_t heap, uint32_t size, int alignment);
EXPORT_DECL(void, MEMFreeToExpHeap, int32_t heap, void * block);

EXPORT_DECL(void, OSEnableHomeButtonMenu, int32_t);

EXPORT_VAR(uint32_t *, pMEMAllocFromDefaultHeapEx);
EXPORT_VAR(uint32_t *, pMEMAllocFromDefaultHeap);
EXPORT_VAR(uint32_t *, pMEMFreeToDefaultHeap);

EXPORT_DECL(int32_t, OSIsHomeButtonMenuEnabled, void);
EXPORT_DECL(uint64_t, OSGetTitleID, void);

EXPORT_DECL(void,OSScreenInit,void);
EXPORT_DECL(unsigned int, OSScreenGetBufferSizeEx,unsigned int bufferNum);
EXPORT_DECL(unsigned int, OSScreenSetBufferEx,unsigned int bufferNum, void * addr);
EXPORT_DECL(unsigned int, OSScreenFlipBuffersEx,unsigned int x);
EXPORT_DECL(int32_t, OSScreenEnableEx,uint32_t bufferNum, int32_t enable);
EXPORT_DECL(int32_t, OSScreenPutFontEx,uint32_t bufferNum, uint32_t posX, uint32_t posY, const char * buffer);
EXPORT_DECL(unsigned int, OSScreenClearBufferEx,unsigned int bufferNum, unsigned int temp);

EXPORT_DECL(void, OSInitMutex, void* mutex);
EXPORT_DECL(void, OSLockMutex, void* mutex);
EXPORT_DECL(void, OSUnlockMutex, void* mutex);
EXPORT_DECL(int32_t, OSTryLockMutex, void* mutex);

void _os_find_export(uint32_t handle, const char *funcName, void *funcPointer) {
    OSDynLoad_FindExport(handle, 0, funcName, funcPointer);

    if(!*(uint32_t *)funcPointer) {
        /*
         * This is effectively OSFatal("Function %s is NULL", funcName),
         * but we can't rely on any library functions like snprintf or
         * strcpy at this point.
         *
         * Buffer bounds are not checked. Beware!
         */
        char buf[256], *bufp = buf;
        const char a[] = "Function ", b[] = " is NULL", *p;
        unsigned int i;

        for (i = 0; i < sizeof(a) - 1; i++)
            *bufp++ = a[i];

        for (p = funcName; *p; p++)
            *bufp++ = *p;

        for (i = 0; i < sizeof(b) - 1; i++)
            *bufp++ = b[i];

        *bufp++ = '\0';

        OSFatal(buf);
    }
}


void InitAcquireOS() {
    OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
}
void InitOSFunctionPointers(void) {

    if(coreinit_handle != 0){
        return;
    }

    uint32_t funcPointer = 0;

    InitAcquireOS();

    OS_FIND_EXPORT(coreinit_handle, OSInitMutex);
    OS_FIND_EXPORT(coreinit_handle, OSLockMutex);
    OS_FIND_EXPORT(coreinit_handle, OSUnlockMutex);
    OS_FIND_EXPORT(coreinit_handle, OSTryLockMutex);

    OS_FIND_EXPORT(coreinit_handle, OSScreenInit);
    OS_FIND_EXPORT(coreinit_handle, OSScreenGetBufferSizeEx);
    OS_FIND_EXPORT(coreinit_handle, OSScreenSetBufferEx);
    OS_FIND_EXPORT(coreinit_handle, OSScreenFlipBuffersEx);
    OS_FIND_EXPORT(coreinit_handle, OSScreenEnableEx);
    OS_FIND_EXPORT(coreinit_handle, OSScreenPutFontEx);
    OS_FIND_EXPORT(coreinit_handle, OSScreenClearBufferEx);
    OS_FIND_EXPORT(coreinit_handle, OSEnableHomeButtonMenu);

    OS_FIND_EXPORT(coreinit_handle, OSIsHomeButtonMenuEnabled);
    OS_FIND_EXPORT(coreinit_handle, DCInvalidateRange);
    OS_FIND_EXPORT(coreinit_handle, DCFlushRange);
    OS_FIND_EXPORT(coreinit_handle, DCStoreRange);
    OS_FIND_EXPORT(coreinit_handle, ICInvalidateRange);
    OS_FIND_EXPORT(coreinit_handle, OSEffectiveToPhysical);
    OS_FIND_EXPORT(coreinit_handle, OSSleepTicks);
    OS_FIND_EXPORT(coreinit_handle, MEMCreateExpHeapEx);
    OS_FIND_EXPORT(coreinit_handle, MEMAllocFromExpHeapEx);
    OS_FIND_EXPORT(coreinit_handle, MEMFreeToExpHeap);
    OS_FIND_EXPORT(coreinit_handle, OSGetTitleID);

    OS_FIND_EXPORT(coreinit_handle, OSCreateThread);
    OS_FIND_EXPORT(coreinit_handle, OSResumeThread);
    OS_FIND_EXPORT(coreinit_handle, OSSuspendThread);
    OS_FIND_EXPORT(coreinit_handle, OSIsThreadTerminated);
    OS_FIND_EXPORT(coreinit_handle, OSIsThreadSuspended);
    OS_FIND_EXPORT(coreinit_handle, OSJoinThread);
    OS_FIND_EXPORT(coreinit_handle, OSSetThreadPriority);
    OS_FIND_EXPORT(coreinit_handle, OSDetachThread);
    OS_FIND_EXPORT(coreinit_handle, OSGetCurrentThread);
    OS_FIND_EXPORT(coreinit_handle, OSGetThreadName);
    OS_FIND_EXPORT(coreinit_handle, OSGetActiveThreadLink);
    OS_FIND_EXPORT(coreinit_handle, OSGetThreadAffinity);
    OS_FIND_EXPORT(coreinit_handle, OSGetThreadPriority);
    OS_FIND_EXPORT(coreinit_handle, OSSetThreadName);
    OS_FIND_EXPORT(coreinit_handle, OSGetCoreId);

    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &pMEMAllocFromDefaultHeapEx);
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeap", &pMEMAllocFromDefaultHeap);
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &pMEMFreeToDefaultHeap);
}
