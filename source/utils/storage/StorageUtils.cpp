#include "StorageItem.h"
#include "StorageItemRoot.h"
#include "StorageSubItem.h"
#include "fs/CFile.hpp"
#include "fs/FSUtils.h"
#include "utils/base64.h"
#include "utils/json.hpp"
#include "utils/logger.h"
#include "utils/utils.h"

#include <forward_list>
#include <malloc.h>
#include <mutex>
#include <wups/storage.h>

namespace StorageUtils {
    std::forward_list<StorageItemRoot> gStorage;
    std::mutex gStorageMutex;

    namespace Helper {
        static WUPSStorageError ConvertToWUPSError(const StorageSubItem::StorageSubItemError &error) {
            switch (error) {
                case StorageSubItem::STORAGE_SUB_ITEM_ERROR_NONE:
                    return WUPS_STORAGE_ERROR_SUCCESS;
                case StorageSubItem::STORAGE_SUB_ITEM_ERROR_MALLOC_FAILED:
                    return WUPS_STORAGE_ERROR_MALLOC_FAILED;
                case StorageSubItem::STORAGE_SUB_ITEM_KEY_ALREADY_IN_USE:
                    return WUPS_STORAGE_ERROR_ALREADY_EXISTS;
            }
            return WUPS_STORAGE_ERROR_UNKNOWN_ERROR;
        }

        static bool deserializeFromJson(const nlohmann::json &json, StorageSubItem &item) {
            for (auto it = json.begin(); it != json.end(); ++it) {
                StorageSubItem::StorageSubItemError subItemError = StorageSubItem::STORAGE_SUB_ITEM_ERROR_NONE;
                if (it.value().is_object()) {
                    auto res = item.createSubItem(it.key().c_str(), subItemError);
                    if (!res) {
                        DEBUG_FUNCTION_LINE_WARN("Failed to create sub item: Error %d", subItemError);
                        return false;
                    }
                    if (!it.value().empty()) {
                        if (!deserializeFromJson(it.value(), *res)) {
                            DEBUG_FUNCTION_LINE_WARN("Deserialization of sub item failed.");
                            return false;
                        }
                    }
                } else {
                    auto res = item.createItem(it.key().c_str(), subItemError);

                    if (!res) {
                        DEBUG_FUNCTION_LINE_WARN("Failed to create Item for key %s. Error %d", it.key().c_str(), subItemError);
                        return false;
                    }
                    if (it.value().is_string()) {
                        auto val = it.value().get<std::string>();
                        res->setValue(val);
                    } else if (it.value().is_boolean()) {
                        auto val = it.value().get<bool>();
                        res->setValue(val);
                    } else if (it.value().is_number_unsigned()) {
                        auto val = it.value().get<std::uint64_t>();
                        res->setValue(val);
                    } else if (it.value().is_number_integer()) {
                        auto val = it.value().get<std::int64_t>();
                        res->setValue(val);
                    } else if (it.value().is_number_float()) {
                        auto val = it.value().get<std::double_t>();
                        res->setValue(val);
                    } else {
                        DEBUG_FUNCTION_LINE_ERR("Unknown type %s for value %s", it.value().type_name(), it.key().c_str());
                    }
                }
            }
            return true;
        }

        static std::unique_ptr<StorageItemRoot> deserializeFromJson(const nlohmann::json &json, std::string_view key) {
            if (json.empty() || !json.is_object()) {
                return nullptr;
            }
            if (auto root = make_unique_nothrow<StorageItemRoot>(key)) {
                if (deserializeFromJson(json, *root)) {
                    return root;
                }
            }
            return nullptr;
        }

        static nlohmann::json serializeToJson(const StorageSubItem &baseItem) {
            nlohmann::json json = nlohmann::json::object();

            for (const auto &curSubItem : baseItem.getSubItems()) {
                json[curSubItem.getKey()] = serializeToJson(curSubItem);
            }

            for (const auto &[key, value] : baseItem.getItems()) {
                switch ((StorageItemType) value.getType()) {
                    case StorageItemType::String: {
                        std::string res;
                        if (value.getValue(res)) {
                            json[key] = res;
                        }
                        break;
                    }
                    case StorageItemType::Boolean: {
                        bool res;
                        if (value.getValue(res)) {
                            json[key] = res;
                        }
                        break;
                    }
                    case StorageItemType::S64: {
                        int64_t res;
                        if (value.getValue(res)) {
                            json[key] = res;
                        }
                        break;
                    }
                    case StorageItemType::U64: {
                        uint64_t res;
                        if (value.getValue(res)) {
                            json[key] = res;
                        }
                        break;
                    }
                    case StorageItemType::Double: {
                        double res;
                        if (value.getValue(res)) {
                            json[key] = res;
                        }
                        break;
                    }
                    case StorageItemType::Binary: {
                        std::vector<uint8_t> tmp;
                        if (value.getValue(tmp)) {
                            if (auto *enc = b64_encode(tmp.data(), tmp.size())) {
                                json[key] = enc;
                                free(enc);
                            } else {
                                DEBUG_FUNCTION_LINE_WARN("Failed to store binary item: Malloc failed");
                            }
                        }
                        break;
                    }
                    case StorageItemType::None:
                        DEBUG_FUNCTION_LINE_WARN("Skip: StorageItemType::None");
                        break;
                }
            }

            return json;
        }

        static StorageItemRoot *getRootItem(wups_storage_root_item root) {
            for (auto &cur : gStorage) {
                if (cur.getHandle() == reinterpret_cast<uint32_t>(root)) {
                    return &cur;
                }
            }

            return nullptr;
        }

        static StorageSubItem *getSubItem(wups_storage_root_item root, wups_storage_item parent) {
            if (auto rootItem = getRootItem(root)) {
                if (parent == nullptr) {
                    return rootItem;
                }
                return rootItem->getSubItem(parent);
            }
            return nullptr;
        }

        WUPSStorageError LoadFromFile(std::string_view plugin_id, nlohmann::json &outJson) {
            const std::string filePath = getPluginPath() + "/config/" + plugin_id.data() + ".json";
            if (ParseJsonFromFile(filePath, outJson)) {
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return WUPS_STORAGE_ERROR_IO_ERROR;
        }

        WUPSStorageError LoadFromFile(std::string_view plugin_id, StorageItemRoot &rootItem) {
            nlohmann::json j;

            if (WUPSStorageError err; (err = LoadFromFile(plugin_id, j)) != WUPS_STORAGE_ERROR_SUCCESS) {
                return err;
            }

            std::unique_ptr<StorageItemRoot> storage;
            if (j.empty() || !j.is_object() || !j.contains("storageitems") || j["storageitems"].empty() || !j["storageitems"].is_object()) {
                storage = make_unique_nothrow<StorageItemRoot>(plugin_id);
            } else {
                storage = StorageUtils::Helper::deserializeFromJson(j["storageitems"], plugin_id);
                if (!storage) {
                    storage = make_unique_nothrow<StorageItemRoot>(plugin_id);
                }
            }
            rootItem = std::move(*storage);
            return WUPS_STORAGE_ERROR_SUCCESS;
        }

        static WUPSStorageError WriteStorageToSD(wups_storage_root_item root, bool forceSave) {
            const StorageItemRoot *rootItem = nullptr;
            for (const auto &cur : gStorage) {
                if (cur.getHandle() == reinterpret_cast<uint32_t>(root)) {
                    rootItem = &cur;
                    break;
                }
            }
            if (!rootItem) {
                return WUPS_STORAGE_ERROR_INTERNAL_NOT_INITIALIZED;
            }

            const std::string folderPath = getPluginPath() + "/config/";
            const std::string filePath   = folderPath + rootItem->getPluginId() + ".json";

            nlohmann::json j;
            j["storageitems"] = serializeToJson(*rootItem);

            if (!forceSave) {
                nlohmann::json jsonFromFile;
                WUPSStorageError loadErr;
                if ((loadErr = Helper::LoadFromFile(rootItem->getPluginId(), jsonFromFile)) == WUPS_STORAGE_ERROR_SUCCESS) {
                    if (j == jsonFromFile) {
                        DEBUG_FUNCTION_LINE_VERBOSE("Storage has no changes, avoid saving \"%s.json\"", rootItem->getPluginId().c_str());
                        return WUPS_STORAGE_ERROR_SUCCESS;
                    }
                } else if (loadErr != WUPS_STORAGE_ERROR_NOT_FOUND) {
                    DEBUG_FUNCTION_LINE_WARN("Failed to load \"%s.json\"", rootItem->getPluginId().c_str());
                }
                DEBUG_FUNCTION_LINE_VERBOSE("Saving \"%s.json\"...", rootItem->getPluginId().c_str());
            } else {
                DEBUG_FUNCTION_LINE_VERBOSE("Force saving \"%s.json\"...", rootItem->getPluginId().c_str());
            }

            if (!FSUtils::CreateSubfolder(folderPath)) {
                return WUPS_STORAGE_ERROR_IO_ERROR;
            }

            CFile file(filePath, CFile::WriteOnly);
            if (!file.isOpen()) {
                DEBUG_FUNCTION_LINE_ERR("Cannot create file %s", filePath.c_str());
                return WUPS_STORAGE_ERROR_IO_ERROR;
            }

            const std::string jsonString = j.dump(4, ' ', false, nlohmann::json::error_handler_t::ignore);
            const auto writeResult       = file.write(reinterpret_cast<const uint8_t *>(jsonString.c_str()), jsonString.size());

            file.close();

            if (writeResult != static_cast<int32_t>(jsonString.size())) {
                return WUPS_STORAGE_ERROR_IO_ERROR;
            }
            return WUPS_STORAGE_ERROR_SUCCESS;
        }

        static StorageItem *createOrGetItem(wups_storage_root_item root, wups_storage_item parent, const char *key, WUPSStorageError &error) {
            const auto subItem = getSubItem(root, parent);
            if (!subItem) {
                error = WUPS_STORAGE_ERROR_NOT_FOUND;
                return {};
            }
            auto res                                         = subItem->getItem(key);
            StorageSubItem::StorageSubItemError subItemError = StorageSubItem::STORAGE_SUB_ITEM_ERROR_NONE;
            if (!res) {
                res = subItem->createItem(key, subItemError);
                if (!res) {
                    error = ConvertToWUPSError(subItemError);
                }
            }
            if (res) {
                error = WUPS_STORAGE_ERROR_SUCCESS;
            }
            return res;
        }

        template<typename T>
        WUPSStorageError StoreItemGeneric(wups_storage_root_item root, wups_storage_item parent, const char *key, const T &value) {
            WUPSStorageError err;
            if (auto item = createOrGetItem(root, parent, key, err); item && err == WUPS_STORAGE_ERROR_SUCCESS) {
                item->setValue(value);
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return err;
        }

        template<typename T>
        WUPSStorageError GetItemEx(wups_storage_root_item root, wups_storage_item parent, const char *key, T &result) {
            const auto subItem = getSubItem(root, parent);
            if (!subItem) {
                return WUPS_STORAGE_ERROR_NOT_FOUND;
            }
            if (auto item = subItem->getItem(key)) {
                if (item->getValue(result)) {
                    return WUPS_STORAGE_ERROR_SUCCESS;
                }
                return WUPS_STORAGE_ERROR_UNEXPECTED_DATA_TYPE;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        template<typename T>
        WUPSStorageError GetItemGeneric(wups_storage_root_item root, wups_storage_item parent, const char *key, T *result, uint32_t *outSize) {
            if (!result) {
                return WUPS_STORAGE_ERROR_INVALID_ARGS;
            }
            T tmp;
            auto res = GetItemEx<T>(root, parent, key, tmp);
            if (res == WUPS_STORAGE_ERROR_SUCCESS) {
                *result = tmp;
                if (outSize) {
                    *outSize = sizeof(T);
                }
            }
            return res;
        }


        /**
        * Binary items are serialized as base64 encoded string. The first time they are read they'll get converted into binary data.
        */
        WUPSStorageError GetAndFixBinaryItem(wups_storage_root_item root, wups_storage_item parent, const char *key, std::vector<uint8_t> &result) {
            const auto subItem = getSubItem(root, parent);
            if (!subItem) {
                return WUPS_STORAGE_ERROR_NOT_FOUND;
            }
            if (const auto item = subItem->getItem(key)) {
                // Trigger potential string->binary conversion
                if (!item->attemptBinaryConversion()) {
                    return WUPS_STORAGE_ERROR_MALLOC_FAILED;
                }
                if (item->getValue(result)) {
                    return WUPS_STORAGE_ERROR_SUCCESS;
                }
                return WUPS_STORAGE_ERROR_UNEXPECTED_DATA_TYPE;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        static WUPSStorageError GetStringItem(wups_storage_root_item root, wups_storage_item parent, const char *key, void *data, uint32_t maxSize, uint32_t *outSize) {
            std::string tmp;
            const auto res = GetItemEx<std::string>(root, parent, key, tmp);
            if (res == WUPS_STORAGE_ERROR_SUCCESS) {
                if (maxSize <= tmp.size()) { // maxSize needs to be bigger because of the null-terminator
                    return WUPS_STORAGE_ERROR_BUFFER_TOO_SMALL;
                }
                strncpy(static_cast<char *>(data), tmp.c_str(), tmp.size());
                static_cast<char *>(data)[maxSize - 1] = '\0';
                if (outSize) {
                    *outSize = strlen(static_cast<char *>(data)) + 1;
                }
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return res;
        }

        static WUPSStorageError GetBinaryItem(wups_storage_root_item root, wups_storage_item parent, const char *key, const void *data, uint32_t maxSize, uint32_t *outSize) {
            std::vector<uint8_t> tmp;
            const auto res = GetAndFixBinaryItem(root, parent, key, tmp);
            if (res == WUPS_STORAGE_ERROR_SUCCESS) {
                if (tmp.empty()) { // we need this to support getting empty std::vector
                    return WUPS_STORAGE_ERROR_SUCCESS;
                }
                if (data == nullptr) {
                    return WUPS_STORAGE_ERROR_INVALID_ARGS;
                }
                if (maxSize < tmp.size()) {
                    return WUPS_STORAGE_ERROR_BUFFER_TOO_SMALL;
                }
                memcpy((uint8_t *) data, tmp.data(), tmp.size());
                if (outSize) {
                    *outSize = tmp.size();
                }
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return res;
        }
    } // namespace Helper

    namespace API {
        namespace Internal {
            WUPSStorageError OpenStorage(std::string_view plugin_id, wups_storage_root_item &outItem) {
                std::lock_guard lock(gStorageMutex);
                gStorage.emplace_front(plugin_id);
                auto &root = gStorage.front();

                if (const WUPSStorageError err = Helper::LoadFromFile(plugin_id, root); err == WUPS_STORAGE_ERROR_NOT_FOUND) {
                    // Create new clean StorageItemRoot if no existing storage was found
                    root = StorageItemRoot(plugin_id);
                } else if (err != WUPS_STORAGE_ERROR_SUCCESS) {
                    // Return on any other error
                    gStorage.pop_front();
                    return err;
                }

                outItem = reinterpret_cast<wups_storage_root_item>(root.getHandle());

                return WUPS_STORAGE_ERROR_SUCCESS;
            }

            WUPSStorageError CloseStorage(wups_storage_root_item root) {
                std::lock_guard lock(gStorageMutex);

                const auto res = Helper::WriteStorageToSD(root, false);
                // TODO: handle write error?

                if (!remove_first_if(gStorage, [&root](auto &cur) { return cur.getHandle() == reinterpret_cast<uint32_t>(root); })) {
                    DEBUG_FUNCTION_LINE_WARN("Failed to close storage: Not opened (\"%08X\")", root);
                    return WUPS_STORAGE_ERROR_NOT_FOUND;
                }
                return res;
            }
        } // namespace Internal

        WUPSStorageError SaveStorage(wups_storage_root_item root, const bool force) {
            std::lock_guard lock(gStorageMutex);
            return StorageUtils::Helper::WriteStorageToSD(root, force);
        }

        WUPSStorageError ForceReloadStorage(wups_storage_root_item root) {
            std::lock_guard lock(gStorageMutex);

            const auto rootItem = Helper::getRootItem(root);
            if (!rootItem) {
                return WUPS_STORAGE_ERROR_INTERNAL_NOT_INITIALIZED;
            }

            WUPSStorageError result;
            if ((result = Helper::LoadFromFile(rootItem->getPluginId(), *rootItem)) != WUPS_STORAGE_ERROR_SUCCESS) {
                return result;
            }
            return result;
        }

        WUPSStorageError WipeStorage(wups_storage_root_item root) {
            std::lock_guard lock(gStorageMutex);

            const auto rootItem = Helper::getRootItem(root);
            if (!rootItem) {
                return WUPS_STORAGE_ERROR_NOT_FOUND;
            }

            rootItem->wipe();

            return WUPS_STORAGE_ERROR_SUCCESS;
        }

        WUPSStorageError CreateSubItem(wups_storage_root_item root, wups_storage_item parent, const char *key, wups_storage_item *outItem) {
            if (!outItem) {
                return WUPS_STORAGE_ERROR_INVALID_ARGS;
            }
            std::lock_guard lock(gStorageMutex);
            if (const auto subItem = StorageUtils::Helper::getSubItem(root, parent)) {
                StorageSubItem::StorageSubItemError error = StorageSubItem::STORAGE_SUB_ITEM_ERROR_NONE;
                const auto res                            = subItem->createSubItem(key, error);
                if (!res) {
                    return StorageUtils::Helper::ConvertToWUPSError(error);
                }
                *outItem = reinterpret_cast<wups_storage_item>(res->getHandle());
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        WUPSStorageError GetSubItem(wups_storage_root_item root, wups_storage_item parent, const char *key, wups_storage_item *outItem) {
            if (!outItem) {
                return WUPS_STORAGE_ERROR_INVALID_ARGS;
            }
            std::lock_guard lock(gStorageMutex);
            if (const auto subItem = StorageUtils::Helper::getSubItem(root, parent)) {
                const auto res = subItem->getSubItem(key);
                if (!res) {
                    return WUPS_STORAGE_ERROR_NOT_FOUND;
                }
                *outItem = reinterpret_cast<wups_storage_item>(res->getHandle());
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        WUPSStorageError DeleteItem(wups_storage_root_item root, wups_storage_item parent, const char *key) {
            std::lock_guard lock(gStorageMutex);
            if (const auto subItem = StorageUtils::Helper::getSubItem(root, parent)) {
                if (const auto res = subItem->deleteItem(key); !res) {
                    return WUPS_STORAGE_ERROR_NOT_FOUND;
                }
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        WUPSStorageError GetItemSize(wups_storage_root_item root, wups_storage_item parent, const char *key, const WUPSStorageItemType itemType, uint32_t *outSize) {
            if (!outSize) {
                return WUPS_STORAGE_ERROR_INVALID_ARGS;
            }
            if (itemType != WUPS_STORAGE_ITEM_STRING && itemType != WUPS_STORAGE_ITEM_BINARY) {
                return WUPS_STORAGE_ERROR_UNEXPECTED_DATA_TYPE;
            }
            std::lock_guard lock(gStorageMutex);
            const auto subItem = StorageUtils::Helper::getSubItem(root, parent);
            if (!subItem) {
                return WUPS_STORAGE_ERROR_NOT_FOUND;
            }
            if (const auto item = subItem->getItem(key)) {
                if (itemType == WUPS_STORAGE_ITEM_BINARY) {
                    // Trigger potential string -> binary conversion.
                    if (!item->attemptBinaryConversion()) {
                        return WUPS_STORAGE_ERROR_MALLOC_FAILED;
                    }
                }
                uint32_t tmp = 0;
                bool res     = false;
                if (itemType == WUPS_STORAGE_ITEM_STRING) {
                    res = item->getItemSizeString(tmp);
                } else if (itemType == WUPS_STORAGE_ITEM_BINARY) {
                    res = item->getItemSizeBinary(tmp);
                }
                if (res) {
                    *outSize = tmp;
                    return WUPS_STORAGE_ERROR_SUCCESS;
                }
                DEBUG_FUNCTION_LINE_WARN("Failed to get size for item %s", key);
                return WUPS_STORAGE_ERROR_UNEXPECTED_DATA_TYPE;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        WUPSStorageError StoreItem(wups_storage_root_item root, wups_storage_item parent, const char *key, WUPSStorageItemType itemType, void *data, const uint32_t length) {
            std::lock_guard lock(gStorageMutex);
            switch (static_cast<WUPSStorageItemTypes>(itemType)) {
                case WUPS_STORAGE_ITEM_S32: {
                    if (data == nullptr || length != sizeof(int32_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as S32: %d", key, *(int32_t *) data);
                    return StorageUtils::Helper::StoreItemGeneric<int32_t>(root, parent, key, *static_cast<int32_t *>(data));
                }
                case WUPS_STORAGE_ITEM_S64: {
                    if (data == nullptr || length != sizeof(int64_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as S64: %lld", key, *(int64_t *) data);
                    return StorageUtils::Helper::StoreItemGeneric<int64_t>(root, parent, key, *static_cast<int64_t *>(data));
                }
                case WUPS_STORAGE_ITEM_U32: {
                    if (data == nullptr || length != sizeof(uint32_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as u32: %u", key, *(uint32_t *) data);
                    return StorageUtils::Helper::StoreItemGeneric<uint32_t>(root, parent, key, *static_cast<uint32_t *>(data));
                }
                case WUPS_STORAGE_ITEM_U64: {
                    if (data == nullptr || length != sizeof(uint64_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as u64: %llu", key, *(uint64_t *) data);
                    return StorageUtils::Helper::StoreItemGeneric<uint64_t>(root, parent, key, *static_cast<uint64_t *>(data));
                }
                case WUPS_STORAGE_ITEM_STRING: {
                    if (data == nullptr) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    const std::string tmp = length > 0 ? std::string(static_cast<const char *>(data), length) : std::string();

                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as string: %s", key, tmp.c_str());
                    return StorageUtils::Helper::StoreItemGeneric<std::string>(root, parent, key, tmp);
                }
                case WUPS_STORAGE_ITEM_BINARY: {
                    if (data == nullptr && length > 0) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    const std::vector<uint8_t> tmp = (data != nullptr && length > 0) ? std::vector(static_cast<uint8_t *>(data), static_cast<uint8_t *>(data) + length) : std::vector<uint8_t>();

                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as binary: size %d", key, tmp.size());
                    return StorageUtils::Helper::StoreItemGeneric<std::vector<uint8_t>>(root, parent, key, tmp);
                }
                case WUPS_STORAGE_ITEM_BOOL: {
                    if (data == nullptr || length != sizeof(bool)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as bool: %d", key, *(bool *) data);
                    return StorageUtils::Helper::StoreItemGeneric<bool>(root, parent, key, *static_cast<bool *>(data));
                }
                case WUPS_STORAGE_ITEM_FLOAT: {
                    if (data == nullptr || length != sizeof(float)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as float: %f", key, *(float *) data);
                    return StorageUtils::Helper::StoreItemGeneric<float>(root, parent, key, *static_cast<float *>(data));
                }
                case WUPS_STORAGE_ITEM_DOUBLE: {
                    if (data == nullptr || length != sizeof(double)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as double: %f", key, *(double *) data);
                    return StorageUtils::Helper::StoreItemGeneric<double>(root, parent, key, *static_cast<double *>(data));
                }
            }
            DEBUG_FUNCTION_LINE_ERR("Store failed!");
            return WUPS_STORAGE_ERROR_UNEXPECTED_DATA_TYPE;
        }

        WUPSStorageError GetItem(wups_storage_root_item root,
                                 wups_storage_item parent,
                                 const char *key,
                                 WUPSStorageItemType itemType,
                                 void *data,
                                 const uint32_t maxSize,
                                 uint32_t *outSize) {
            std::lock_guard lock(gStorageMutex);
            if (outSize) {
                *outSize = 0;
            }
            switch (static_cast<WUPSStorageItemTypes>(itemType)) {
                case WUPS_STORAGE_ITEM_STRING: {
                    if (!data || maxSize == 0) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetStringItem(root, parent, key, data, maxSize, outSize);
                }
                case WUPS_STORAGE_ITEM_BINARY: {
                    return StorageUtils::Helper::GetBinaryItem(root, parent, key, data, maxSize, outSize);
                }
                case WUPS_STORAGE_ITEM_BOOL: {
                    if (!data || maxSize != sizeof(bool)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<bool>(root, parent, key, static_cast<bool *>(data), outSize);
                }
                case WUPS_STORAGE_ITEM_S32: {
                    if (!data || maxSize != sizeof(int32_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<int32_t>(root, parent, key, static_cast<int32_t *>(data), outSize);
                }
                case WUPS_STORAGE_ITEM_S64: {
                    if (!data || maxSize != sizeof(int64_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<int64_t>(root, parent, key, static_cast<int64_t *>(data), outSize);
                }
                case WUPS_STORAGE_ITEM_U32: {
                    if (!data || maxSize != sizeof(uint32_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<uint32_t>(root, parent, key, static_cast<uint32_t *>(data), outSize);
                }
                case WUPS_STORAGE_ITEM_U64: {
                    if (!data || maxSize != sizeof(uint64_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<uint64_t>(root, parent, key, static_cast<uint64_t *>(data), outSize);
                }
                case WUPS_STORAGE_ITEM_FLOAT: {
                    if (!data || maxSize != sizeof(float)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<float>(root, parent, key, static_cast<float *>(data), outSize);
                }
                case WUPS_STORAGE_ITEM_DOUBLE: {
                    if (!data || maxSize != sizeof(double)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<double>(root, parent, key, static_cast<double *>(data), outSize);
                }
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }
    } // namespace API
} // namespace StorageUtils