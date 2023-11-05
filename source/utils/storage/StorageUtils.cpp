#include "StorageUtils.h"
#include "NotificationsUtils.h"
#include "StorageItemRoot.h"
#include "fs/CFile.hpp"
#include "fs/FSUtils.h"
#include "utils/StringTools.h"
#include "utils/base64.h"
#include "utils/json.hpp"
#include "utils/logger.h"
#include "utils/utils.h"
#include <memory>
#include <string>
namespace StorageUtils {
    std::forward_list<std::shared_ptr<StorageItemRoot>> gStorage;
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

        static std::unique_ptr<StorageItemRoot> deserializeFromJson(const nlohmann::json &json, const std::string &key) {
            if (json.empty() || !json.is_object()) {
                return nullptr;
            }
            auto root = make_unique_nothrow<StorageItemRoot>(key);
            if (root) {
                if (deserializeFromJson(json, *root)) {
                    return root;
                }
            }
            return nullptr;
        }

        static nlohmann::json serializeToJson(const StorageSubItem &baseItem) {
            nlohmann::json json = nlohmann::json::object();

            for (const auto &curSubItem : baseItem.getSubItems()) {
                json[curSubItem->getKey()] = serializeToJson(*curSubItem);
            }

            for (const auto &[key, value] : baseItem.getItems()) {
                switch ((StorageItemType) value->getType()) {
                    case StorageItemType::String: {
                        std::string res;
                        if (value->getValue(res)) {
                            json[key] = res;
                        }
                        break;
                    }
                    case StorageItemType::Boolean: {
                        bool res;
                        if (value->getValue(res)) {
                            json[key] = res;
                        }
                        break;
                    }
                    case StorageItemType::S64: {
                        int64_t res;
                        if (value->getValue(res)) {
                            json[key] = res;
                        }
                        break;
                    }
                    case StorageItemType::U64: {
                        uint64_t res;
                        if (value->getValue(res)) {
                            json[key] = res;
                        }
                        break;
                    }
                    case StorageItemType::Double: {
                        double res;
                        if (value->getValue(res)) {
                            json[key] = res;
                        }
                        break;
                    }
                    case StorageItemType::Binary: {
                        std::vector<uint8_t> tmp;
                        if (value->getValue(tmp)) {
                            auto *enc = b64_encode(tmp.data(), tmp.size());
                            if (enc) {
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

        static StorageSubItem *getRootItem(wups_storage_root_item root) {
            for (const auto &cur : gStorage) {
                if (cur->getHandle() == (uint32_t) root) {
                    return cur.get();
                }
            }

            return nullptr;
        }

        static StorageSubItem *getSubItem(wups_storage_root_item root, wups_storage_item parent) {
            auto rootItem = getRootItem(root);
            if (rootItem) {
                if (parent == nullptr) {
                    return rootItem;
                }
                return rootItem->getSubItem(parent);
            }
            return nullptr;
        }


        static WUPSStorageError WriteStorageToSD(const char *plugin_id) {
            std::shared_ptr<StorageItemRoot> rootItem;
            for (const auto &cur : gStorage) {
                if (cur->getPluginId() == plugin_id) {
                    rootItem = cur;
                    break;
                }
            }
            if (!rootItem) {
                return WUPS_STORAGE_ERROR_NOT_FOUND;
            }

            std::string folderPath = getPluginPath() + "/config/";
            std::string filePath   = folderPath + rootItem->getPluginId() + ".json";

            FSUtils::CreateSubfolder(folderPath);

            CFile file(filePath, CFile::WriteOnly);
            if (!file.isOpen()) {
                DEBUG_FUNCTION_LINE_ERR("Cannot create file %s", filePath.c_str());
                return WUPS_STORAGE_ERROR_SUCCESS;
            }

            nlohmann::json j;
            j["storageitems"] = serializeToJson(*rootItem);

            std::string jsonString = j.dump(4, ' ', false, nlohmann::json::error_handler_t::ignore);
            auto writeResult       = file.write((const uint8_t *) jsonString.c_str(), jsonString.size());

            file.close();

            if (writeResult != (int32_t) jsonString.size()) {
                return WUPS_STORAGE_ERROR_SUCCESS;
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
                if (!(res = subItem->createItem(key, subItemError))) {
                    error = StorageUtils::Helper::ConvertToWUPSError(subItemError);
                }
            }
            if (res) {
                error = WUPS_STORAGE_ERROR_SUCCESS;
            }
            return res;
        }

        template<typename T>
        WUPSStorageError StoreItemGeneric(wups_storage_root_item root, wups_storage_item parent, const char *key, T value) {
            WUPSStorageError err;
            auto item = createOrGetItem(root, parent, key, err);
            if (item && err == WUPS_STORAGE_ERROR_SUCCESS) {
                item->setValue(value);
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return err;
        }

        template<typename T>
        WUPSStorageError GetItemEx(wups_storage_root_item root, wups_storage_item parent, const char *key, T &result) {
            auto subItem = getSubItem(root, parent);
            if (!subItem) {
                return WUPS_STORAGE_ERROR_NOT_FOUND;
            }
            auto item = subItem->getItem(key);
            if (item) {
                if (item->getValue(result)) {
                    return WUPS_STORAGE_ERROR_SUCCESS;
                }
                DEBUG_FUNCTION_LINE_ERR("GetValue failed? %s", key);
                return WUPS_STORAGE_ERROR_UNEXPECTED_DATA_TYPE;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        template<typename T>
        WUPSStorageError GetItemGeneric(wups_storage_root_item root, wups_storage_item parent, const char *key, T *result) {
            if (!result) {
                return WUPS_STORAGE_ERROR_INVALID_ARGS;
            }
            T tmp;
            auto res = GetItemEx<T>(root, parent, key, tmp);
            if (res == WUPS_STORAGE_ERROR_SUCCESS) {
                *result = tmp;
            }
            return res;
        }


        /**
        * Binary items are serialized as base64 encoded string. The first time they are read they'll get converted into binary data.
        */
        WUPSStorageError GetAndFixBinaryItem(wups_storage_root_item root, wups_storage_item parent, const char *key, std::vector<uint8_t> &result) {
            auto subItem = getSubItem(root, parent);
            if (!subItem) {
                return WUPS_STORAGE_ERROR_NOT_FOUND;
            }
            WUPSStorageError err = WUPS_STORAGE_ERROR_UNEXPECTED_DATA_TYPE;
            auto item            = subItem->getItem(key);
            if (item) {
                // Trigger potential string->binary conversion
                if (!item->attemptBinaryConversion()) {
                    return WUPS_STORAGE_ERROR_MALLOC_FAILED;
                }
                if (item->getValue(result)) {
                    return WUPS_STORAGE_ERROR_SUCCESS;
                }
                return err;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        static WUPSStorageError GetStringItem(wups_storage_root_item root, wups_storage_item parent, const char *key, void *data, uint32_t maxSize, uint32_t *outSize) {
            std::string tmp;
            auto res = GetItemEx<std::string>(root, parent, key, tmp);
            if (res == WUPS_STORAGE_ERROR_SUCCESS) {
                if (maxSize <= tmp.size()) { // maxSize needs to be bigger because of the null-terminator
                    return WUPS_STORAGE_ERROR_BUFFER_TOO_SMALL;
                }
                strncpy((char *) data, tmp.c_str(), tmp.size());
                ((char *) data)[maxSize - 1] = '\0';
                if (outSize) {
                    *outSize = strlen((char *) data) + 1;
                }
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return res;
        }

        static WUPSStorageError GetBinaryItem(wups_storage_root_item root, wups_storage_item parent, const char *key, const void *data, uint32_t maxSize, uint32_t *outSize) {
            std::vector<uint8_t> tmp;
            auto res = GetAndFixBinaryItem(root, parent, key, tmp);
            if (res == WUPS_STORAGE_ERROR_SUCCESS) {
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
        WUPSStorageError OpenStorage(const char *plugin_id, wups_storage_root_item *item) {
            std::lock_guard lock(gStorageMutex);
            // Check if we already have a storage with the given plugin id
            for (const auto &cur : gStorage) {
                if (cur->getPluginId() == plugin_id) {
                    *item = (wups_storage_root_item) cur->getHandle();
                    return WUPS_STORAGE_ERROR_ALREADY_OPENED;
                }
            }


            std::string filePath = getPluginPath() + "/config/" + plugin_id + ".json";
            nlohmann::json j;
            {
                CFile file(filePath, CFile::ReadOnly);
                if (file.isOpen() && file.size() > 0) {
                    auto *json_data = (uint8_t *) memalign(0x40, ROUNDUP(file.size() + 1, 0x40));
                    if (!json_data) {
                        DEBUG_FUNCTION_LINE_WARN("Failed to create StorageItem: Malloc failed");
                        return WUPS_STORAGE_ERROR_MALLOC_FAILED;
                    }

                    file.read(json_data, file.size());

                    json_data[file.size()] = '\0';

                    file.close();
                    j = nlohmann::json::parse(json_data, nullptr, false);
                    free(json_data);

                    if (j == nlohmann::detail::value_t::discarded || j.empty() || !j.is_object()) {
                        std::string errorMessage = string_format("Corrupted plugin storage detected: \"%s\". You have to reconfigure the plugin.", plugin_id);
                        DEBUG_FUNCTION_LINE_ERR("%s", errorMessage.c_str());
                        remove(filePath.c_str());

                        DisplayErrorNotificationMessage(errorMessage, 10.0f);
                    }
                }
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

            if (!storage) {
                return WUPS_STORAGE_ERROR_MALLOC_FAILED;
            }

            *item = (wups_storage_root_item) storage->getHandle();
            gStorage.push_front(std::move(storage));

            return WUPS_STORAGE_ERROR_SUCCESS;
        }

        WUPSStorageError CloseStorage(const char *plugin_id) {
            std::lock_guard lock(gStorageMutex);

            auto res = StorageUtils::Helper::WriteStorageToSD(plugin_id);
            // TODO: handle write error?

            if (!remove_locked_first_if(gStorageMutex, gStorage, [plugin_id](auto &cur) { return cur->getPluginId() == plugin_id; })) {
                DEBUG_FUNCTION_LINE_WARN("Failed to close storage: Not opened (\"%s\")", plugin_id);
                return WUPS_STORAGE_ERROR_NOT_FOUND;
            }
            return res;
        }

        WUPSStorageError CreateSubItem(wups_storage_root_item root, wups_storage_item parent, const char *key, wups_storage_item *outItem) {
            if (!outItem) {
                return WUPS_STORAGE_ERROR_INVALID_ARGS;
            }
            std::lock_guard lock(gStorageMutex);
            auto subItem = StorageUtils::Helper::getSubItem(root, parent);
            if (subItem) {
                StorageSubItem::StorageSubItemError error = StorageSubItem::STORAGE_SUB_ITEM_ERROR_NONE;
                auto res                                  = subItem->createSubItem(key, error);
                if (!res) {
                    return StorageUtils::Helper::ConvertToWUPSError(error);
                }
                *outItem = (wups_storage_item) res->getHandle();
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        WUPSStorageError GetSubItem(wups_storage_root_item root, wups_storage_item parent, const char *key, wups_storage_item *outItem) {
            if (!outItem) {
                return WUPS_STORAGE_ERROR_INVALID_ARGS;
            }
            std::lock_guard lock(gStorageMutex);
            auto subItem = StorageUtils::Helper::getSubItem(root, parent);
            if (subItem) {
                auto res = subItem->getSubItem(key);
                if (!res) {
                    return WUPS_STORAGE_ERROR_NOT_FOUND;
                }
                *outItem = (wups_storage_item) res->getHandle();
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        WUPSStorageError DeleteItem(wups_storage_root_item root, wups_storage_item parent, const char *key) {
            std::lock_guard lock(gStorageMutex);
            auto subItem = StorageUtils::Helper::getSubItem(root, parent);
            if (subItem) {
                auto res = subItem->deleteItem(key);
                if (!res) {
                    return WUPS_STORAGE_ERROR_NOT_FOUND;
                }
                return WUPS_STORAGE_ERROR_SUCCESS;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        WUPSStorageError GetItemSize(wups_storage_root_item root, wups_storage_item parent, const char *key, uint32_t *outSize) {
            if (!outSize) {
                return WUPS_STORAGE_ERROR_INVALID_ARGS;
            }
            std::lock_guard lock(gStorageMutex);
            auto subItem = StorageUtils::Helper::getSubItem(root, parent);
            if (!subItem) {
                return WUPS_STORAGE_ERROR_NOT_FOUND;
            }
            auto item = subItem->getItem(key);
            if (item) {
                // Trigger potential string -> binary conversion.
                if (!item->attemptBinaryConversion()) {
                    return WUPS_STORAGE_ERROR_MALLOC_FAILED;
                }
                uint32_t tmp = 0;
                if (item->getItemSize(tmp)) {
                    *outSize = tmp;
                    return WUPS_STORAGE_ERROR_SUCCESS;
                }
                return WUPS_STORAGE_ERROR_UNEXPECTED_DATA_TYPE;
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }

        WUPSStorageError StoreItem(wups_storage_root_item root, wups_storage_item parent, const char *key, WUPSStorageItemType itemType, void *data, uint32_t length) {
            std::lock_guard lock(gStorageMutex);
            switch ((WUPSStorageItemTypes) itemType) {
                case WUPS_STORAGE_ITEM_S32: {
                    if (data == nullptr || length != sizeof(int32_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as S32: %d", key, *(int32_t *) data);
                    return StorageUtils::Helper::StoreItemGeneric<int32_t>(root, parent, key, *(int32_t *) data);
                }
                case WUPS_STORAGE_ITEM_S64: {
                    if (data == nullptr || length != sizeof(int64_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as S64: %lld", key, *(int64_t *) data);
                    return StorageUtils::Helper::StoreItemGeneric<int64_t>(root, parent, key, *(int64_t *) data);
                }
                case WUPS_STORAGE_ITEM_U32: {
                    if (data == nullptr || length != sizeof(uint32_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as u32: %u", key, *(uint32_t *) data);
                    return StorageUtils::Helper::StoreItemGeneric<uint32_t>(root, parent, key, *(uint32_t *) data);
                }
                case WUPS_STORAGE_ITEM_U64: {
                    if (data == nullptr || length != sizeof(uint64_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as u64: %llu", key, *(uint64_t *) data);
                    return StorageUtils::Helper::StoreItemGeneric<uint64_t>(root, parent, key, *(uint64_t *) data);
                }
                case WUPS_STORAGE_ITEM_STRING: {
                    if (data == nullptr || length == 0) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    std::string tmp((const char *) data, length);

                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as string: %s", key, tmp.c_str());
                    return StorageUtils::Helper::StoreItemGeneric<std::string>(root, parent, key, tmp);
                }
                case WUPS_STORAGE_ITEM_BINARY: {
                    if (data == nullptr || length == 0) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    std::vector<uint8_t> tmp((uint8_t *) data, ((uint8_t *) data) + length);

                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as binary: size %d", key, tmp.size());
                    return StorageUtils::Helper::StoreItemGeneric<std::vector<uint8_t>>(root, parent, key, tmp);
                }
                case WUPS_STORAGE_ITEM_BOOL: {
                    if (data == nullptr || length != sizeof(bool)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as bool: %d", key, *(bool *) data);
                    return StorageUtils::Helper::StoreItemGeneric<bool>(root, parent, key, *(bool *) data);
                }
                case WUPS_STORAGE_ITEM_FLOAT: {
                    if (data == nullptr || length != sizeof(float)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as float: %f", key, *(float *) data);
                    return StorageUtils::Helper::StoreItemGeneric<float>(root, parent, key, *(float *) data);
                }
                case WUPS_STORAGE_ITEM_DOUBLE: {
                    if (data == nullptr || length != sizeof(double)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    DEBUG_FUNCTION_LINE_VERBOSE("Store %s as double: %f", key, *(double *) data);
                    return StorageUtils::Helper::StoreItemGeneric<double>(root, parent, key, *(double *) data);
                }
            }
            DEBUG_FUNCTION_LINE_ERR("Store failed!");
            return WUPS_STORAGE_ERROR_UNEXPECTED_DATA_TYPE;
        }

        WUPSStorageError GetItem(wups_storage_root_item root, wups_storage_item parent, const char *key, WUPSStorageItemType itemType, void *data, uint32_t maxSize, uint32_t *outSize) {
            std::lock_guard lock(gStorageMutex);
            switch ((WUPSStorageItemTypes) itemType) {
                case WUPS_STORAGE_ITEM_STRING: {
                    if (!data || maxSize == 0) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetStringItem(root, parent, key, data, maxSize, outSize);
                }
                case WUPS_STORAGE_ITEM_BINARY: {
                    if (!data || maxSize == 0) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetBinaryItem(root, parent, key, data, maxSize, outSize);
                }
                case WUPS_STORAGE_ITEM_BOOL: {
                    if (!data || maxSize != sizeof(bool)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<bool>(root, parent, key, (bool *) data);
                }
                case WUPS_STORAGE_ITEM_S32: {
                    if (!data || maxSize != sizeof(int32_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<int32_t>(root, parent, key, (int32_t *) data);
                }
                case WUPS_STORAGE_ITEM_S64: {
                    if (!data || maxSize != sizeof(int64_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<int64_t>(root, parent, key, (int64_t *) data);
                }
                case WUPS_STORAGE_ITEM_U32: {
                    if (!data || maxSize != sizeof(uint32_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<uint32_t>(root, parent, key, (uint32_t *) data);
                }
                case WUPS_STORAGE_ITEM_U64: {
                    if (!data || maxSize != sizeof(uint64_t)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<uint64_t>(root, parent, key, (uint64_t *) data);
                }
                case WUPS_STORAGE_ITEM_FLOAT: {
                    if (!data || maxSize != sizeof(float)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<float>(root, parent, key, (float *) data);
                }
                case WUPS_STORAGE_ITEM_DOUBLE: {
                    if (!data || maxSize != sizeof(double)) {
                        return WUPS_STORAGE_ERROR_INVALID_ARGS;
                    }
                    return StorageUtils::Helper::GetItemGeneric<double>(root, parent, key, (double *) data);
                }
            }
            return WUPS_STORAGE_ERROR_NOT_FOUND;
        }
    } // namespace API
} // namespace StorageUtils