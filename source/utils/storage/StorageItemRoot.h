#pragma once

#include "StorageItem.h"
#include "StorageSubItem.h"
#include "utils/logger.h"
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <wups/storage.h>

class StorageItemRoot : public StorageSubItem {
public:
    explicit StorageItemRoot(const std::string &plugin_name) : StorageSubItem(plugin_name), mPluginName(plugin_name) {
    }

    [[nodiscard]] const std::string &getPluginId() const {
        return mPluginName;
    }

private:
    std::string mPluginName;
};
