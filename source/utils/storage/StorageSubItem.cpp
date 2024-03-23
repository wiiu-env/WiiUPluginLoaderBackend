#include "StorageSubItem.h"

StorageSubItem *StorageSubItem::getSubItem(wups_storage_item item) {
    // Try to find the sub-item based on item handle.
    for (auto &cur : mSubCategories) {
        if (cur.getHandle() == (uint32_t) item) {
            return &cur;
        }
    }

    // If not found in current category, recursively search in sub-categories.
    for (auto &cur : mSubCategories) {
        auto res = cur.getSubItem(item);
        if (res) {
            return res;
        }
    }

    return nullptr;
}

const StorageSubItem *StorageSubItem::getSubItem(const char *key) const {
    // Try to find the sub-item based on key.
    for (const auto &cur : mSubCategories) {
        if (cur.getKey() == key) {
            return &cur;
        }
    }

    return nullptr;
}

bool StorageSubItem::deleteItem(const char *key) {
    if (remove_first_if(mSubCategories, [key](auto &cur) { return cur.getKey() == key; })) {
        return true;
    }

    auto itemItr = mItems.find(key);
    if (itemItr != mItems.end()) {
        mItems.erase(itemItr);
        return true; // Item found and deleted.
    }
    return false;
}

StorageItem *StorageSubItem::createItem(const char *key, StorageSubItem::StorageSubItemError &error) {
    for (const auto &cur : mSubCategories) {
        if (cur.getKey() == key) {
            error = STORAGE_SUB_ITEM_KEY_ALREADY_IN_USE;
            return nullptr;
        }
    }

    auto result = mItems.insert({key, StorageItem(key)});
    if (result.second) {
        return &result.first->second;
    }
    return nullptr;
}

StorageSubItem *StorageSubItem::createSubItem(const char *key, StorageSubItem::StorageSubItemError &error) {
    auto resItr = mItems.find(key);
    if (resItr != mItems.end()) {
        error = STORAGE_SUB_ITEM_KEY_ALREADY_IN_USE;
        return nullptr;
    }
    for (const auto &cur : mSubCategories) {
        if (cur.getKey() == key) {
            error = STORAGE_SUB_ITEM_KEY_ALREADY_IN_USE;
            return nullptr;
        }
    }

    mSubCategories.emplace_front(key);
    return &mSubCategories.front();
}

StorageItem *StorageSubItem::getItem(const char *name) {
    auto resItr = mItems.find(name);
    if (resItr != mItems.end()) {
        return &resItr->second;
    }
    return nullptr;
}
