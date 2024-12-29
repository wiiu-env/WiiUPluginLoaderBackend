#include "utils.h"

#include "StringTools.h"
#include "fs/CFile.hpp"
#include "globals.h"
#include "json.hpp"
#include "logger.h"

#include <coreinit/ios.h>

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