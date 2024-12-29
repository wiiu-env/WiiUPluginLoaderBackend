#pragma once

#include <wups/storage.h>

#include <string_view>

#include <cstdint>

namespace StorageUtils::API {
    namespace Internal {
        WUPSStorageError OpenStorage(std::string_view plugin_id, wups_storage_root_item &outItem);
        WUPSStorageError CloseStorage(wups_storage_root_item item);
    } // namespace Internal

    WUPSStorageError SaveStorage(wups_storage_root_item root, bool force);
    WUPSStorageError ForceReloadStorage(wups_storage_root_item root);
    WUPSStorageError WipeStorage(wups_storage_root_item root);

    WUPSStorageError DeleteItem(wups_storage_root_item root, wups_storage_item parent, const char *key);
    WUPSStorageError CreateSubItem(wups_storage_root_item root, wups_storage_item parent, const char *key, wups_storage_item *outItem);
    WUPSStorageError GetSubItem(wups_storage_root_item root, wups_storage_item parent, const char *key, wups_storage_item *outItem);
    WUPSStorageError StoreItem(wups_storage_root_item root, wups_storage_item parent, const char *key, WUPSStorageItemType itemType, void *data, uint32_t length);
    WUPSStorageError GetItem(wups_storage_root_item root, wups_storage_item parent, const char *key, WUPSStorageItemType itemType, void *data, uint32_t maxSize, uint32_t *outSize);
    WUPSStorageError GetItemSize(wups_storage_root_item root, wups_storage_item parent, const char *key, WUPSStorageItemType itemType, uint32_t *outSize);
} // namespace StorageUtils::API
