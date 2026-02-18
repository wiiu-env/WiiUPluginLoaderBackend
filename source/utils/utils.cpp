#include "utils.h"

#include "StringTools.h"
#include "fs/CFile.hpp"
#include "globals.h"
#include "json.hpp"
#include "logger.h"

#include <coreinit/debug.h>
#include <coreinit/ios.h>

#include <wups/hooks.h>
#include <wups/storage.h>

#include <algorithm>
#include <string>

#include <malloc.h>
#include <sys/dirent.h>

static std::string sPluginPath;
static std::string sModulePath;
static std::string sEnvironmentPath;

std::string getEnvironmentPath() {
    if (!sEnvironmentPath.empty()) {
        return sEnvironmentPath;
    }
    char environmentPath[0x100] = {};

    if (const auto handle = IOS_Open("/dev/mcp", IOS_OPEN_READ); handle >= 0) {
        int in = 0xF9; // IPC_CUSTOM_COPY_ENVIRONMENT_PATH
        if (IOS_Ioctl(handle, 100, &in, sizeof(in), environmentPath, sizeof(environmentPath)) != IOS_ERROR_OK) {
            return "fs:/vol/external01/wiiu/environments/aroma";
        }

        IOS_Close(handle);
    }
    sEnvironmentPath = environmentPath;
    return sEnvironmentPath;
}

std::string getPluginPath() {
    if (!sPluginPath.empty()) {
        return sPluginPath;
    }

    sPluginPath = getEnvironmentPath().append("/plugins");
    return sPluginPath;
}

std::string getModulePath() {
    if (!sModulePath.empty()) {
        return sModulePath;
    }

    sModulePath = getEnvironmentPath().append("/modules");
    return sModulePath;
}

// https://gist.github.com/ccbrown/9722406
void dumpHex(const void *data, const size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    DEBUG_FUNCTION_LINE("0x%p (0x0000): ", data);
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

UtilsIOError ParseJsonFromFile(const std::string &filePath, nlohmann::json &outJson) {
    CFile file(filePath, CFile::ReadOnly);
    if (!file.isOpen() || file.size() == 0) {
        return UTILS_IO_ERROR_NOT_FOUND;
    }
    auto *json_data = static_cast<uint8_t *>(memalign(0x40, ROUNDUP(file.size() + 1, 0x40)));
    if (!json_data) {
        return UTILS_IO_ERROR_MALLOC_FAILED;
    }
    auto result      = UTILS_IO_ERROR_SUCCESS;
    uint64_t readRes = file.read(json_data, file.size());
    if (readRes == file.size()) {
        json_data[file.size()] = '\0';
        outJson                = nlohmann::json::parse(json_data, nullptr, false);
    } else {
        result = UTILS_IO_ERROR_GENERIC;
    }
    file.close();
    free(json_data);
    return result;
}

std::vector<std::string> getPluginFilePaths(std::string_view basePath) {
    std::vector<std::string> result;
    struct dirent *dp;
    DIR *dfd;

    if (basePath.empty()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to scan plugin dir: Path was empty");
        return result;
    }

    if ((dfd = opendir(basePath.data())) == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("Couldn't open dir %s", basePath.data());
        return result;
    }

    while ((dp = readdir(dfd)) != nullptr) {
        if (dp->d_type == DT_DIR) {
            continue;
        }

        if (std::string_view(dp->d_name).starts_with('.') || std::string_view(dp->d_name).starts_with('_') || !std::string_view(dp->d_name).ends_with(".wps")) {
            DEBUG_FUNCTION_LINE_WARN("Skip file %s/%s", basePath.data(), dp->d_name);
            continue;
        }

        auto full_file_path = string_format("%s/%s", basePath.data(), dp->d_name);
        result.push_back(full_file_path);
    }
    closedir(dfd);
    return result;
}

std::vector<std::string> getNonBaseAromaPluginFilenames(std::string_view basePath) {
    std::vector<std::string> result;

    for (const auto &filePath : getPluginFilePaths(basePath)) {
        std::string fileName = StringTools::FullpathToFilename(filePath.c_str());

        const char *baseAromaFileNames[] = {
                "AromaBasePlugin.wps",
                "drc_region_free.wps",
                "homebrew_on_menu.wps",
                "regionfree.wps",
        };

        bool found = false;
        for (const auto &cur : baseAromaFileNames) {
            if (fileName == cur) {
                found = true;
                break;
            }
        }
        if (!found) {
            result.push_back(fileName);
        }
    }
    return result;
}

__attribute__((noinline))
std::vector<uint32_t>
CaptureStackTrace(uint32_t maxDepth) {
    std::vector<uint32_t> trace;
    trace.reserve(maxDepth);

    uint32_t *stackPointer;
    // Grab the current Stack Pointer (r1)
    asm volatile("mr %0, 1"
                 : "=r"(stackPointer));

    for (uint32_t i = 0; i < maxDepth + 1; ++i) {
        // Basic alignment check
        if (!stackPointer || (reinterpret_cast<uintptr_t>(stackPointer) & 0x3)) {
            break;
        }

        uint32_t backChain  = stackPointer[0];
        uint32_t returnAddr = stackPointer[1];

        if (returnAddr == 0) break;
        if (i != 0) {
            trace.push_back(returnAddr);
        }

        if (backChain == 0) break;
        stackPointer = reinterpret_cast<uint32_t *>(backChain);
    }

    return trace;
}

#define SC17_FindClosestSymbol ((uint32_t(*)(uint32_t addr, int32_t * outDistance, char *symbolNameBuffer, uint32_t symbolNameBufferLength, char *moduleNameBuffer, uint32_t moduleNameBufferLength))(0x101C400 + 0x1f934))

std::string getModuleAndSymbolName(uint32_t addr) {
    int distance         = 0;
    char moduleName[50]  = {};
    char symbolName[256] = {};

    if (SC17_FindClosestSymbol(addr, &distance, symbolName, sizeof(symbolName) - 1, moduleName, sizeof(moduleName) - 1) != 0) {
        return string_format("0x%08X", addr);
    } else {
        moduleName[sizeof(moduleName) - 1] = '\0';
        symbolName[sizeof(symbolName) - 1] = '\0';
        return string_format("%s|%s+0x%X",
                             moduleName,
                             symbolName,
                             distance);
    }
}

void PrintCapturedStackTrace(const std::span<const uint32_t> trace) {
    if (trace.empty()) {
        DEBUG_FUNCTION_LINE_INFO("┌────────────────────── CAPTURED TRACE ──────────────────────┐");
        DEBUG_FUNCTION_LINE_INFO("│ <Empty Trace>");
        DEBUG_FUNCTION_LINE_INFO("└────────────────────────────────────────────────────────────┘");
        return;
    }

    DEBUG_FUNCTION_LINE_INFO("┌────────────────────── CAPTURED TRACE ──────────────────────┐");
    for (size_t i = 0; i < trace.size(); ++i) {
        uint32_t addr        = trace[i];
        int distance         = 0;
        char moduleName[50]  = {};
        char symbolName[256] = {};

        if (SC17_FindClosestSymbol(addr, &distance, symbolName, sizeof(symbolName) - 1, moduleName, sizeof(moduleName) - 1) != 0) {
            DEBUG_FUNCTION_LINE_INFO("│ [%02d] 0x%08X", i, addr);
        } else {
            moduleName[sizeof(moduleName) - 1] = '\0';
            symbolName[sizeof(symbolName) - 1] = '\0';
            DEBUG_FUNCTION_LINE_INFO("│ [%02d] %s : %s + 0x%X (0x%08X)",
                                     i,
                                     moduleName,
                                     symbolName,
                                     distance,
                                     addr);
        }
    }

    DEBUG_FUNCTION_LINE_INFO("└────────────────────────────────────────────────────────────┘");
}

std::string hookNameToString(const wups_loader_hook_type_t type) {
    switch (type) {
        case WUPS_LOADER_HOOK_INIT_WUT_MALLOC:
            return "WUPS_LOADER_HOOK_INIT_WUT_MALLOC";
        case WUPS_LOADER_HOOK_FINI_WUT_MALLOC:
            return "WUPS_LOADER_HOOK_FINI_WUT_MALLOC";
        case WUPS_LOADER_HOOK_INIT_WUT_NEWLIB:
            return "WUPS_LOADER_HOOK_INIT_WUT_NEWLIB";
        case WUPS_LOADER_HOOK_FINI_WUT_NEWLIB:
            return "WUPS_LOADER_HOOK_FINI_WUT_NEWLIB";
        case WUPS_LOADER_HOOK_INIT_WUT_STDCPP:
            return "WUPS_LOADER_HOOK_INIT_WUT_STDCPP";
        case WUPS_LOADER_HOOK_FINI_WUT_STDCPP:
            return "WUPS_LOADER_HOOK_FINI_WUT_STDCPP";
        case WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB:
            return "WUPS_LOADER_HOOK_INIT_WUT_DEVOPTAB";
        case WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB:
            return "WUPS_LOADER_HOOK_FINI_WUT_DEVOPTAB";
        case WUPS_LOADER_HOOK_INIT_WUT_SOCKETS:
            return "WUPS_LOADER_HOOK_INIT_WUT_SOCKETS";
        case WUPS_LOADER_HOOK_FINI_WUT_SOCKETS:
            return "WUPS_LOADER_HOOK_FINI_WUT_SOCKETS";
        case WUPS_LOADER_HOOK_INIT_WRAPPER:
            return "WUPS_LOADER_HOOK_INIT_WRAPPER";
        case WUPS_LOADER_HOOK_FINI_WRAPPER:
            return "WUPS_LOADER_HOOK_FINI_WRAPPER";
        case WUPS_LOADER_HOOK_GET_CONFIG_DEPRECATED:
            return "WUPS_LOADER_HOOK_GET_CONFIG_DEPRECATED";
        case WUPS_LOADER_HOOK_CONFIG_CLOSED_DEPRECATED:
            return "WUPS_LOADER_HOOK_CONFIG_CLOSED_DEPRECATED";
        case WUPS_LOADER_HOOK_INIT_STORAGE_DEPRECATED:
            return "WUPS_LOADER_HOOK_INIT_STORAGE_DEPRECATED";
        case WUPS_LOADER_HOOK_INIT_PLUGIN:
            return "WUPS_LOADER_HOOK_INIT_PLUGIN";
        case WUPS_LOADER_HOOK_DEINIT_PLUGIN:
            return "WUPS_LOADER_HOOK_DEINIT_PLUGIN";
        case WUPS_LOADER_HOOK_APPLICATION_STARTS:
            return "WUPS_LOADER_HOOK_APPLICATION_STARTS";
        case WUPS_LOADER_HOOK_RELEASE_FOREGROUND:
            return "WUPS_LOADER_HOOK_RELEASE_FOREGROUND";
        case WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND:
            return "WUPS_LOADER_HOOK_ACQUIRED_FOREGROUND";
        case WUPS_LOADER_HOOK_APPLICATION_REQUESTS_EXIT:
            return "WUPS_LOADER_HOOK_APPLICATION_REQUESTS_EXIT";
        case WUPS_LOADER_HOOK_APPLICATION_ENDS:
            return "WUPS_LOADER_HOOK_APPLICATION_ENDS";
        case WUPS_LOADER_HOOK_INIT_STORAGE:
            return "WUPS_LOADER_HOOK_INIT_STORAGE";
        case WUPS_LOADER_HOOK_INIT_CONFIG:
            return "WUPS_LOADER_HOOK_INIT_CONFIG";
        case WUPS_LOADER_HOOK_INIT_BUTTON_COMBO:
            return "WUPS_LOADER_HOOK_INIT_BUTTON_COMBO";
        case WUPS_LOADER_HOOK_INIT_WUT_THREAD:
            return "WUPS_LOADER_HOOK_INIT_WUT_THREAD";
    }
    return "<UNKNOWN>";
}