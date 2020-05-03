/****************************************************************************
 * Copyright (C) 2016 Maschell
 * With code from chadderz and dimok
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

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <coreinit/memorymap.h>
#include <coreinit/cache.h>
#include <coreinit/dynload.h>

#include "kernel/kernel_utils.h"
#include "function_patcher.h"
#include "logger.h"

#define LIB_CODE_RW_BASE_OFFSET                         0xC1000000
#define CODE_RW_BASE_OFFSET                             0x00000000
#define DEBUG_LOG_DYN                                   0

OSDynLoad_Module acp_handle_internal = 0;
OSDynLoad_Module aoc_handle_internal = 0;
OSDynLoad_Module sound_handle_internal = 0;
OSDynLoad_Module sound_handle_internal_old = 0;
OSDynLoad_Module libcurl_handle_internal = 0;
OSDynLoad_Module gx2_handle_internal = 0;
OSDynLoad_Module nfp_handle_internal = 0;
OSDynLoad_Module nn_act_handle_internal = 0;
OSDynLoad_Module nn_nim_handle_internal = 0;
OSDynLoad_Module nn_save_handle_internal = 0;
OSDynLoad_Module ntag_handle_internal = 0;
OSDynLoad_Module coreinit_handle_internal = 0;
OSDynLoad_Module padscore_handle_internal = 0;
OSDynLoad_Module proc_ui_handle_internal = 0;
OSDynLoad_Module nsysnet_handle_internal = 0;
OSDynLoad_Module sysapp_handle_internal = 0;
OSDynLoad_Module syshid_handle_internal = 0;
OSDynLoad_Module vpad_handle_internal = 0;
OSDynLoad_Module vpadbase_handle_internal = 0;

/*
* Patches a function that is loaded at the start of each application. Its not required to restore, at least when they are really dynamic.
* "normal" functions should be patch with the normal patcher. Current Code by Maschell with the help of dimok. Orignal code by Chadderz.
*/
void PatchInvidualMethodHooks(hooks_magic_t method_hooks[], int32_t hook_information_size, volatile uint32_t dynamic_method_calls[]) {
    resetLibs();

    DEBUG_FUNCTION_LINE("Patching %d given functions", hook_information_size);
    /* Patch branches to it.  */
    volatile uint32_t *space = &dynamic_method_calls[0];

    int32_t method_hooks_count = hook_information_size;

    uint32_t skip_instr = 1;
    uint32_t my_instr_len = 6;
    uint32_t instr_len = my_instr_len + skip_instr;
    uint32_t flush_len = 4 * instr_len;
    for (int32_t i = 0; i < method_hooks_count; i++) {
        log_printf("Patching %s ...", method_hooks[i].functionName);
        if (method_hooks[i].functionType == STATIC_FUNCTION && method_hooks[i].alreadyPatched == 1) {
            if (isDynamicFunction((uint32_t) OSEffectiveToPhysical(method_hooks[i].realAddr))) {
                log_printf("The function %s is a dynamic function. Please fix that <3", method_hooks[i].functionName);
                method_hooks[i].functionType = DYNAMIC_FUNCTION;
            } else {
                log_printf("Skipping %s, its already patched", method_hooks[i].functionName);
                space += instr_len;
                continue;
            }
        }

        uint32_t physical = 0;
        uint32_t repl_addr = (uint32_t) method_hooks[i].replaceAddr;
        uint32_t call_addr = (uint32_t) method_hooks[i].replaceCall;

        uint32_t real_addr = GetAddressOfFunction(method_hooks[i].functionName, method_hooks[i].library);

        if (!real_addr) {
            log_printf("");
            DEBUG_FUNCTION_LINE("OSDynLoad_FindExport failed for %s", method_hooks[i].functionName);
            space += instr_len;
            continue;
        }

        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("%s is located at %08X!", method_hooks[i].functionName, real_addr);
        }

        physical = (uint32_t) OSEffectiveToPhysical(real_addr);
        if (!physical) {
            log_printf("Error. Something is wrong with the physical address");
            space += instr_len;
            continue;
        }

        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("%s physical is located at %08X!", method_hooks[i].functionName, physical);
        }

        *(volatile uint32_t *) (call_addr) = (uint32_t) (space) - CODE_RW_BASE_OFFSET;


        uint32_t targetAddr = (uint32_t) space;
        if (targetAddr < 0x00800000 || targetAddr >= 0x01000000) {
            targetAddr = (uint32_t) OSEffectiveToPhysical(targetAddr);
        } else {
            targetAddr = targetAddr + 0x30800000 - 0x00800000;
        }

        KernelCopyData(targetAddr, physical, 4);

        ICInvalidateRange((void *) (space), 4);
        DCFlushRange((void *) (space), 4);
        space++;

        //Only works if skip_instr == 1
        if (skip_instr == 1) {
            // fill the restore instruction section
            method_hooks[i].realAddr = real_addr;
            method_hooks[i].restoreInstruction = *(space - 1);
            if (DEBUG_LOG_DYN) {
                DEBUG_FUNCTION_LINE("method_hooks[i].realAddr = %08X!", method_hooks[i].realAddr);
            }
            if (DEBUG_LOG_DYN) {
                DEBUG_FUNCTION_LINE("method_hooks[i].restoreInstruction = %08X!", method_hooks[i].restoreInstruction);
            }
        } else {
            log_printf("Error. Can't save %s for restoring!", method_hooks[i].functionName);
        }

        //adding jump to real function thx @ dimok for the assembler code
        /*
            90 61 ff e0     stw     r3,-32(r1)
            3c 60 12 34     lis     r3,4660
            60 63 56 78     ori     r3,r3,22136
            7c 69 03 a6     mtctr   r3
            80 61 ff e0     lwz     r3,-32(r1)
            4e 80 04 20     bctr*/
        *space = 0x9061FFE0;
        space++;
        *space = 0x3C600000 | (((real_addr + (skip_instr * 4)) >> 16) & 0x0000FFFF); // lis r3, real_addr@h
        space++;
        *space = 0x60630000 | ((real_addr + (skip_instr * 4)) & 0x0000ffff); // ori r3, r3, real_addr@l
        space++;
        *space = 0x7C6903A6; // mtctr   r3
        space++;
        *space = 0x8061FFE0; // lwz     r3,-32(r1)
        space++;
        *space = 0x4E800420; // bctr
        space++;
        DCFlushRange((void *) (space - instr_len), flush_len);
        ICInvalidateRange((unsigned char *) (space - instr_len), flush_len);

        //setting jump back
        uint32_t replace_instr = 0x48000002 | (repl_addr & 0x03fffffc);
        DCFlushRange(&replace_instr, 4);

        KernelCopyData(physical, (uint32_t) OSEffectiveToPhysical((uint32_t) &replace_instr), 4);
        ICInvalidateRange((void *) (real_addr), 4);

        method_hooks[i].alreadyPatched = 1;
        log_printf("done!\n");

    }
    DEBUG_FUNCTION_LINE("Done with patching given functions!");
}

/* ****************************************************************** */
/*                  RESTORE ORIGINAL INSTRUCTIONS                     */
/* ****************************************************************** */
void RestoreInvidualInstructions(hooks_magic_t method_hooks[], int32_t hook_information_size) {
    resetLibs();
    DEBUG_FUNCTION_LINE("Restoring given functions!");
    int32_t method_hooks_count = hook_information_size;
    for (int32_t i = 0; i < method_hooks_count; i++) {
        DEBUG_FUNCTION_LINE("Restoring %s... ", method_hooks[i].functionName);
        if (method_hooks[i].restoreInstruction == 0 || method_hooks[i].realAddr == 0) {
            log_printf("I dont have the information for the restore =( skip");
            continue;
        }

        uint32_t real_addr = GetAddressOfFunction(method_hooks[i].functionName, method_hooks[i].library);

        if (!real_addr) {
            log_printf("OSDynLoad_FindExport failed for %s", method_hooks[i].functionName);
            continue;
        }

        uint32_t physical = (uint32_t) OSEffectiveToPhysical(real_addr);
        if (!physical) {
            log_printf("Something is wrong with the physical address");
            continue;
        }

        if (isDynamicFunction(physical)) {
            log_printf("Its a dynamic function. We don't need to restore it!", method_hooks[i].functionName);
        } else {
            physical = (uint32_t) OSEffectiveToPhysical(method_hooks[i].realAddr); //When its an static function, we need to use the old location
            if (DEBUG_LOG_DYN) {
                DEBUG_FUNCTION_LINE("Restoring %08X to %08X", (uint32_t) method_hooks[i].restoreInstruction, physical);
            }
            uint32_t targetAddr = (uint32_t) &method_hooks[i].restoreInstruction;
            if (targetAddr < 0x00800000 || targetAddr >= 0x01000000) {
                targetAddr = (uint32_t) OSEffectiveToPhysical(targetAddr);
            } else {
                targetAddr = targetAddr + 0x30800000 - 0x00800000;
            }

            KernelCopyData(physical, targetAddr, 4);
            if (DEBUG_LOG_DYN) {
                DEBUG_FUNCTION_LINE("ICInvalidateRange %08X", (void *) method_hooks[i].realAddr);
            }
            ICInvalidateRange((void *) method_hooks[i].realAddr, 4);
            log_printf("done");
        }
        method_hooks[i].alreadyPatched = 0; // In case a
    }

    DEBUG_FUNCTION_LINE("Done with restoring given functions!");
}

int32_t isDynamicFunction(uint32_t physicalAddress) {
    if ((physicalAddress & 0x80000000) == 0x80000000) {
        return 1;
    }
    return 0;
}

uint32_t GetAddressOfFunction(const char *functionName, uint32_t library) {
    uint32_t real_addr = 0;

    /*
    if(strcmp(functionName, "OSDynLoad_Acquire") == 0) {
        memcpy(&real_addr, &OSDynLoad_Acquire, 4);
        return real_addr;
    } else if(strcmp(functionName, "LiWaitOneChunk") == 0) {
        real_addr = (uint32_t)addr_LiWaitOneChunk;
        return real_addr;
    } else if(strcmp(functionName, "LiBounceOneChunk") == 0) {
        //! not required on firmwares above 3.1.0
        if(OS_FIRMWARE >= 400)
            return 0;

        uint32_t addr_LiBounceOneChunk = 0x010003A0;
        real_addr = (uint32_t)addr_LiBounceOneChunk;
        return real_addr;
    }
    */

    OSDynLoad_Module rpl_handle = 0;
    if (library == LIB_CORE_INIT) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_CORE_INIT", functionName);
        }
        if (coreinit_handle_internal == 0) {
            OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle_internal);
        }
        if (coreinit_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_CORE_INIT failed to acquire");
            return 0;
        }
        rpl_handle = coreinit_handle_internal;
    } else if (library == LIB_NSYSNET) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_NSYSNET", functionName);
        }
        if (nsysnet_handle_internal == 0) {
            OSDynLoad_Acquire("nsysnet.rpl", &nsysnet_handle_internal);
        }
        if (nsysnet_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_NSYSNET failed to acquire");
            return 0;
        }
        rpl_handle = nsysnet_handle_internal;
    } else if (library == LIB_GX2) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_GX2", functionName);
        }
        if (gx2_handle_internal == 0) {
            OSDynLoad_Acquire("gx2.rpl", &gx2_handle_internal);
        }
        if (gx2_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_GX2 failed to acquire");
            return 0;
        }
        rpl_handle = gx2_handle_internal;
    } else if (library == LIB_AOC) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_AOC", functionName);
        }
        if (aoc_handle_internal == 0) {
            OSDynLoad_Acquire("nn_aoc.rpl", &aoc_handle_internal);
        }
        if (aoc_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_AOC failed to acquire");
            return 0;
        }
        rpl_handle = aoc_handle_internal;
    } else if (library == LIB_AX) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_AX", functionName);
        }
        if (sound_handle_internal == 0) {
            OSDynLoad_Acquire("sndcore2.rpl", &sound_handle_internal);
        }
        if (sound_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_AX failed to acquire");
            return 0;
        }
        rpl_handle = sound_handle_internal;
    } else if (library == LIB_AX_OLD) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_AX_OLD", functionName);
        }
        if (sound_handle_internal_old == 0) {
            OSDynLoad_Acquire("snd_core.rpl", &sound_handle_internal_old);
        }
        if (sound_handle_internal_old == 0) {
            DEBUG_FUNCTION_LINE("LIB_AX_OLD failed to acquire");
            return 0;
        }
        rpl_handle = sound_handle_internal_old;
    } else if (library == LIB_FS) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_FS", functionName);
        }
        if (coreinit_handle_internal == 0) {
            OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle_internal);
        }
        if (coreinit_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_FS failed to acquire");
            return 0;
        }
        rpl_handle = coreinit_handle_internal;
    } else if (library == LIB_OS) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_OS", functionName);
        }
        if (coreinit_handle_internal == 0) {
            OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle_internal);
        }
        if (coreinit_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_OS failed to acquire");
            return 0;
        }
        rpl_handle = coreinit_handle_internal;
    } else if (library == LIB_PADSCORE) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_PADSCORE", functionName);
        }
        if (padscore_handle_internal == 0) {
            OSDynLoad_Acquire("padscore.rpl", &padscore_handle_internal);
        }
        if (padscore_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_PADSCORE failed to acquire");
            return 0;
        }
        rpl_handle = padscore_handle_internal;
    } else if (library == LIB_SOCKET) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_SOCKET", functionName);
        }
        if (nsysnet_handle_internal == 0) {
            OSDynLoad_Acquire("nsysnet.rpl", &nsysnet_handle_internal);
        }
        if (nsysnet_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_SOCKET failed to acquire");
            return 0;
        }
        rpl_handle = nsysnet_handle_internal;
    } else if (library == LIB_SYS) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_SYS", functionName);
        }
        if (sysapp_handle_internal == 0) {
            OSDynLoad_Acquire("sysapp.rpl", &sysapp_handle_internal);
        }
        if (sysapp_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_SYS failed to acquire");
            return 0;
        }
        rpl_handle = sysapp_handle_internal;
    } else if (library == LIB_VPAD) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_VPAD", functionName);
        }
        if (vpad_handle_internal == 0) {
            OSDynLoad_Acquire("vpad.rpl", &vpad_handle_internal);
        }
        if (vpad_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_VPAD failed to acquire");
            return 0;
        }
        rpl_handle = vpad_handle_internal;
    } else if (library == LIB_NN_ACP) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_NN_ACP", functionName);
        }
        if (acp_handle_internal == 0) {
            OSDynLoad_Acquire("nn_acp.rpl", &acp_handle_internal);
        }
        if (acp_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_NN_ACP failed to acquire");
            return 0;
        }
        rpl_handle = acp_handle_internal;
    } else if (library == LIB_SYSHID) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_SYSHID", functionName);
        }
        if (syshid_handle_internal == 0) {
            OSDynLoad_Acquire("nsyshid.rpl", &syshid_handle_internal);
        }
        if (syshid_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_SYSHID failed to acquire");
            return 0;
        }
        rpl_handle = syshid_handle_internal;
    } else if (library == LIB_VPADBASE) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_VPADBASE", functionName);
        }
        if (vpadbase_handle_internal == 0) {
            OSDynLoad_Acquire("vpadbase.rpl", &vpadbase_handle_internal);
        }
        if (vpadbase_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_VPADBASE failed to acquire");
            return 0;
        }
        rpl_handle = vpadbase_handle_internal;
    } else if (library == LIB_PROC_UI) {
        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("FindExport of %s! From LIB_PROC_UI", functionName);
        }
        if (proc_ui_handle_internal == 0) {
            OSDynLoad_Acquire("proc_ui.rpl", &proc_ui_handle_internal);
        }
        if (proc_ui_handle_internal == 0) {
            DEBUG_FUNCTION_LINE("LIB_PROC_UI failed to acquire");
            return 0;
        }
        rpl_handle = proc_ui_handle_internal;
    } else if (library == LIB_NTAG) {
        if (DEBUG_LOG_DYN) {
            log_printf("FindExport of %s! From LIB_NTAG", functionName);
        }
        if (ntag_handle_internal == 0) {
            OSDynLoad_Acquire("ntag.rpl", &ntag_handle_internal);
        }
        if (ntag_handle_internal == 0) {
            log_print("LIB_NTAG failed to acquire");
            return 0;
        }
        rpl_handle = ntag_handle_internal;
    } else if (library == LIB_NFP) {
        if (DEBUG_LOG_DYN) {
            log_printf("FindExport of %s! From LIB_NFP", functionName);
        }
        if (nfp_handle_internal == 0) {
            OSDynLoad_Acquire("nn_nfp.rpl", &nfp_handle_internal);
        }
        if (nfp_handle_internal == 0) {
            log_print("LIB_NFP failed to acquire");
            return 0;
        }
        rpl_handle = nfp_handle_internal;
    } else if (library == LIB_SAVE) {
        if (DEBUG_LOG_DYN) {
            log_printf("FindExport of %s! From LIB_SAVE", functionName);
        }
        if (nn_save_handle_internal == 0) {
            OSDynLoad_Acquire("nn_save.rpl", &nn_save_handle_internal);
        }
        if (nn_save_handle_internal == 0) {
            log_print("LIB_SAVE failed to acquire");
            return 0;
        }
        rpl_handle = nn_save_handle_internal;
    } else if (library == LIB_ACT) {
        if (DEBUG_LOG_DYN) {
            log_printf("FindExport of %s! From LIB_ACT", functionName);
        }
        if (nn_act_handle_internal == 0) {
            OSDynLoad_Acquire("nn_act.rpl", &nn_act_handle_internal);
        }
        if (nn_act_handle_internal == 0) {
            log_print("LIB_ACT failed to acquire");
            return 0;
        }
        rpl_handle = nn_act_handle_internal;
    } else if (library == LIB_NIM) {
        if (DEBUG_LOG_DYN) {
            log_printf("FindExport of %s! From LIB_NIM", functionName);
        }
        if (nn_nim_handle_internal == 0) {
            OSDynLoad_Acquire("nn_nim.rpl", &nn_nim_handle_internal);
        }
        if (nn_nim_handle_internal == 0) {
            log_print("LIB_NIM failed to acquire");
            return 0;
        }
        rpl_handle = nn_nim_handle_internal;
    }

    if (!rpl_handle) {
        DEBUG_FUNCTION_LINE("Failed to find the RPL handle for %s", functionName);
        return 0;
    }

    OSDynLoad_FindExport(rpl_handle, 0, functionName, (void **) &real_addr);

    if (!real_addr) {
        OSDynLoad_FindExport(rpl_handle, 1, functionName, (void **) &real_addr);
        if (!real_addr) {
            DEBUG_FUNCTION_LINE("OSDynLoad_FindExport failed for %s", functionName);
            return 0;
        }
    }

    if ((library == LIB_NN_ACP) && (uint32_t) (*(volatile uint32_t *) (real_addr) & 0x48000002) == 0x48000000) {
        uint32_t address_diff = (uint32_t) (*(volatile uint32_t *) (real_addr) & 0x03FFFFFC);
        if ((address_diff & 0x03000000) == 0x03000000) {
            address_diff |= 0xFC000000;
        }
        real_addr += (int32_t) address_diff;
        if ((uint32_t) (*(volatile uint32_t *) (real_addr) & 0x48000002) == 0x48000000) {
            return 0;
        }
    }

    return real_addr;
}

void resetLibs() {
    acp_handle_internal = 0;
    aoc_handle_internal = 0;
    sound_handle_internal = 0;
    sound_handle_internal_old = 0;
    libcurl_handle_internal = 0;
    gx2_handle_internal = 0;
    nfp_handle_internal = 0;
    nn_act_handle_internal = 0;
    nn_nim_handle_internal = 0;
    nn_save_handle_internal = 0;
    ntag_handle_internal = 0;
    coreinit_handle_internal = 0;
    padscore_handle_internal = 0;
    proc_ui_handle_internal = 0;
    nsysnet_handle_internal = 0;
    sysapp_handle_internal = 0;
    syshid_handle_internal = 0;
    vpad_handle_internal = 0;
    vpadbase_handle_internal = 0;
}
