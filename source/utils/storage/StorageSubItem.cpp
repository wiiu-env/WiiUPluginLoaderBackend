#include "StorageSubItem.h"

#include <utils/utils.h>

StorageSubItem::StorageSubItem(const std::string_view key) : StorageItem(key) {
}

StorageSubItem *StorageSubItem::getSubItem(wups_storage_item item) {
    // Try to find the sub-item based on item handle.
    for (auto &cur : mSubCategories) {
        if (cur.getHandle() == reinterpret_cast<uint32_t>(item)) {
            return &cur;
        }
    }

    // If not found in current category, recursively search in sub-categories.
    for (auto &cur : mSubCategories) {
        if (const auto res = cur.getSubItem(item)) {
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

    if (const auto itemItr = mItems.find(key); itemItr != mItems.end()) {
        mItems.erase(itemItr);
        return true; // Item found and deleted.
    }
    return false;
}

StorageItem *StorageSubItem::createItem(const char *key, StorageSubItemError &error) {
    if (getSubItem(key) != nullptr) {
        error = STORAGE_SUB_ITEM_KEY_ALREADY_IN_USE;
        return nullptr;
    }

    if (const auto [addedItem, itemAdded] = mItems.insert({key, StorageItem(key)}); itemAdded) {
        return &addedItem->second;
    }
    error = STORAGE_SUB_ITEM_KEY_ALREADY_IN_USE;
    return nullptr;
}

StorageSubItem *StorageSubItem::createSubItem(const char *key, StorageSubItemError &error) {
    if (getItem(key) != nullptr || getSubItem(key) != nullptr) {
        error = STORAGE_SUB_ITEM_KEY_ALREADY_IN_USE;
        return nullptr;
    }

    mSubCategories.emplace_front(key);
    return &mSubCategories.front();
}

StorageItem *StorageSubItem::getItem(const char *name) {
    if (const auto resItr = mItems.find(name); resItr != mItems.end()) {
        return &resItr->second;
    }
    return nullptr;
}

const std::forward_list<StorageSubItem> &StorageSubItem::getSubItems() const {
    return mSubCategories;
}

const std::map<std::string, StorageItem> &StorageSubItem::getItems() const {
    return mItems;
}