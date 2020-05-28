/****************************************************************************
 * Copyright (C) 2016-2018 Maschell
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
#include <wups.h>
#include <coreinit/debug.h>

#include "function_patcher.h"
#include <utils/logger.h>
#include <utils/utils.h>
#include <coreinit/memorymap.h>
#include <coreinit/cache.h>
#include <kernel/kernel.h>
#include <whb/log.h>
#include <coreinit/dynload.h>

#define DEBUG_LOG_DYN                                   0

rpl_handling rpl_handles[] __attribute__((section(".data"))) = {
        {WUPS_LOADER_LIBRARY_AVM,      "avm.rpl",      0},
        {WUPS_LOADER_LIBRARY_CAMERA,   "camera.rpl",   0},
        {WUPS_LOADER_LIBRARY_COREINIT, "coreinit.rpl", 0},
        {WUPS_LOADER_LIBRARY_DC,       "dc.rpl",       0},
        {WUPS_LOADER_LIBRARY_DMAE,     "dmae.rpl",     0},
        {WUPS_LOADER_LIBRARY_DRMAPP,   "drmapp.rpl",   0},
        {WUPS_LOADER_LIBRARY_ERREULA,  "erreula.rpl",  0},
        {WUPS_LOADER_LIBRARY_GX2,      "gx2.rpl",      0},
        {WUPS_LOADER_LIBRARY_H264,     "h264.rpl",     0},
        {WUPS_LOADER_LIBRARY_LZMA920,  "lzma920.rpl",  0},
        {WUPS_LOADER_LIBRARY_MIC,      "mic.rpl",      0},
        {WUPS_LOADER_LIBRARY_NFC,      "nfc.rpl",      0},
        {WUPS_LOADER_LIBRARY_NIO_PROF, "nio_prof.rpl", 0},
        {WUPS_LOADER_LIBRARY_NLIBCURL, "nlibcurl.rpl", 0},
        {WUPS_LOADER_LIBRARY_NLIBNSS,  "nlibnss.rpl",  0},
        {WUPS_LOADER_LIBRARY_NLIBNSS2, "nlibnss2.rpl", 0},
        {WUPS_LOADER_LIBRARY_NN_AC,    "nn_ac.rpl",    0},
        {WUPS_LOADER_LIBRARY_NN_ACP,   "nn_acp.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_ACT,   "nn_act.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_AOC,   "nn_aoc.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_BOSS,  "nn_boss.rpl",  0},
        {WUPS_LOADER_LIBRARY_NN_CCR,   "nn_ccr.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_CMPT,  "nn_cmpt.rpl",  0},
        {WUPS_LOADER_LIBRARY_NN_DLP,   "nn_dlp.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_EC,    "nn_ec.rpl",    0},
        {WUPS_LOADER_LIBRARY_NN_FP,    "nn_fp.rpl",    0},
        {WUPS_LOADER_LIBRARY_NN_HAI,   "nn_hai.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_HPAD,  "nn_hpad.rpl",  0},
        {WUPS_LOADER_LIBRARY_NN_IDBE,  "nn_idbe.rpl",  0},
        {WUPS_LOADER_LIBRARY_NN_NDM,   "nn_ndm.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_NETS2, "nn_nets2.rpl", 0},
        {WUPS_LOADER_LIBRARY_NN_NFP,   "nn_nfp.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_NIM,   "nn_nim.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_OLV,   "nn_olv.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_PDM,   "nn_pdm.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_SAVE,  "nn_save.rpl",  0},
        {WUPS_LOADER_LIBRARY_NN_SL,    "nn_sl.rpl",    0},
        {WUPS_LOADER_LIBRARY_NN_SPM,   "nn_spm.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_TEMP,  "nn_temp.rpl",  0},
        {WUPS_LOADER_LIBRARY_NN_UDS,   "nn_uds.rpl",   0},
        {WUPS_LOADER_LIBRARY_NN_VCTL,  "nn_vctl.rpl",  0},
        {WUPS_LOADER_LIBRARY_NSYSCCR,  "nsysccr.rpl",  0},
        {WUPS_LOADER_LIBRARY_NSYSHID,  "nsyshid.rpl",  0},
        {WUPS_LOADER_LIBRARY_NSYSKBD,  "nsyskbd.rpl",  0},
        {WUPS_LOADER_LIBRARY_NSYSNET,  "nsysnet.rpl",  0},
        {WUPS_LOADER_LIBRARY_NSYSUHS,  "nsysuhs.rpl",  0},
        {WUPS_LOADER_LIBRARY_NSYSUVD,  "nsysuvd.rpl",  0},
        {WUPS_LOADER_LIBRARY_NTAG,     "ntag.rpl",     0},
        {WUPS_LOADER_LIBRARY_PADSCORE, "padscore.rpl", 0},
        {WUPS_LOADER_LIBRARY_PROC_UI,  "proc_ui.rpl",  0},
        {WUPS_LOADER_LIBRARY_SNDCORE2, "sndcore2.rpl", 0},
        {WUPS_LOADER_LIBRARY_SNDUSER2, "snduser2.rpl", 0},
        {WUPS_LOADER_LIBRARY_SND_CORE, "snd_core.rpl", 0},
        {WUPS_LOADER_LIBRARY_SND_USER, "snd_user.rpl", 0},
        {WUPS_LOADER_LIBRARY_SWKBD,    "swkbd.rpl",    0},
        {WUPS_LOADER_LIBRARY_SYSAPP,   "sysapp.rpl",   0},
        {WUPS_LOADER_LIBRARY_TCL,      "tcl.rpl",      0},
        {WUPS_LOADER_LIBRARY_TVE,      "tve.rpl",      0},
        {WUPS_LOADER_LIBRARY_UAC,      "uac.rpl",      0},
        {WUPS_LOADER_LIBRARY_UAC_RPL,  "uac_rpl.rpl",  0},
        {WUPS_LOADER_LIBRARY_USB_MIC,  "usb_mic.rpl",  0},
        {WUPS_LOADER_LIBRARY_UVC,      "uvc.rpl",      0},
        {WUPS_LOADER_LIBRARY_UVD,      "uvd.rpl",      0},
        {WUPS_LOADER_LIBRARY_VPAD,     "vpad.rpl",     0},
        {WUPS_LOADER_LIBRARY_VPADBASE, "vpadbase.rpl", 0},
        {WUPS_LOADER_LIBRARY_ZLIB125,  "zlib125.rpl",  0}
};

void new_PatchInvidualMethodHooks(plugin_info_t *plugin_data) {
    new_resetLibs();

    DEBUG_FUNCTION_LINE("Patching %d given functions", plugin_data->number_used_functions);

    int32_t method_hooks_count = plugin_data->number_used_functions;

    uint32_t skip_instr = 1;
    uint32_t my_instr_len = 4;
    uint32_t instr_len = my_instr_len + skip_instr + 15;
    uint32_t flush_len = 4 * instr_len;
    for (int32_t i = 0; i < method_hooks_count; i++) {
        replacement_data_function_t *function_data = &plugin_data->functions[i];
        /* Patch branches to it.  */
        volatile uint32_t *space = function_data->replace_data;

        DEBUG_FUNCTION_LINE_WRITE("Patching %s ...", function_data->function_name);

        if (function_data->library == WUPS_LOADER_LIBRARY_OTHER) {
            WHBLogWritef("Oh, using straight PA/VA");
            if (function_data->alreadyPatched == 1) {
                DEBUG_FUNCTION_LINE("Skipping %s, its already patched", function_data->function_name);
                continue;
            }
        } else {
            if (function_data->functionType == STATIC_FUNCTION && function_data->alreadyPatched == 1) {
                if (new_isDynamicFunction((uint32_t) OSEffectiveToPhysical(function_data->realAddr))) {
                    DEBUG_FUNCTION_LINE("INFO: The function %s is a dynamic function.", function_data->function_name);
                    function_data->functionType = DYNAMIC_FUNCTION;
                } else {
                    WHBLogWritef("Skipping %s, its already patched\n", function_data->function_name);
                    continue;
                }
            }
        }

        uint32_t physical = function_data->physicalAddr;
        uint32_t repl_addr = (uint32_t) function_data->replaceAddr;
        uint32_t call_addr = (uint32_t) function_data->replaceCall;

        uint32_t real_addr = function_data->virtualAddr;
        if (function_data->library != WUPS_LOADER_LIBRARY_OTHER) {
            real_addr = new_GetAddressOfFunction(function_data->function_name, function_data->library);
        }

        if (!real_addr) {
            WHBLogWritef("\n");
            DEBUG_FUNCTION_LINE("OSDynLoad_FindExport failed for %s\n", function_data->function_name);
            continue;
        }

        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("%s is located at %08X!\n", function_data->function_name, real_addr);
        }

        if (function_data->library != WUPS_LOADER_LIBRARY_OTHER) {
            physical = (uint32_t) OSEffectiveToPhysical(real_addr);
        }

        if (!physical) {
            WHBLogWritef("Error. Something is wrong with the physical address\n");
            continue;
        }

        if (DEBUG_LOG_DYN) {
            DEBUG_FUNCTION_LINE("%s physical is located at %08X!\n", function_data->function_name, physical);
        }

        *(volatile uint32_t *) (call_addr) = (uint32_t) (space);

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
            function_data->realAddr = real_addr;
            function_data->restoreInstruction = space[-1];
            if (DEBUG_LOG_DYN) {
                DEBUG_FUNCTION_LINE("function_data->realAddr = %08X!\n", function_data->realAddr);
            }
            if (DEBUG_LOG_DYN) {
                DEBUG_FUNCTION_LINE("function_data->restoreInstruction = %08X!\n", function_data->restoreInstruction);
            }
        } else {
            WHBLogWritef("Error. Can't save %s for restoring!\n", function_data->function_name);
        }

        /*
        00808cfc 3d601234      lis        r11 ,0x1234
        00808d00 616b5678      ori        r11 ,r11 ,0x5678
        00808d04 7d6903a6      mtspr      CTR ,r11
        00808d08 4e800420      bctr
         */
        uint32_t ptr = (uint32_t)space;
        *space = 0x3d600000 | (((real_addr + (skip_instr * 4)) >> 16) & 0x0000FFFF); space++;   // lis        r11 ,0x1234
        *space = 0x616b0000 | ((real_addr + (skip_instr * 4)) & 0x0000ffff); space++;           // ori        r11 ,r11 ,0x5678
        *space = 0x7d6903a6; space++;                                                           // mtspr      CTR ,r11
        *space = 0x4e800420; space++;

        // Only use patched function if OSGetUPID is 2 (wii u menu) or 15 (game)
        uint32_t repl_addr_test = (uint32_t) space;
        *space = 0x3d600000 | (((uint32_t*) OSGetUPID)[0] & 0x0000FFFF); space++;               // lis        r11 ,0x0
        *space = 0x816b0000 | (((uint32_t*) OSGetUPID)[1] & 0x0000FFFF); space++;               // lwz        r11 ,0x0(r11)
        *space = 0x2c0b0000 | 0x00000002; space++;                                              // cmpwi      r11 ,0x2
        *space = 0x41820000 | 0x00000020; space++;                                              // beq        myfunc
        *space = 0x2c0b0000 | 0x0000000F; space++;                                              // cmpwi      r11 ,0xF
        *space = 0x41820000 | 0x00000018; space++;                                              // beq        myfunc
        *space = 0x3d600000 | (((real_addr + (skip_instr * 4)) >> 16) & 0x0000FFFF); space++;   // lis        r11 ,0x1234
        *space = 0x616b0000 | ((real_addr + (skip_instr * 4)) & 0x0000ffff); space++;           // ori        r11 ,r11 ,0x5678
        *space = 0x7d6903a6; space++;                                                           // mtspr      CTR ,r11
        *space = function_data->restoreInstruction; space++;                                    //
        *space = 0x4e800420; space++;                                                           // bctr
// myfunc:
        *space = 0x3d600000 | (((repl_addr) >> 16) & 0x0000FFFF); space++;                      // lis        r11 ,0x1234
        *space = 0x616b0000 | ((repl_addr) & 0x0000ffff); space++;                              // ori        r11 ,r11 ,0x5678
        *space = 0x7d6903a6; space++;                                                           // mtspr      CTR ,r11
        *space = 0x4e800420; space++;                                                           // bctr

        DCFlushRange((void *) (((uint32_t) space) - flush_len), flush_len);
        ICInvalidateRange((void *) (((uint32_t) space) - flush_len), flush_len);

        //setting jump back
        uint32_t replace_instr = 0x48000002 | (repl_addr_test & 0x03fffffc);
        ICInvalidateRange(&replace_instr, 4);
        DCFlushRange(&replace_instr, 4);

        KernelCopyData(physical, (uint32_t) OSEffectiveToPhysical((uint32_t) &replace_instr), 4);
        ICInvalidateRange((void *) (real_addr), 4);
        DCFlushRange((void *) (real_addr), 4);

        function_data->alreadyPatched = 1;
        DEBUG_FUNCTION_LINE("done with patching %s!\n", function_data->function_name);

    }
    DEBUG_FUNCTION_LINE("Done with patching given functions!\n");
}

/* ****************************************************************** */
/*                  RESTORE ORIGINAL INSTRUCTIONS                     */
/* ****************************************************************** */
void new_RestoreInvidualInstructions(plugin_info_t *plugin_data) {
    new_resetLibs();
    DEBUG_FUNCTION_LINE("Restoring given functions!");

    int32_t method_hooks_count = plugin_data->number_used_functions;

    for (int32_t i = 0; i < method_hooks_count; i++) {
        replacement_data_function_t *function_data = &plugin_data->functions[i];

        DEBUG_FUNCTION_LINE("Restoring %s... ", function_data->function_name);
        if (function_data->restoreInstruction == 0 || function_data->realAddr == 0) {
            WHBLogPrintf("I dont have the information for the restore =( skip");
            continue;
        }

        uint32_t real_addr = function_data->virtualAddr;
        if (function_data->library != WUPS_LOADER_LIBRARY_OTHER) {
            real_addr = new_GetAddressOfFunction(function_data->function_name, function_data->library);
        }
        if (!real_addr) {
            WHBLogPrintf("OSDynLoad_FindExport failed for %s", function_data->function_name);
            continue;
        }
        uint32_t physical = function_data->physicalAddr;
        if (function_data->library != WUPS_LOADER_LIBRARY_OTHER) {
            physical = (uint32_t) OSEffectiveToPhysical(real_addr);
        }
        if (!physical) {
            WHBLogPrintf("Something is wrong with the physical address");
            continue;
        }
        if ((function_data->library != WUPS_LOADER_LIBRARY_OTHER) && new_isDynamicFunction(physical)) {
            WHBLogPrintf("Its a dynamic function. We don't need to restore it!", function_data->function_name);
        } else {
            if (function_data->library != WUPS_LOADER_LIBRARY_OTHER) {
                physical = (uint32_t) OSEffectiveToPhysical(function_data->realAddr); //When its an static function, we need to use the old location
            }

            if (DEBUG_LOG_DYN) {
                DEBUG_FUNCTION_LINE("Restoring %08X to %08X", (uint32_t) function_data->restoreInstruction, physical);
            }
            uint32_t targetAddr = (uint32_t) &(function_data->restoreInstruction);
            if (targetAddr < 0x00800000 || targetAddr >= 0x01000000) {
                targetAddr = (uint32_t) OSEffectiveToPhysical(targetAddr);
            } else {
                targetAddr = targetAddr + 0x30800000 - 0x00800000;
            }

            DEBUG_FUNCTION_LINE("Copy %d bytes from %08X to %08x", 4, targetAddr, physical);

            KernelCopyData(physical, targetAddr, 4);
            if (DEBUG_LOG_DYN) {
                DEBUG_FUNCTION_LINE("ICInvalidateRange %08X", (void *) function_data->realAddr);
            }

            if (function_data->library != WUPS_LOADER_LIBRARY_OTHER) {
                ICInvalidateRange((void *) function_data->realAddr, 4);
                DCFlushRange((void *) function_data->realAddr, 4);
            }
            WHBLogPrintf("done");
        }
        function_data->alreadyPatched = 0; // In case a
    }

    DEBUG_FUNCTION_LINE("Done with restoring given functions!");
}

int32_t new_isDynamicFunction(uint32_t physicalAddress) {
    if ((physicalAddress & 0x80000000) == 0x80000000) {
        return 1;
    }
    return 0;
}

uint32_t new_GetAddressOfFunction(const char *functionName, wups_loader_library_type_t library) {
    uint32_t real_addr = 0;

    OSDynLoad_Module rpl_handle = 0;

    int32_t rpl_handles_size = sizeof rpl_handles / sizeof rpl_handles[0];

    for (int32_t i = 0; i < rpl_handles_size; i++) {
        if (rpl_handles[i].library == library) {
            if (rpl_handles[i].handle == 0) {
                DEBUG_FUNCTION_LINE("Lets acquire handle for rpl: %s\n", rpl_handles[i].rplname);
                OSDynLoad_Acquire((char *) rpl_handles[i].rplname, &rpl_handles[i].handle);
            }
            if (rpl_handles[i].handle == 0) {
                WHBLogWritef("%s failed to acquire\n", rpl_handles[i].rplname);
                return 0;
            }
            rpl_handle = rpl_handles[i].handle;
            break;
        }
    }

    if (!rpl_handle) {
        DEBUG_FUNCTION_LINE("Failed to find the RPL handle for %s\n", functionName);
        return 0;
    }

    OSDynLoad_FindExport(rpl_handle, 0, functionName, reinterpret_cast<void **>(&real_addr));

    if (!real_addr) {
        DEBUG_FUNCTION_LINE("OSDynLoad_FindExport failed for %s\n", functionName);
        return 0;
    }

    if ((library == WUPS_LOADER_LIBRARY_NN_ACP) && (uint32_t) (*(volatile uint32_t *) (real_addr) & 0x48000002) == 0x48000000) {
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

void new_resetLibs() {
    int32_t rpl_handles_size = sizeof rpl_handles / sizeof rpl_handles[0];

    for (int32_t i = 0; i < rpl_handles_size; i++) {
        if (rpl_handles[i].handle != 0) {
            DEBUG_FUNCTION_LINE("Resetting handle for rpl: %s\n", rpl_handles[i].rplname);
        }
        rpl_handles[i].handle = 0;
        // Release handle?
    }
}
