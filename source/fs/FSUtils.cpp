#include "fs/FSUtils.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <unistd.h>
#include <vector>

int32_t FSUtils::LoadFileToMem(std::string_view filepath, std::vector<uint8_t> &buffer) {
    //! always initialize input
    buffer.clear();

    int32_t iFd = open(filepath.data(), O_RDONLY);
    if (iFd < 0) {
        return -1;
    }

    struct stat file_stat {};
    int rc = fstat(iFd, &file_stat);
    if (rc < 0) {
        close(iFd);
        return -4;
    }
    uint32_t filesize = file_stat.st_size;

    buffer.resize(filesize);

    uint32_t blocksize = 0x80000;
    uint32_t done      = 0;
    int32_t readBytes;

    while (done < filesize) {
        if (done + blocksize > filesize) {
            blocksize = filesize - done;
        }
        readBytes = read(iFd, buffer.data() + done, blocksize);
        if (readBytes <= 0) {
            break;
        }
        done += readBytes;
    }

    ::close(iFd);

    if (done != filesize) {
        buffer.clear();
        return -3;
    }

    return 0;
}

bool FSUtils::CreateSubfolder(std::string_view fullpath) {
    std::error_code err;
    if (!std::filesystem::create_directories(fullpath, err)) {
        return std::filesystem::exists(fullpath, err);
    }
    return true;
}
