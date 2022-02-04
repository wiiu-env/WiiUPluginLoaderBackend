#include "StorageUtils.h"
#include <string>

#include "fs/CFile.hpp"
#include "fs/FSUtils.h"
#include "utils/json.hpp"
#include "utils/logger.h"

static void processJson(wups_storage_item_t *items, nlohmann::json json) {
    if (items == nullptr) {
        return;
    }

    items->data      = (wups_storage_item_t *) malloc(json.size() * sizeof(wups_storage_item_t));
    items->data_size = json.size();

    uint32_t index = 0;
    for (auto it = json.begin(); it != json.end(); ++it) {
        wups_storage_item_t *item = &((wups_storage_item_t *) items->data)[index];
        item->type                = WUPS_STORAGE_TYPE_INVALID;
        item->pending_delete      = false;
        item->data                = nullptr;
        item->key                 = nullptr;

        item->key = (char *) malloc(it.key().size() + 1);
        strcpy(item->key, it.key().c_str());

        if (it.value().is_string()) {
            item->type      = WUPS_STORAGE_TYPE_STRING;
            uint32_t size   = it.value().get<std::string>().size() + 1;
            item->data      = malloc(size);
            item->data_size = size;
            strcpy((char *) item->data, it.value().get<std::string>().c_str());
        } else if (it.value().is_number_integer()) {
            item->type              = WUPS_STORAGE_TYPE_INT;
            item->data              = malloc(sizeof(int32_t));
            item->data_size         = sizeof(int32_t);
            *(int32_t *) item->data = it.value().get<int32_t>();
        } else if (it.value().is_object()) {
            if (it.value().size() > 0) {
                item->type = WUPS_STORAGE_TYPE_ITEM;
                processJson(item, it.value());
            }
        } else {
            DEBUG_FUNCTION_LINE("Unknown type %s for value %s", it.value().type_name(), it.key().c_str());
        }
        index++;
    }
}

int StorageUtils::OpenStorage(const char *plugin_id, wups_storage_item_t *items) {
    if (!plugin_id || !items) {
        return WUPS_STORAGE_ERROR_INVALID_BACKEND_PARAMS;
    }

    std::string filePath = std::string("fs:/vol/external01/wiiu/plugins/config/") + plugin_id + ".json";

    nlohmann::json j;
    CFile file(filePath, CFile::ReadOnly);
    if (file.isOpen() && file.size() > 0) {
        uint8_t *json_data     = new uint8_t[file.size() + 1];
        json_data[file.size()] = '\0';

        file.read(json_data, file.size());
        file.close();

        j = nlohmann::json::parse(json_data, nullptr, false);
        delete[] json_data;

        if (j == nlohmann::detail::value_t::discarded || j.empty() || !j.is_object()) {
            return WUPS_STORAGE_ERROR_INVALID_JSON;
        }
    } else { // empty or no config exists yet
        DEBUG_FUNCTION_LINE("open failed");
        return WUPS_STORAGE_ERROR_SUCCESS;
    }

    processJson(items, j["storageitems"]);

    return WUPS_STORAGE_ERROR_SUCCESS;
}

static nlohmann::json processItems(wups_storage_item_t *items) {
    nlohmann::json json;

    if (!items) {
        return json;
    }

    for (uint32_t i = 0; i < items->data_size; i++) {
        wups_storage_item_t *item = &((wups_storage_item_t *) items->data)[i];

        if (item->pending_delete || item->type == WUPS_STORAGE_TYPE_INVALID || !item->data || !item->key) {
            continue;
        }

        if (item->type == WUPS_STORAGE_TYPE_STRING) {
            json[item->key] = (const char *) item->data;
        } else if (item->type == WUPS_STORAGE_TYPE_INT) {
            json[item->key] = *(int32_t *) item->data;
        } else if (item->type == WUPS_STORAGE_TYPE_ITEM) {
            json[item->key] = processItems(item);
        } else {
            DEBUG_FUNCTION_LINE("Saving type %d not implemented", item->type);
        }
    }
    return json;
}

int StorageUtils::CloseStorage(const char *plugin_id, wups_storage_item_t *items) {
    if (!plugin_id || !items) {
        return WUPS_STORAGE_ERROR_INVALID_BACKEND_PARAMS;
    }

    std::string folderPath = "fs:/vol/external01/wiiu/plugins/config/";
    std::string filePath   = folderPath + plugin_id + ".json";

    FSUtils::CreateSubfolder(folderPath.c_str());

    CFile file(filePath, CFile::WriteOnly);
    if (!file.isOpen()) {
        DEBUG_FUNCTION_LINE("Cannot create file %s", filePath.c_str());
        return WUPS_STORAGE_ERROR_IO;
    };

    nlohmann::json j;
    j["storageitems"] = processItems(items);

    std::string jsonString = j.dump(4);
    file.write((const uint8_t *) jsonString.c_str(), jsonString.size());
    file.close();
    return WUPS_STORAGE_ERROR_SUCCESS;
}
