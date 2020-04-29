#ifndef __KERNEL_UTILS_H_
#define __KERNEL_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kernel_defs.h"

extern void KernelCopyData(uint32_t dst, uint32_t src, uint32_t len);

void kern_write(void * addr, uint32_t value);

uint32_t kern_read(const void *addr);

void SC0x0A_KernelWriteSRs(sr_table_t * table);
void SC0x36_KernelReadSRs(sr_table_t * table);

void KernelReadPTE(uint32_t addr, int32_t length);
void KernelWritePTE(uint32_t addr, int32_t length);

void KernelWrite(uint32_t addr, const void *data, uint32_t length);
void KernelWriteU32(uint32_t addr, uint32_t value);
void kernelInitialize();

#ifdef __cplusplus
}
#endif

#endif // __KERNEL_UTILS_H_
