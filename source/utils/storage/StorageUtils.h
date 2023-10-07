#pragma once

#include <wups/storage.h>

namespace StorageUtils::API {
    WUPSStorageError OpenStorage(const char *plugin_id, wups_storage_root_item *item);
    WUPSStorageError CloseStorage(const char *plugin_id);
    WUPSStorageError DeleteItem(wups_storage_root_item root, wups_storage_item parent, const char *key);
    WUPSStorageError CreateSubItem(wups_storage_root_item root, wups_storage_item parent, const char *key, wups_storage_item *outItem);
    WUPSStorageError GetSubItem(wups_storage_root_item root, wups_storage_item parent, const char *key, wups_storage_item *outItem);
    WUPSStorageError StoreItem(wups_storage_root_item root, wups_storage_item parent, const char *key, WUPSStorageItemType itemType, void *data, uint32_t length);
    WUPSStorageError GetItem(wups_storage_root_item root, wups_storage_item parent, const char *key, WUPSStorageItemType itemType, void *data, uint32_t maxSize, uint32_t *outSize);
    WUPSStorageError GetItemSize(wups_storage_root_item root, wups_storage_item parent, const char *key, uint32_t *outSize);
} // namespace StorageUtils::API
