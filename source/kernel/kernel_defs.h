#ifndef __KERNEL_DEFS_H_
#define __KERNEL_DEFS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define KERN_SYSCALL_TBL1 0xFFE84C70 //Unknown
#define KERN_SYSCALL_TBL2 0xFFE85070 //Games
#define KERN_SYSCALL_TBL3 0xFFE85470 //Loader
#define KERN_SYSCALL_TBL4 0xFFEAAA60 //Home menu
#define KERN_SYSCALL_TBL5 0xFFEAAE60 //Browser

typedef struct _sr_table_t {
    uint32_t value[16];
    uint32_t sdr1;
} sr_table_t;

typedef struct _bat_t {
    uint32_t h;
    uint32_t l;
} bat_t;

typedef struct _bat_table_t {
    bat_t bat[8];
} bat_table_t;


#ifdef __cplusplus
}
#endif

#endif // __KERNEL_DEFS_H_
