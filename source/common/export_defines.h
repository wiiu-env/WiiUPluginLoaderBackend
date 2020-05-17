#pragma once

#include <cstdint>

#define EXPORT_MAXIMUM_NAME_LENGTH     50
typedef struct export_data_t {
    uint32_t type;
    char name[EXPORT_MAXIMUM_NAME_LENGTH];
    uint32_t address = 0;
} export_data_t;