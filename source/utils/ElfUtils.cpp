#include <stdio.h>
#include <string.h>
#include <coreinit/cache.h>

#include "utils/logger.h"

#include "elfio/elfio.hpp"
#include "ElfUtils.h"

// See https://github.com/decaf-emu/decaf-emu/blob/43366a34e7b55ab9d19b2444aeb0ccd46ac77dea/src/libdecaf/src/cafe/loader/cafe_loader_reloc.cpp#L144
bool ElfUtils::elfLinkOne(char type, size_t offset, int32_t addend, uint32_t destination, uint32_t symbol_addr, relocation_trampolin_entry_t *trampolin_data, uint32_t trampolin_data_length, RelocationType reloc_type, uint8_t trampolinId) {
    if (type == R_PPC_NONE) {
        return true;
    }

    auto target = destination + offset;
    auto value = symbol_addr + addend;

    auto relValue = value - static_cast<uint32_t>(target);

    switch (type) {
        case R_PPC_NONE:
            break;
        case R_PPC_ADDR32:
            *((uint32_t *) (target)) = value;
            break;
        case R_PPC_ADDR16_LO:
            *((uint16_t *) (target)) = static_cast<uint16_t>(value & 0xFFFF);
            break;
        case R_PPC_ADDR16_HI:
            *((uint16_t *) (target)) = static_cast<uint16_t>(value >> 16);
            break;
        case R_PPC_ADDR16_HA:
            *((uint16_t *) (target)) = static_cast<uint16_t>((value + 0x8000) >> 16);
            break;
        case R_PPC_DTPMOD32:
            DEBUG_FUNCTION_LINE("################IMPLEMENT ME");
            //*((int32_t *)(target)) = tlsModuleIndex;
            break;
        case R_PPC_DTPREL32:
            *((uint32_t *) (target)) = value;
            break;
        case R_PPC_GHS_REL16_HA:
            *((uint16_t *) (target)) = static_cast<uint16_t>((relValue + 0x8000) >> 16);
            break;
        case R_PPC_GHS_REL16_HI:
            *((uint16_t *) (target)) = static_cast<uint16_t>(relValue >> 16);
            break;
        case R_PPC_GHS_REL16_LO:
            *((uint16_t *) (target)) = static_cast<uint16_t>(relValue & 0xFFFF);
            break;
        case R_PPC_REL14: {
            auto distance = static_cast<int32_t>(value) - static_cast<int32_t>(target);
            if (distance > 0x7FFC || distance < -0x7FFC) {
                DEBUG_FUNCTION_LINE("***14-bit relative branch cannot hit target.");
                return false;
            }

            if (distance & 3) {
                DEBUG_FUNCTION_LINE("***RELOC ERROR %d: lower 2 bits must be zero before shifting.", -470040);
                return false;
            }

            if ((distance >= 0 && (distance & 0xFFFF8000)) ||
                (distance < 0 && ((distance & 0xFFFF8000) != 0xFFFF8000))) {
                DEBUG_FUNCTION_LINE("***RELOC ERROR %d: upper 17 bits before shift must all be the same.", -470040);
                return false;
            }

            *(int32_t *) target = (*(int32_t *) target & 0xFFBF0003) | (distance & 0x0000fffc);
            break;
        }
        case R_PPC_REL24: {
            // if (isWeakSymbol && !symbolValue) {
            //     symbolValue = static_cast<uint32_t>(target);
            //     value = symbolValue + addend;
            // }
            auto distance = static_cast<int32_t>(value) - static_cast<int32_t>(target);
            if (distance > 0x1FFFFFC || distance < -0x1FFFFFC) {
                if (trampolin_data == NULL) {
                    DEBUG_FUNCTION_LINE("***24-bit relative branch cannot hit target. Trampolin isn't provided");
                    DEBUG_FUNCTION_LINE("***value %08X - target %08X = distance %08X", value, target, distance);
                    return false;
                } else {
                    relocation_trampolin_entry_t *freeSlot = NULL;
                    for (uint32_t i = 0; i < trampolin_data_length; i++) {
                        // We want to override "old" relocations of imports
                        // Pending relocations have the status RELOC_TRAMP_IMPORT_IN_PROGRESS.
                        // When all relocations are done successfully, they will be turned into RELOC_TRAMP_IMPORT_DONE
                        // so they can be overridden/updated/reused on the next application launch.
                        //
                        // Relocations that won't change will have the status RELOC_TRAMP_FIXED and are set to free when the module is unloaded.
                        if (trampolin_data[i].status == RELOC_TRAMP_FREE ||
                            trampolin_data[i].status == RELOC_TRAMP_IMPORT_DONE) {
                            freeSlot = &(trampolin_data[i]);
                            break;
                        }
                    }
                    if (freeSlot == NULL) {
                        DEBUG_FUNCTION_LINE("***24-bit relative branch cannot hit target. Trampolin data list is full");
                        DEBUG_FUNCTION_LINE("***value %08X - target %08X = distance %08X", value, target, target - (uint32_t) &(freeSlot->trampolin[0]));
                        return false;
                    }
                    if (target - (uint32_t) &(freeSlot->trampolin[0]) > 0x1FFFFFC) {
                        DEBUG_FUNCTION_LINE("**Cannot link 24-bit jump (too far to tramp buffer).");
                        DEBUG_FUNCTION_LINE("***value %08X - target %08X = distance %08X", value, target, (target - (uint32_t) &(freeSlot->trampolin[0])));
                        return false;
                    }

                    freeSlot->trampolin[0] = 0x3D600000 | ((((uint32_t) value) >> 16) & 0x0000FFFF); // lis r11, real_addr@h
                    freeSlot->trampolin[1] = 0x616B0000 | (((uint32_t) value) & 0x0000ffff); // ori r11, r11, real_addr@l
                    freeSlot->trampolin[2] = 0x7D6903A6; // mtctr   r11
                    freeSlot->trampolin[3] = 0x4E800420; // bctr
                    DCFlushRange((void *) freeSlot->trampolin, sizeof(freeSlot->trampolin));
                    ICInvalidateRange((unsigned char *) freeSlot->trampolin, sizeof(freeSlot->trampolin));

                    freeSlot->id = trampolinId;
                    DCFlushRange((void *) &freeSlot->id, sizeof(freeSlot->id));
                    ICInvalidateRange((unsigned char *) &freeSlot->id, sizeof(freeSlot->id));

                    if (reloc_type == RELOC_TYPE_FIXED) {
                        freeSlot->status = RELOC_TRAMP_FIXED;
                    } else {
                        // Relocations for the imports may be overridden
                        freeSlot->status = RELOC_TRAMP_IMPORT_DONE;
                    }
                    uint32_t symbolValue = (uint32_t) &(freeSlot->trampolin[0]);
                    value = symbolValue + addend;
                    distance = static_cast<int32_t>(value) - static_cast<int32_t>(target);
                }
            }

            if (distance & 3) {
                DEBUG_FUNCTION_LINE("***RELOC ERROR %d: lower 2 bits must be zero before shifting.", -470022);
                return false;
            }

            if (distance < 0 && (distance & 0xFE000000) != 0xFE000000) {
                DEBUG_FUNCTION_LINE("***RELOC ERROR %d: upper 7 bits before shift must all be the same (1).", -470040);
                return false;
            }

            if (distance >= 0 && (distance & 0xFE000000)) {
                DEBUG_FUNCTION_LINE("***RELOC ERROR %d: upper 7 bits before shift must all be the same (0).", -470040);
                return false;
            }

            *(int32_t *) target = (*(int32_t *) target & 0xfc000003) | (distance & 0x03fffffc);
            break;
        }
        default:
            DEBUG_FUNCTION_LINE("***ERROR: Unsupported Relocation_Add Type (%08X):", type);
            return false;
    }
    return true;
}
