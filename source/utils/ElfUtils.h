#pragma once

#include <stdint.h>
#include <wums/defines/relocation_defines.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t LoadFileToMem(const char *relativefilepath, char **fileOut, uint32_t *sizeOut);
uint32_t load_loader_elf_from_sd(unsigned char *baseAddress, const char *relativePath);
uint32_t load_loader_elf(unsigned char *baseAddress, char *elf_data, uint32_t fileSize);

#define R_PPC_NONE           0
#define R_PPC_ADDR32         1
#define R_PPC_ADDR16_LO      4
#define R_PPC_ADDR16_HI      5
#define R_PPC_ADDR16_HA      6
#define R_PPC_REL24          10
#define R_PPC_REL14          11
#define R_PPC_DTPMOD32       68
#define R_PPC_DTPREL32       78
#define R_PPC_EMB_SDA21      109
#define R_PPC_EMB_RELSDA     116
#define R_PPC_DIAB_SDA21_LO  180
#define R_PPC_DIAB_SDA21_HI  181
#define R_PPC_DIAB_SDA21_HA  182
#define R_PPC_DIAB_RELSDA_LO 183
#define R_PPC_DIAB_RELSDA_HI 184
#define R_PPC_DIAB_RELSDA_HA 185
#define R_PPC_GHS_REL16_HA   251
#define R_PPC_GHS_REL16_HI   252
#define R_PPC_GHS_REL16_LO   253

// Masks for manipulating Power PC relocation targets
#define PPC_WORD32           0xFFFFFFFF
#define PPC_WORD30           0xFFFFFFFC
#define PPC_LOW24            0x03FFFFFC
#define PPC_LOW14            0x0020FFFC
#define PPC_HALF16           0xFFFF

#ifdef __cplusplus
}
#endif

class ElfUtils {

public:
    static bool elfLinkOne(char type, size_t offset, int32_t addend, uint32_t destination, uint32_t symbol_addr, std::vector<relocation_trampoline_entry_t> &trampolineData,
                           RelocationType reloc_type, uint8_t trampolineId);
};
