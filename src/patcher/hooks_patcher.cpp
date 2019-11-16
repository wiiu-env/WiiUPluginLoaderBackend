#include <utils/logger.h>
#include <utils/function_patcher.h>
#include "hooks_patcher.h"

hooks_magic_t method_hooks_hooks[] __attribute__((section(".data"))) = {
};

uint32_t method_hooks_size_hooks __attribute__((section(".data"))) = sizeof(method_hooks_hooks) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile uint32_t method_calls_hooks[sizeof(method_hooks_hooks) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

