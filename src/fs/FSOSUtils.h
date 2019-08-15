#ifndef __FS_OS_UTILS_H_
#define __FS_OS_UTILS_H_

#include <stdint.h>

class FSOSUtils {
public:

    static int32_t MountFS(void *pClient, void *pCmd, char **mount_path);
    static int32_t UmountFS(void *pClient, void *pCmd, const char *mountPath);
};

#endif // __FS_OS_UTILS_H_
