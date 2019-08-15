#ifndef __COREINIT_H_
#define __COREINIT_H_


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define OSTimerClockSpeed ((248625000) / 4)

#define OSSecondsToTicks(val)        ((uint64_t)(val) * (uint64_t)OSTimerClockSpeed)
#define OSMillisecondsToTicks(val)  (((uint64_t)(val) * (uint64_t)OSTimerClockSpeed) / 1000ull)
#define OSMicrosecondsToTicks(val)  (((uint64_t)(val) * (uint64_t)OSTimerClockSpeed) / 1000000ull)

#define os_usleep(usecs)                OSSleepTicks(OSMicrosecondsToTicks(usecs))
#define os_sleep(secs)                  OSSleepTicks(OSSecondsToTicks(secs))

#define OSDynLoad_Acquire ((void (*)(char* rpl, unsigned int *handle))0x0102A3B4)
#define OSDynLoad_FindExport ((void (*)(unsigned int handle, int isdata, const char *symbol, void *address))0x0102B828)
#define OSFatal ((void (*)(char* msg))0x01031618)

#define EXPORT_DECL(res, func, ...)     res (* func)(__VA_ARGS__) __attribute__((section(".data"))) = 0;
#define EXPORT_VAR(type, var)           type var __attribute__((section(".data")));


#define EXPORT_FUNC_WRITE(func, val)    *(uint32_t*)(((uint32_t)&func) + 0) = (uint32_t)val

#define OS_FIND_EXPORT(handle, func)    _os_find_export(handle, # func, &funcPointer);                                  \
                                        EXPORT_FUNC_WRITE(func, funcPointer);

#define OS_FIND_EXPORT_EX(handle, func, func_p)                                                                         \
                                        _os_find_export(handle, # func, &funcPointer);                                  \
                                        EXPORT_FUNC_WRITE(func_p, funcPointer);

#define OS_MUTEX_SIZE                   44

extern void _os_find_export(uint32_t handle, const char *funcName, void *funcPointer);


typedef struct OSThread_ OSThread;

typedef struct OSContext_ {
    char tag[8];

    uint32_t gpr[32];

    uint32_t cr;
    uint32_t lr;
    uint32_t ctr;
    uint32_t xer;

    uint32_t srr0;
    uint32_t srr1;

    uint32_t ex0;
    uint32_t ex1;

    uint32_t exception_type;
    uint32_t reserved;

    double fpscr;
    double fpr[32];

    uint16_t spinLockCount;
    uint16_t state;

    uint32_t gqr[8];
    uint32_t pir;
    double psf[32];

    uint64_t coretime[3];
    uint64_t starttime;

    uint32_t error;
    uint32_t attributes;

    uint32_t pmc1;
    uint32_t pmc2;
    uint32_t pmc3;
    uint32_t pmc4;
    uint32_t mmcr0;
    uint32_t mmcr1;
} OSContext;

typedef int (*ThreadFunc)(int argc, void *argv);

typedef struct OSThreadLink_ {
    OSThread *next;
    OSThread *prev;
}  OSThreadLink;

struct OSThread_ {
    OSContext context;

    uint32_t txtTag;
    uint8_t state;
    uint8_t attr;

    short threadId;
    int suspend;
    int priority;

    char _[0x394 - 0x330 - sizeof(OSThreadLink)];
    OSThreadLink linkActive;

    void *stackBase;
    void *stackEnd;

    ThreadFunc entryPoint;

    char _3A0[0x6A0 - 0x3A0];
};

typedef struct OSThreadQueue_ {
    OSThread *head;
    OSThread *tail;
    void *parentStruct;
    uint32_t reserved;
} OSThreadQueue;

typedef struct OSMessage_ {
    uint32_t message;
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
} OSMessage;

typedef struct OSMessageQueue_ {
    uint32_t tag;
    char *name;
    uint32_t reserved;

    OSThreadQueue sendQueue;
    OSThreadQueue recvQueue;
    OSMessage *messages;
    int msgCount;
    int firstIndex;
    int usedCount;
} OSMessageQueue;

extern void (*DCInvalidateRange)(void *buffer, uint32_t length);
extern void (* DCFlushRange)(const void *addr, uint32_t length);
extern void (* DCStoreRange)(const void *addr, uint32_t length);
extern void (* ICInvalidateRange)(const void *addr, uint32_t length);
extern void* (* OSEffectiveToPhysical)(uint32_t);
extern void* (* OSSleepTicks)(uint64_t ticks);
extern void (* OSEnableHomeButtonMenu)(int32_t);


extern int32_t (* OSCreateThread)(OSThread *thread, int32_t (*callback)(int32_t, void*), int32_t argc, void *args, uint32_t stack, uint32_t stack_size, int32_t priority, uint32_t attr);
extern int32_t (* OSResumeThread)(OSThread *thread);
extern int32_t (* OSSuspendThread)(OSThread *thread);
extern int32_t (* OSIsThreadTerminated)(OSThread *thread);
extern int32_t (* OSIsThreadSuspended)(OSThread *thread);
extern int32_t (* OSJoinThread)(OSThread * thread, int32_t * ret_val);
extern int32_t (* OSSetThreadPriority)(OSThread * thread, int32_t priority);
extern void (* OSDetachThread)(OSThread * thread);
extern OSThread * (* OSGetCurrentThread)(void);
extern const char * (* OSGetThreadName)(OSThread * thread);

extern void (* OSGetActiveThreadLink)(OSThread * thread, void* link);
extern uint32_t (* OSGetThreadAffinity)(OSThread * thread);
extern int32_t (* OSGetThreadPriority)(OSThread * thread);
extern void (* OSSetThreadName)(OSThread * thread, const char *name);
extern int32_t (* OSGetCoreId)(void);
extern uint64_t (* OSGetTitleID)(void);

extern int32_t (* OSIsHomeButtonMenuEnabled)(void);

extern int32_t (* MEMCreateExpHeapEx)(void* heap, uint32_t size, uint16_t flags);
extern void* (*  MEMAllocFromExpHeapEx)( int32_t heap, uint32_t size, int alignment);
extern void  (* MEMFreeToExpHeap)( int32_t heap, void * block);


extern void(*OSScreenInit)();
extern unsigned int(*OSScreenGetBufferSizeEx)(unsigned int bufferNum);
extern unsigned int(*OSScreenSetBufferEx)(unsigned int bufferNum, void * addr);
extern unsigned int(*OSScreenFlipBuffersEx)(unsigned int x);
extern int32_t (*OSScreenEnableEx)(uint32_t bufferNum, int32_t enable);
extern int32_t (*OSScreenPutFontEx)(uint32_t bufferNum, uint32_t posX, uint32_t posY, const char * buffer);
extern unsigned int(*OSScreenClearBufferEx)(unsigned int bufferNum, unsigned int temp);

extern void (* OSInitMutex)(void* mutex);
extern void (* OSLockMutex)(void* mutex);
extern void (* OSUnlockMutex)(void* mutex);
extern int32_t (* OSTryLockMutex)(void* mutex);


extern uint32_t coreinit_handle;

extern uint32_t *pMEMAllocFromDefaultHeapEx;
extern uint32_t *pMEMAllocFromDefaultHeap;
extern uint32_t *pMEMFreeToDefaultHeap;

void InitAcquireOS();
void InitOSFunctionPointers(void);

#ifdef __cplusplus
}
#endif

#endif // __COREINIT_H_
