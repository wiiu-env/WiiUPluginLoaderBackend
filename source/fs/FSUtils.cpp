#include "fs/FSUtils.h"
#include "fs/CFile.hpp"
#include "utils/logger.h"
#include "utils/utils.h"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>

int32_t FSUtils::LoadFileToMem(const char *filepath, uint8_t **inbuffer, uint32_t *size) {
    //! always initialze input
    *inbuffer = NULL;
    if (size) {
        *size = 0;
    }

    int32_t iFd = open(filepath, O_RDONLY);
    if (iFd < 0) {
        return -1;
    }

    struct stat file_stat;
    int rc = fstat(iFd, &file_stat);
    if (rc < 0) {
        close(iFd);
        return -4;
    }
    uint32_t filesize = file_stat.st_size;

    auto *buffer = (uint8_t *) memalign(0x40, ROUNDUP(filesize, 0x40));
    if (buffer == nullptr) {
        close(iFd);
        return -2;
    }

    uint32_t blocksize = 0x80000;
    uint32_t done      = 0;
    int32_t readBytes;

    while (done < filesize) {
        if (done + blocksize > filesize) {
            blocksize = filesize - done;
        }
        readBytes = read(iFd, buffer + done, blocksize);
        if (readBytes <= 0) {
            break;
        }
        done += readBytes;
    }

    ::close(iFd);

    if (done != filesize) {
        free(buffer);
        buffer = nullptr;
        return -3;
    }

    *inbuffer = buffer;

    //! sign is optional input
    if (size) {
        *size = filesize;
    }

    return filesize;
}

int32_t FSUtils::CheckFile(const char *filepath) {
    if (!filepath)
        return 0;

    struct stat filestat;

    char dirnoslash[strlen(filepath) + 2];
    snprintf(dirnoslash, sizeof(dirnoslash), "%s", filepath);

    while (dirnoslash[strlen(dirnoslash) - 1] == '/')
        dirnoslash[strlen(dirnoslash) - 1] = '\0';

    char *notRoot = strrchr(dirnoslash, '/');
    if (!notRoot) {
        strcat(dirnoslash, "/");
    }

    if (stat(dirnoslash, &filestat) == 0)
        return 1;

    return 0;
}

int32_t FSUtils::CreateSubfolder(const char *fullpath) {
    if (!fullpath)
        return 0;

    int32_t result = 0;

    char dirnoslash[strlen(fullpath) + 1];
    strcpy(dirnoslash, fullpath);

    int32_t pos = strlen(dirnoslash) - 1;
    while (dirnoslash[pos] == '/') {
        dirnoslash[pos] = '\0';
        pos--;
    }

    if (CheckFile(dirnoslash)) {
        return 1;
    } else {
        char parentpath[strlen(dirnoslash) + 2];
        strcpy(parentpath, dirnoslash);
        char *ptr = strrchr(parentpath, '/');

        if (!ptr) {
            //!Device root directory (must be with '/')
            strcat(parentpath, "/");
            struct stat filestat;
            if (stat(parentpath, &filestat) == 0)
                return 1;

            return 0;
        }

        ptr++;
        ptr[0] = '\0';

        result = CreateSubfolder(parentpath);
    }

    if (!result)
        return 0;

    if (mkdir(dirnoslash, 0777) == -1) {
        return 0;
    }

    return 1;
}

int32_t FSUtils::saveBufferToFile(const char *path, void *buffer, uint32_t size) {
    CFile file(path, CFile::WriteOnly);
    if (!file.isOpen()) {
        DEBUG_FUNCTION_LINE("Failed to open %s\n", path);
        return 0;
    }
    int32_t written = file.write((const uint8_t *) buffer, size);
    file.close();
    return written;
}
