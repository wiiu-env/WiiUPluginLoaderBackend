#pragma once
#include <stdint.h>

typedef enum RelocationTrampolinStatus{
   RELOC_TRAMP_FREE                 = 0,
   RELOC_TRAMP_FIXED                = 1,
   RELOC_TRAMP_IMPORT_IN_PROGRESS   = 2,
   RELOC_TRAMP_IMPORT_DONE          = 3,
} RelocationTrampolinStatus;

typedef enum RelocationType{
   RELOC_TYPE_FIXED     = 0,
   RELOC_TYPE_IMPORT   = 1
} RelocationType;

typedef struct relocation_trampolin_entry_t {
    uint32_t id;
    uint32_t trampolin[4];
    RelocationTrampolinStatus status = RELOC_TRAMP_FREE;
} relocation_trampolin_entry_t;
