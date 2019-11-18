#include "kernel_utils.h"
#include "kernel_defs.h"
#include "dynamic_libs/coreinit.h"

#define KERN_SYSCALL_TBL1 0xFFE84C70 //Unknown
#define KERN_SYSCALL_TBL2 0xFFE85070 //Games
#define KERN_SYSCALL_TBL3 0xFFE85470 //Loader
#define KERN_SYSCALL_TBL4 0xFFEAAA60 //Home menu
#define KERN_SYSCALL_TBL5 0xFFEAAE60 //Browser

extern void SCKernelCopyData(uint32_t dst, uint32_t src, uint32_t len);

static void KernelReadSRs(sr_table_t * table) {
    uint32_t i = 0;

    // calculate PT_size ((end-start)*8/4096)*4 or (end-start)/128
    // Minimum page table size is 64Kbytes.

    asm volatile("eieio; isync");

    asm volatile("mfspr %0, 25" : "=r" (table->sdr1));

    asm volatile("mfsr %0, 0" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 1" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 2" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 3" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 4" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 5" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 6" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 7" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 8" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 9" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 10" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 11" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 12" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 13" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 14" : "=r" (table->value[i]));
    i++;
    asm volatile("mfsr %0, 15" : "=r" (table->value[i]));
    i++;

    asm volatile("eieio; isync");
}

static void KernelWriteSRs(sr_table_t * table) {
    uint32_t i = 0;

    asm volatile("eieio; isync");

    // Writing didn't work for all at once so we only write number 8.
    // TODO: fix this and change it if required.

    /*asm volatile("mtsr 0, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 1, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 2, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 3, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 4, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 5, %0" : : "r" (table->value[i])); i++;*/
    //asm volatile("mtsr 6, %0" : : "r" (table->value[6])); i++;
    /*asm volatile("mtsr 7, %0" : : "r" (table->value[i])); i++;*/
    asm volatile("mtsr 8, %0" : : "r" (table->value[8]));
    //i++;
    /*asm volatile("mtsr 9, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 10, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 11, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 12, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 13, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 14, %0" : : "r" (table->value[i])); i++;
    asm volatile("mtsr 15, %0" : : "r" (table->value[i])); i++;*/


    asm volatile("isync");
}

void KernelWrite(uint32_t addr, const void *data, uint32_t length) {
    uint32_t dst = (uint32_t) OSEffectiveToPhysical(addr);
    uint32_t src = (uint32_t) OSEffectiveToPhysical((uint32_t)data);
    KernelCopyData(dst, src, length);
    DCFlushRange((void *)addr, length);
    ICInvalidateRange((void *)addr, length);
}

void KernelWriteU32(uint32_t addr, uint32_t value) {
    uint32_t dst = (uint32_t) OSEffectiveToPhysical(addr);
    uint32_t src = (uint32_t) OSEffectiveToPhysical((uint32_t)&value);
    KernelCopyData(dst, src, 4);
    DCFlushRange((void *)addr, 4);
    ICInvalidateRange((void *)addr, 4);
}

/* Write a 32-bit word with kernel permissions */
void __attribute__ ((noinline)) kern_write(void * addr, uint32_t value) {
    asm volatile (
        "li 3,1\n"
        "li 4,0\n"
        "mr 5,%1\n"
        "li 6,0\n"
        "li 7,0\n"
        "lis 8,1\n"
        "mr 9,%0\n"
        "mr %1,1\n"
        "li 0,0x3500\n"
        "sc\n"
        "nop\n"
        "mr 1,%1\n"
        :
        :	"r"(addr), "r"(value)
        :	"memory", "ctr", "lr", "0", "3", "4", "5", "6", "7", "8", "9", "10",
        "11", "12"
    );
}

/* Read a 32-bit word with kernel permissions */
uint32_t __attribute__ ((noinline)) kern_read(const void *addr)
{
	uint32_t result;
	asm volatile (
		"li 3,1\n"
		"li 4,0\n"
		"li 5,0\n"
		"li 6,0\n"
		"li 7,0\n"
		"lis 8,1\n"
		"mr 9,%1\n"
		"li 0,0x3400\n"
		"mr %0,1\n"
		"sc\n"
		"nop\n"
		"mr 1,%0\n"
		"mr %0,3\n"
		:	"=r"(result)
		:	"b"(addr)
		:	"memory", "ctr", "lr", "0", "3", "4", "5", "6", "7", "8", "9", "10",
			"11", "12"
	);

	return result;
}

void PatchSyscall(int index, uint32_t addr) {
    //DEBUG_FUNCTION_LINE("Patching Syscall 0x%02X\n",index);
    kern_write((void *) (KERN_SYSCALL_TBL1 + index * 4), addr);
    kern_write((void *) (KERN_SYSCALL_TBL2 + index * 4), addr);
    kern_write((void *) (KERN_SYSCALL_TBL3 + index * 4), addr);
    kern_write((void *) (KERN_SYSCALL_TBL4 + index * 4), addr);
    kern_write((void *) (KERN_SYSCALL_TBL5 + index * 4), addr);
}

void KernelReadPTE(uint32_t outputAddr, int32_t length) {
    uint32_t dst = (uint32_t) OSEffectiveToPhysical(outputAddr);
    uint32_t src = 0xFFE20000;
    ICInvalidateRange(&dst, 4);
    DCFlushRange(&dst, 4);
    DCFlushRange(&src, 4);
    KernelCopyData(dst, src, length);
    DCFlushRange((void *)outputAddr, length);
    ICInvalidateRange((void *)outputAddr, length);
}

void KernelWritePTE(uint32_t inputAddr, int32_t length) {
    uint32_t dst = 0xFFE20000;
    uint32_t src = (uint32_t) OSEffectiveToPhysical(inputAddr);
    ICInvalidateRange(&src, 4);
    DCFlushRange(&src, 4);
    KernelCopyData(dst, src, length);
}

static void NOPAtPhysicalAddress(uint32_t addr) {
    uint32_t dst = 0x60000000;
    ICInvalidateRange(&dst, 4);
    DCFlushRange(&dst, 4);
    KernelCopyData(addr,(uint32_t)OSEffectiveToPhysical((uint32_t)&dst),4);
}

void kernelInitialize() {
    static uint8_t ucSyscallsSetupRequired = 1;
    if(!ucSyscallsSetupRequired)
        return;

    ucSyscallsSetupRequired = 0;

    PatchSyscall(0x25, (uint32_t)SCKernelCopyData);
    PatchSyscall(0x36, (uint32_t)KernelReadSRs);
    PatchSyscall(0x0A, (uint32_t)KernelWriteSRs);

    // Override all writes to SR8 with nops.
    NOPAtPhysicalAddress(0xFFF1D754);
    NOPAtPhysicalAddress(0xFFF1D64C);
    NOPAtPhysicalAddress(0xFFE00638);
}
