#pragma once

#include <function_patcher/function_patching.h>

#ifdef __cplusplus
extern "C" {
#endif

extern function_replacement_data_t method_hooks_hooks_static[];
extern uint32_t method_hooks_size_hooks_static;

#ifdef __cplusplus
}
#endif