#include "StorageItemRoot.h"

StorageItemRoot::StorageItemRoot(const std::string_view plugin_name) : StorageSubItem(plugin_name), mPluginName(plugin_name) {
}

[[nodiscard]] const std::string &StorageItemRoot::getPluginId() const {
    return mPluginName;
}

void StorageItemRoot::wipe() {
    mSubCategories.clear();
    mItems.clear();
}