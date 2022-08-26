#pragma once

#include <wups/storage.h>

class StorageUtils {
public:
    static WUPSStorageError OpenStorage(const char *plugin_id, wups_storage_item_t *items);

    static WUPSStorageError CloseStorage(const char *plugin_id, wups_storage_item_t *items);
};
