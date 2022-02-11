#pragma once

#include <wups/storage.h>

class StorageUtils {
public:
    static int OpenStorage(const char *plugin_id, wups_storage_item_t *items);

    static int CloseStorage(const char *plugin_id, wups_storage_item_t *items);
};
