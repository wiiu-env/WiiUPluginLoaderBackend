#pragma once

#include "StorageItem.h"
#include "utils/utils.h"
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include <wups/storage.h>

class StorageSubItem : public StorageItem {
public:
    enum StorageSubItemError {
        STORAGE_SUB_ITEM_ERROR_NONE          = 0,
        STORAGE_SUB_ITEM_ERROR_MALLOC_FAILED = 1,
        STORAGE_SUB_ITEM_KEY_ALREADY_IN_USE  = 2,
    };

    explicit StorageSubItem(std::string_view key) : StorageItem(key) {
    }

    StorageSubItem *getSubItem(wups_storage_item item);

    const StorageSubItem *getSubItem(const char *key) const;

    bool deleteItem(const char *key);

    StorageItem *createItem(const char *key, StorageSubItem::StorageSubItemError &error);

    StorageSubItem *createSubItem(const char *key, StorageSubItem::StorageSubItemError &error);

    StorageItem *getItem(const char *name);

    [[nodiscard]] const std::forward_list<StorageSubItem> &getSubItems() const {
        return mSubCategories;
    }

    [[nodiscard]] const std::map<std::string, StorageItem> &getItems() const {
        return mItems;
    }

protected:
    std::forward_list<StorageSubItem> mSubCategories;
    std::map<std::string, StorageItem> mItems;
};
