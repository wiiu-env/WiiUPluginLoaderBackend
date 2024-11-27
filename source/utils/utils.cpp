#include "utils.h"
#include "globals.h"
#include "logger.h"
#include <algorithm>
#include <coreinit/ios.h>
#include <malloc.h>
#include <string>

static std::string sPluginPath;
std::string getPluginPath() {
    if (!sPluginPath.empty()) {
        return sPluginPath;
    }
    char environmentPath[0x100] = {};

    if (const auto handle = IOS_Open("/dev/mcp", IOS_OPEN_READ); handle >= 0) {
        int in = 0xF9; // IPC_CUSTOM_COPY_ENVIRONMENT_PATH
        if (IOS_Ioctl(handle, 100, &in, sizeof(in), environmentPath, sizeof(environmentPath)) != IOS_ERROR_OK) {
            return "fs:/vol/external01/wiiu/plugins";
        }

        IOS_Close(handle);
    }
    sPluginPath = std::string(environmentPath).append("/plugins");
    return sPluginPath;
}

// https://gist.github.com/ccbrown/9722406
void dumpHex(const void *data, const size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    DEBUG_FUNCTION_LINE("0x%08X (0x0000): ", data);
    for (i = 0; i < size; ++i) {
        WHBLogWritef("%02X ", ((unsigned char *) data)[i]);
        if (((unsigned char *) data)[i] >= ' ' && ((unsigned char *) data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char *) data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i + 1) % 8 == 0 || i + 1 == size) {
            WHBLogWritef(" ");
            if ((i + 1) % 16 == 0) {
                WHBLogPrintf("|  %s ", ascii);
                if (i + 1 < size) {
                    DEBUG_FUNCTION_LINE("0x%08X (0x%04X); ", ((uint32_t) data) + i + 1, i + 1);
                }
            } else if (i + 1 == size) {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8) {
                    WHBLogWritef(" ");
                }
                for (j = (i + 1) % 16; j < 16; ++j) {
                    WHBLogWritef("   ");
                }
                WHBLogPrintf("|  %s ", ascii);
            }
        }
    }
}

OSDynLoad_Error CustomDynLoadAlloc(int32_t size, int32_t align, void **outAddr) {
    if (!outAddr) {
        return OS_DYNLOAD_INVALID_ALLOCATOR_PTR;
    }

    if (align >= 0 && align < 4) {
        align = 4;
    } else if (align < 0 && align > -4) {
        align = -4;
    }

    if (*outAddr = memalign(align, size); !*outAddr) {
        return OS_DYNLOAD_OUT_OF_MEMORY;
    }
    // keep track of allocated memory to clean it up if RPLs won't get unloaded properly
    gAllocatedAddresses.push_back(*outAddr);

    return OS_DYNLOAD_OK;
}

void CustomDynLoadFree(void *addr) {
    free(addr);

    // Remove from list
    if (const auto it = std::ranges::find(gAllocatedAddresses, addr); it != gAllocatedAddresses.end()) {
        gAllocatedAddresses.erase(it);
    }
}