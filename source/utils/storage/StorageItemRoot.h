#pragma once

#include "StorageSubItem.h"

#include <string>

class StorageItemRoot : public StorageSubItem {
public:
    explicit StorageItemRoot(std::string_view plugin_name);

    [[nodiscard]] const std::string &getPluginId() const;

    void wipe();

    void migrate(StorageItemRoot &&other);

private:
    std::string mPluginName;
};
