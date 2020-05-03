#pragma once

#include <stdint.h>
#include <wups.h>

#define FUNCTION_PATCHER_METHOD_STORE_SIZE                  13
#define MAXIMUM_FUNCTION_NAME_LENGTH                        83

struct replacement_data_function_t {
    uint32_t                    physicalAddr;                                       /* [needs to be filled]  */
    uint32_t                    virtualAddr;                                        /* [needs to be filled]  */
    uint32_t                    replaceAddr;                                        /* [needs to be filled] Address of our replacement function */
    uint32_t                    replaceCall;                                        /* [needs to be filled] Address to access the real_function */
    wups_loader_library_type_t  library;                                            /* [needs to be filled] rpl where the function we want to replace is. */
    char                        function_name[MAXIMUM_FUNCTION_NAME_LENGTH];        /* [needs to be filled] name of the function we want to replace */
    uint32_t                    realAddr;                                           /* [will be filled] Address of the real function we want to replace. */
    volatile uint32_t           replace_data [FUNCTION_PATCHER_METHOD_STORE_SIZE];  /* [will be filled] Space for us to store some jump instructions */
    uint32_t                    restoreInstruction;                                 /* [will be filled] Copy of the instruction we replaced to jump to our code. */
    uint8_t                     functionType;                                       /* [will be filled] */
    uint8_t                     alreadyPatched;                                     /* [will be filled] */
};

struct replacement_data_hook_t {
    void * func_pointer = NULL;                                     /* [will be filled] */
    wups_loader_hook_type_t type;                                   /* [will be filled] */
};
