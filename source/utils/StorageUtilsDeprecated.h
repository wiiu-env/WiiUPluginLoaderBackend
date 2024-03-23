#pragma once
#include <cstdint>

namespace WUPSStorageDeprecated {
    typedef enum wups_storage_type_t_ {
        WUPS_STORAGE_TYPE_INVALID,
        WUPS_STORAGE_TYPE_STRING,
        WUPS_STORAGE_TYPE_INT,
        WUPS_STORAGE_TYPE_ITEM,
    } wups_storage_type_t;

    typedef enum {
        WUPS_STORAGE_ERROR_SUCCESS                = 0,
        WUPS_STORAGE_ERROR_NOT_OPENED             = -1,
        WUPS_STORAGE_ERROR_ALREADY_OPENED         = -2,
        WUPS_STORAGE_ERROR_INVALID_ARGS           = -3,
        WUPS_STORAGE_ERROR_NOT_FOUND              = -4,
        WUPS_STORAGE_ERROR_NOT_INITIALIZED        = -5,
        WUPS_STORAGE_ERROR_INVALID_BACKEND_PARAMS = -6,
        WUPS_STORAGE_ERROR_INVALID_JSON           = -7,
        WUPS_STORAGE_ERROR_IO                     = -8,
        WUPS_STORAGE_ERROR_B64_DECODE_FAILED      = -9,
        WUPS_STORAGE_ERROR_BUFFER_TOO_SMALL       = -10,
        WUPS_STORAGE_ERROR_MALLOC_FAILED          = -11,
        WUPS_STORAGE_ERROR_NOT_ACTIVE_CATEGORY    = -13,
    } WUPSStorageError;

    typedef struct wups_storage_item_t_ {
        char *key;
        void *data;
        uint32_t data_size;
        uint32_t deleted;
        wups_storage_type_t type;
    } wups_storage_item_t;

    typedef WUPSStorageError (*OpenStorageFunction)(const char *plugin_id, wups_storage_item_t *items);
    typedef WUPSStorageError (*CloseStorageFunction)(const char *plugin_id, wups_storage_item_t *items);

    typedef struct wups_loader_init_storage_args_t_ {
        OpenStorageFunction open_storage_ptr;
        CloseStorageFunction close_storage_ptr;
        const char *plugin_id;
    } wups_loader_init_storage_args_t;

    class StorageUtils {
    public:
        static WUPSStorageError OpenStorage(const char *plugin_id, wups_storage_item_t *items);

        static WUPSStorageError CloseStorage(const char *plugin_id, wups_storage_item_t *items);
    };
} // namespace WUPSStorageDeprecated
