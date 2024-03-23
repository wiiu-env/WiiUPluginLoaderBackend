#include "WUPSConfigAPI.h"
#include "WUPSConfig.h"
#include "WUPSConfigCategory.h"
#include "WUPSConfigItem.h"
#include "WUPSConfigItemV1.h"
#include "WUPSConfigItemV2.h"
#include "globals.h"
#include "plugin/PluginConfigData.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include <algorithm>
#include <memory>
#include <vector>
#include <wums/exports.h>
#include <wups/config.h>
#include <wups/config_api.h>

namespace WUPSConfigAPIBackend {
    std::vector<std::unique_ptr<WUPSConfig>> sConfigs;
    std::mutex sConfigsMutex;

    std::vector<std::unique_ptr<WUPSConfigCategory>> sConfigCategories;
    std::mutex sConfigCategoryMutex;

    std::vector<std::unique_ptr<WUPSConfigItem>> sConfigItems;
    std::mutex sConfigItemsMutex;

    namespace Intern {
        WUPSConfig *GetConfigByHandle(WUPSConfigHandle handle) {
            std::lock_guard lock(sConfigsMutex);
            auto itr = std::find_if(sConfigs.begin(), sConfigs.end(), [&handle](auto &cur) { return handle == cur.get(); });
            if (itr == sConfigs.end()) {
                return nullptr;
            }
            return itr->get();
        }

        std::unique_ptr<WUPSConfig> PopConfigByHandle(WUPSConfigHandle handle) {
            return pop_locked_first_if(sConfigsMutex, sConfigs, [&handle](auto &cur) { return handle == cur.get(); });
        }

        static WUPSConfigCategory *GetCategoryByHandleRecursive(WUPSConfigCategory *category, WUPSConfigCategoryHandle handle) {
            if (handle == category) {
                return category;
            }
            for (const auto &cat : category->getCategories()) {
                auto res = GetCategoryByHandleRecursive(cat.get(), handle);
                if (res) {
                    return res;
                }
            }
            return nullptr;
        }

        WUPSConfigCategory *GetCategoryByHandle(WUPSConfigCategoryHandle handle, bool checkRecursive) {
            std::lock_guard lock(sConfigCategoryMutex);
            auto itr = std::find_if(sConfigCategories.begin(), sConfigCategories.end(), [&handle](auto &cur) { return handle == cur.get(); });
            if (itr == sConfigCategories.end()) {
                if (checkRecursive) {
                    std::lock_guard config_lock(sConfigsMutex);
                    for (const auto &curConfig : sConfigs) {
                        auto *category = Intern::GetCategoryByHandleRecursive(curConfig.get(), handle);
                        if (category) {
                            return category;
                        }
                    }
                }
                return nullptr;
            }
            return itr->get();
        }


        std::unique_ptr<WUPSConfigCategory> PopCategoryByHandle(WUPSConfigCategoryHandle handle) {
            return pop_locked_first_if(sConfigCategoryMutex, sConfigCategories, [&handle](auto &cur) { return handle == cur.get(); });
        }

        WUPSConfigItem *GetItemByHandle(WUPSConfigItemHandle handle) {
            std::lock_guard lock(sConfigItemsMutex);
            auto itr = std::find_if(sConfigItems.begin(), sConfigItems.end(), [&handle](auto &cur) { return handle == cur.get(); });
            if (itr == sConfigItems.end()) {
                return nullptr;
            }
            return itr->get();
        }

        std::unique_ptr<WUPSConfigItem> PopItemByHandle(WUPSConfigItemHandle handle) {
            return pop_locked_first_if(sConfigItemsMutex, sConfigItems, [&handle](auto &cur) { return handle == cur.get(); });
        }

        WUPSConfigAPIStatus CreateConfig(const char *name, WUPSConfigHandle *out) {
            if (out == nullptr || name == nullptr) {
                DEBUG_FUNCTION_LINE("Invalid param: \"out\" or \"name\" was NULL");
                return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
            }

            auto config = make_unique_nothrow<WUPSConfig>(name);
            if (!config) {
                DEBUG_FUNCTION_LINE_ERR("Failed to allocate WUPSConfig");
                return WUPSCONFIG_API_RESULT_OUT_OF_MEMORY;
            }
            std::lock_guard lock(sConfigsMutex);
            *out = WUPSConfigHandle(config.get());
            sConfigs.push_back(std::move(config));
            return WUPSCONFIG_API_RESULT_SUCCESS;
        }

        void CleanAllHandles() {
            std::lock_guard lock(sConfigsMutex);
            std::lock_guard lock1(sConfigCategoryMutex);
            std::lock_guard lock2(sConfigItemsMutex);
            sConfigs.clear();
            sConfigCategories.clear();
            sConfigItems.clear();
        }
    } // namespace Intern

    /**
     * @brief Initialize the WUPSConfigAPI with extended functionality.
     *
     * This function initializes the WUPSConfigAPI with extended functionality by associating a plugin
     * identified by `pluginIdentifier` with the provided options and callback functions. The `options`
     * parameter specifies the configuration options for the plugin, while `openedCallback` and
     * `closedCallback` specify the callback functions to be invoked when the plugin menu is opened
     * and closed, respectively.
     *
     * @param pluginIdentifier  The identifier of the plugin to be associated with the WUPSConfigAPI.
     * @param options           The configuration options for the plugin.
     * @param openedCallback    The callback function to be invoked when the plugin menu is opened.
     * @param closedCallback    The callback function to be invoked when the plugin menu is closed.
     *
     * @return The status of the initialization process. Possible return values are:
     *         - WUPSCONFIG_API_RESULT_SUCCESS: The initialization was successful.
     *         - WUPSCONFIG_API_RESULT_INVALID_ARGUMENT: The `openedCallback` or `closedCallback` parameter is nullptr.
     *         - WUPSCONFIG_API_RESULT_UNSUPPORTED_VERSION: The specified `options.version` is not supported.
     *         - WUPSCONFIG_API_RESULT_NOT_FOUND: The plugin with the given identifier was not found.
     */
    WUPSConfigAPIStatus InitEx(uint32_t pluginIdentifier, WUPSConfigAPIOptions options, WUPSConfigAPI_MenuOpenedCallback openedCallback, WUPSConfigAPI_MenuClosedCallback closedCallback) {
        if (openedCallback == nullptr || closedCallback == nullptr) {
            return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
        }
        for (auto &cur : gLoadedPlugins) {
            if (cur->getHandle() == pluginIdentifier) {
                if (options.version != 1) {
                    return WUPSCONFIG_API_RESULT_UNSUPPORTED_VERSION;
                }
                auto configDat = PluginConfigData::create(options, openedCallback, closedCallback);
                if (!configDat) {
                    DEBUG_FUNCTION_LINE_WARN("Failed to create config data for %08X", pluginIdentifier);
                    return WUPSCONFIG_API_RESULT_UNSUPPORTED_VERSION;
                }
                cur->setConfigData(configDat.value());
                return WUPSCONFIG_API_RESULT_SUCCESS;
            }
        }
        return WUPSCONFIG_API_RESULT_NOT_FOUND;
    }

    namespace Category {
        WUPSConfigAPIStatus Create(WUPSConfigAPICreateCategoryOptions options, WUPSConfigCategoryHandle *out) {
            if (out == nullptr) {
                DEBUG_FUNCTION_LINE("Invalid param: \"out\" or \"name\" was NULL");
                return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
            }
            if (options.version != WUPS_API_CATEGORY_OPTION_VERSION_V1) {
                DEBUG_FUNCTION_LINE_WARN("Invalid category option version: expected %08X but got %08X", WUPS_API_CATEGORY_OPTION_VERSION_V1, options.version);
                return WUPSCONFIG_API_RESULT_UNSUPPORTED_VERSION;
            }
            auto category = make_unique_nothrow<WUPSConfigCategory>(options.data.v1.name);
            if (!category) {
                DEBUG_FUNCTION_LINE_ERR("Failed to allocate WUPSConfigCategory");
                return WUPSCONFIG_API_RESULT_OUT_OF_MEMORY;
            }

            std::lock_guard lock(sConfigCategoryMutex);
            *out = WUPSConfigCategoryHandle(category.get());
            sConfigCategories.push_back(std::move(category));
            return WUPSCONFIG_API_RESULT_SUCCESS;
        }

        WUPSConfigAPIStatus Destroy(WUPSConfigCategoryHandle handle) {
            if (handle == nullptr) {
                DEBUG_FUNCTION_LINE("Invalid param: \"handle\" was NULL");
                return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
            }

            if (!remove_locked_first_if(sConfigCategoryMutex, sConfigCategories, [&handle](auto &cur) { return handle == cur.get(); })) {
                {
                    // Ignore any attempts to destroy to create root item.
                    std::lock_guard lock(sConfigsMutex);
                    if (std::any_of(sConfigs.begin(), sConfigs.end(), [&handle](auto &cur) { return handle == cur.get(); })) {
                        return WUPSCONFIG_API_RESULT_SUCCESS;
                    }
                }
                DEBUG_FUNCTION_LINE_WARN("Failed to destroy WUPSConfigCategory (for handle: \"%08X\")", handle.handle);
                return WUPSCONFIG_API_RESULT_NOT_FOUND;
            }
            return WUPSCONFIG_API_RESULT_SUCCESS;
        }

        WUPSConfigAPIStatus AddCategory(WUPSConfigCategoryHandle parentHandle, WUPSConfigCategoryHandle categoryHandle) {
            if (parentHandle == nullptr || categoryHandle == nullptr) {
                DEBUG_FUNCTION_LINE("Invalid param: \"parentHandle\" or \"categoryHandle\" was NULL");
                return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
            }
            std::lock_guard lockConfigs(sConfigsMutex);
            std::lock_guard lockCats(sConfigCategoryMutex);
            WUPSConfigCategory *parentCat = Intern::GetConfigByHandle(WUPSConfigHandle(parentHandle.handle));
            if (!parentCat) {
                parentCat = Intern::GetCategoryByHandle(parentHandle);
                if (!parentCat) {
                    DEBUG_FUNCTION_LINE_WARN("Failed to find parent for handle %08X", parentHandle.handle);
                    return WUPSCONFIG_API_RESULT_NOT_FOUND;
                }
            }

            auto category = Intern::PopCategoryByHandle(categoryHandle);
            if (!category) {
                DEBUG_FUNCTION_LINE_WARN("Failed to find category. parentHandle: %08X categoryHandle: %08X", parentHandle.handle, categoryHandle.handle);
                return WUPSCONFIG_API_RESULT_NOT_FOUND;
            }

            if (!parentCat->addCategory(category)) {
                sConfigCategories.push_back(std::move(category));
                DEBUG_FUNCTION_LINE_WARN("Failed to add category to parent. parentHandle: %08X categoryHandle: %08X", parentHandle.handle, categoryHandle.handle);
                return WUPSCONFIG_API_RESULT_UNKNOWN_ERROR; // TODO!!!
            }
            return WUPSCONFIG_API_RESULT_SUCCESS;
        }

        WUPSConfigAPIStatus AddItem(WUPSConfigCategoryHandle parentHandle, WUPSConfigItemHandle itemHandle) {
            if (parentHandle == nullptr || itemHandle == nullptr) {
                DEBUG_FUNCTION_LINE("Invalid param: \"handle\" or \"item_Handle\" was NULL");
                return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
            }

            std::lock_guard lockConfigs(sConfigsMutex);
            std::lock_guard lockCats(sConfigCategoryMutex);
            WUPSConfigCategory *parentCat = Intern::GetConfigByHandle(WUPSConfigHandle(parentHandle.handle));
            if (!parentCat) {
                parentCat = Intern::GetCategoryByHandle(parentHandle, true);
                if (!parentCat) {
                    DEBUG_FUNCTION_LINE_WARN("Failed to find parent for handle %08X", parentHandle.handle);
                    return WUPSCONFIG_API_RESULT_NOT_FOUND;
                }
            }

            auto item = Intern::PopItemByHandle(itemHandle);
            if (!item) {
                DEBUG_FUNCTION_LINE_ERR("Failed to get item for handle %08X", itemHandle.handle);
                return WUPSCONFIG_API_RESULT_NOT_FOUND;
            }

            if (!parentCat->addItem(item)) {
                std::lock_guard lockItems(sConfigItemsMutex);
                sConfigItems.push_back(std::move(item));
                DEBUG_FUNCTION_LINE_ERR("Failed to add item %08X to category %08X", itemHandle.handle, parentHandle.handle);
                return WUPSCONFIG_API_RESULT_UNKNOWN_ERROR; // TODO
            }

            return WUPSCONFIG_API_RESULT_SUCCESS;
        }
    } // namespace Category

    namespace Item {
        WUPSConfigAPIStatus Create(WUPSConfigAPICreateItemOptions options, WUPSConfigItemHandle *out) {
            if (out == nullptr) {
                DEBUG_FUNCTION_LINE("Invalid param: \"out\" was NULL");
                return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
            }

            std::unique_ptr<WUPSConfigItem> item = nullptr;
            if (options.version == WUPS_API_ITEM_OPTION_VERSION_V1) {
                item = make_unique_nothrow<WUPSConfigItemV1>(options.data.v1.configId, options.data.v1.displayName, options.data.v1.callbacks, options.data.v1.context);
            } else if (options.version == WUPS_API_ITEM_OPTION_VERSION_V2) {
                item = make_unique_nothrow<WUPSConfigItemV2>(options.data.v2.displayName, options.data.v2.callbacks, options.data.v2.context);
            } else {
                DEBUG_FUNCTION_LINE_WARN("Invalid item option version: expected %08X or %08X but got %08X", WUPS_API_ITEM_OPTION_VERSION_V1, WUPS_API_ITEM_OPTION_VERSION_V2, options.version);
                return WUPSCONFIG_API_RESULT_UNSUPPORTED_VERSION;
            }

            if (!item) {
                DEBUG_FUNCTION_LINE_ERR("Failed to allocate WUPSConfigItem");
                return WUPSCONFIG_API_RESULT_OUT_OF_MEMORY;
            }
            std::lock_guard lock(sConfigItemsMutex);
            *out = WUPSConfigItemHandle(item.get());
            sConfigItems.push_back(std::move(item));
            return WUPSCONFIG_API_RESULT_SUCCESS;
        }

        WUPSConfigAPIStatus Destroy(WUPSConfigItemHandle handle) {
            if (handle == nullptr) {
                DEBUG_FUNCTION_LINE("Invalid param: \"handle\" was NULL");
                return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
            }

            if (!remove_locked_first_if(sConfigItemsMutex, sConfigItems, [&handle](auto &cur) { return cur.get() == handle; })) {
                DEBUG_FUNCTION_LINE_WARN("Failed to destroy WUPSConfigItem (handle: \"%08X\")", handle);
                return WUPSCONFIG_API_RESULT_NOT_FOUND;
            }
            return WUPSCONFIG_API_RESULT_SUCCESS;
        }
    } // namespace Item

    WUPSConfigAPIStatus WUPSConfigAPI_GetVersion(WUPSConfigAPIVersion *out) {
        if (out == nullptr) {
            return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
        }
        *out = 1;
        return WUPSCONFIG_API_RESULT_SUCCESS;
    }

    WUMS_EXPORT_FUNCTION(WUPSConfigAPI_GetVersion);

    WUMS_EXPORT_FUNCTION_EX(WUPSConfigAPIBackend::InitEx, WUPSConfigAPI_InitEx);
    WUMS_EXPORT_FUNCTION_EX(WUPSConfigAPIBackend::Category::Create, WUPSConfigAPI_Category_CreateEx);
    WUMS_EXPORT_FUNCTION_EX(WUPSConfigAPIBackend::Category::Destroy, WUPSConfigAPI_Category_Destroy);
    WUMS_EXPORT_FUNCTION_EX(WUPSConfigAPIBackend::Category::AddCategory, WUPSConfigAPI_Category_AddCategory);
    WUMS_EXPORT_FUNCTION_EX(WUPSConfigAPIBackend::Category::AddItem, WUPSConfigAPI_Category_AddItem);

    WUMS_EXPORT_FUNCTION_EX(WUPSConfigAPIBackend::Item::Create, WUPSConfigAPI_Item_CreateEx);
    WUMS_EXPORT_FUNCTION_EX(WUPSConfigAPIBackend::Item::Destroy, WUPSConfigAPI_Item_Destroy);
} // namespace WUPSConfigAPIBackend
