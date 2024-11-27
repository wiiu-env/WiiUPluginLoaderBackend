#include "WUPSConfigCategory.h"


namespace WUPSConfigAPIBackend {

    WUPSConfigCategory::WUPSConfigCategory(const std::string_view name) : mName(name) {
    }

    WUPSConfigCategory::~WUPSConfigCategory() = default;

    const std::string &WUPSConfigCategory::getName() const {
        return mName;
    }

    bool WUPSConfigCategory::addItem(std::unique_ptr<WUPSConfigItem> &item) {
        if (item != nullptr) {
            mItems.push_back(std::move(item));
            return true;
        }
        return false;
    }

    const std::vector<std::unique_ptr<WUPSConfigItem>> &WUPSConfigCategory::getItems() const {
        return mItems;
    }

    const std::vector<std::unique_ptr<WUPSConfigCategory>> &WUPSConfigCategory::getCategories() const {
        return mCategories;
    }

    bool WUPSConfigCategory::addCategory(std::unique_ptr<WUPSConfigCategory> &category) {
        mCategories.push_back(std::move(category));
        return true;
    }

} // namespace WUPSConfigAPIBackend