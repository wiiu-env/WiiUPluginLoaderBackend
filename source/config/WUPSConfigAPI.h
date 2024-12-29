#pragma once

#include <wups/config.h>

#include <memory>

namespace WUPSConfigAPIBackend {
    class WUPSConfigItem;
    class WUPSConfig;
    class WUPSConfigCategory;
    namespace Intern {
        /**
         * @brief Retrieves a WUPSConfig pointer based on a given handle.
         *
         * This function searches for a WUPSConfig within a list of WUPSConfig objects based on a handle.
         * If a matching handle is found, a pointer to the WUPSConfig object is returned. Otherwise, nullptr is returned.
         *
         * @param handle The handle used to identify the desired WUPSConfig object.
         * @return A pointer to the WUPSConfig object with the matching handle, or nullptr if not found.
         *
         * @note This function locks the sConfigsMutex during the search operation to ensure thread safety.
         */
        WUPSConfig *GetConfigByHandle(WUPSConfigHandle handle);

        /**
         * @brief Pop a WUPSConfig object from the sConfigs vector based on the provided handle.
         *
         * This function retrieves a WUPSConfig object from the sConfigs vector based on the given handle.
         * It locks the sConfigsMutex to ensure thread safety during the operation.
         *
         * @param handle The handle of the WUPSConfig object to be retrieved.
         * @return A std::unique_ptr to the WUPSConfig object if found, nullptr otherwise.
         */
        std::unique_ptr<WUPSConfig> PopConfigByHandle(WUPSConfigHandle handle);

        /**
         * @brief Get a WUPSConfigCategory object by its handle
         *
         * This function searches for a WUPSConfigCategory object with the given handle.
         * If the 'checkRecursive' flag is set to true, the function also searches recursively
         * through all the WUPSConfig objects for the requested category.
         *
         * @param handle The handle of the desired WUPSConfigCategory
         * @param checkRecursive Flag to indicate whether recursive search is required
         * @return A pointer to the found WUPSConfigCategory object, or nullptr if not found
         */
        WUPSConfigCategory *GetCategoryByHandle(WUPSConfigCategoryHandle handle, bool checkRecursive = false);

        /**
         * @brief Pop a WUPSConfigCategory from the list of categories by its handle.
         *
         * This function searches for a WUPSConfigCategory in the list of categories using the given handle.
         * If a matching WUPSConfigCategory is found, it is removed from the list and returned as a unique_ptr.
         * If no matching category is found, nullptr is returned.
         *
         * @param handle The handle of the WUPSConfigCategory to pop.
         * @return std::unique_ptr<WUPSConfigCategory> The popped WUPSConfigCategory or nullptr if not found.
         */
        std::unique_ptr<WUPSConfigCategory> PopCategoryByHandle(WUPSConfigCategoryHandle handle);

        /**
         * @brief Retrieves a WUPSConfigItem object by its handle.
         *
         * This function searches for a WUPSConfigItem object in the sConfigItems vector
         * with a matching handle. It acquires a lock on the sConfigItemsMutex
         * to ensure thread safety during the search operation. If a matching object is found,
         * a pointer to the object is returned. Otherwise, nullptr is returned.
         *
         * @param handle The handle of the WUPSConfigItem to retrieve.
         * @return A pointer to the WUPSConfigItem object with the specified handle, or nullptr if not found.
         */
        WUPSConfigItem *GetItemByHandle(WUPSConfigItemHandle handle);

        /**
         * @brief Removes and returns an item from the configuration items list based on its handle.
         *
         * This function pops and returns an item from the `sConfigItems` vector based on its handle.
         * The handle is used to match the item in the vector using a predicate. If a matching item is found,
         * it is moved to the returned unique_ptr and removed from the vector.
         *
         * @param handle The handle of the item to be popped.
         * @return A unique_ptr to the popped item. If no matching item is found, returns a nullptr.
         *
         * @note The function locks the `sConfigItemsMutex` mutex to ensure thread safety while performing the operation.
         *
         * @see WUPSConfigItem
         * @see pop_locked_first_if
         * @see sConfigItemsMutex
         * @see sConfigItems
         */
        std::unique_ptr<WUPSConfigItem> PopItemByHandle(WUPSConfigItemHandle handle);

        /**
         * @brief Creates a new configuration with the given name.
         *
         * This function creates a new configuration with the specified name. The configuration is allocated dynamically
         * using `make_unique_nothrow` and added to the `sConfigs` vector.
         *
         * @param name The name of the configuration.
         * @param out A pointer to a `WUPSConfigHandle` where the created configuration will be stored.
         *
         * @return A `WUPSConfigAPIStatus` indicating the status of the operation.
         *         - `WUPSCONFIG_API_RESULT_SUCCESS` if the configuration was created successfully.
         *         - `WUPSCONFIG_API_RESULT_INVALID_ARGUMENT` if the `out` or `name` parameter is null.
         *         - `WUPSCONFIG_API_RESULT_OUT_OF_MEMORY` if memory allocation fails.
         *
         * @note The caller is responsible for managing the lifetime of the created configuration and freeing the
         *       associated resources when they are no longer needed.
         */
        WUPSConfigAPIStatus CreateConfig(const char *name, WUPSConfigHandle *out);

        /**
         * @brief Cleans all handles and clears the configuration data.
         *
         * This function acquires locks on three different mutexes: `sConfigsMutex`, `sConfigCategoryMutex`, and `sConfigItemsMutex`.
         * It then clears the vectors `sConfigs`, `sConfigCategories`, and `sConfigItems`, effectively cleaning all handles and
         * removing all configuration data.
         *
         * @note This function assumes that the mutexes and vectors are already defined and initialized.
         *
         * @see sConfigsMutex
         * @see sConfigCategoryMutex
         * @see sConfigItemsMutex
         * @see sConfigs
         * @see sConfigCategories
         * @see sConfigItems
         */
        void CleanAllHandles();
    } // namespace Intern

    namespace Category {
        /**
         * @brief Create a new WUPSConfigCategory.
         *
         * This function creates a new WUPSConfigCategory with the given options.
         * The created category will be added to the global config categories list and a handle to the category will be
         * returned.
         *
         * @param options The options to create the category.
         * @param out[out] Pointer to store the handle to the newly created category.
         * @return WUPSConfigAPIStatus The status of the operation.
         *         - WUPSCONFIG_API_RESULT_SUCCESS: Success.
         *         - WUPSCONFIG_API_RESULT_INVALID_ARGUMENT: Invalid parameter, `out` or `name` is NULL.
         *         - WUPSCONFIG_API_RESULT_UNSUPPORTED_VERSION: Invalid category option version.
         *         - WUPSCONFIG_API_RESULT_OUT_OF_MEMORY: Failed to allocate WUPSConfigCategory.
         *
         * @note The caller is responsible for deleting the WUPSConfigCategory instance unless it has been transferred to
         * another category
         */
        WUPSConfigAPIStatus Create(WUPSConfigAPICreateCategoryOptions options, WUPSConfigCategoryHandle *out);

        /**
         * @brief Destroy a WUPSConfigCategory.
         *
         * This function destroys a WUPSConfigCategory identified by the given handle.
         *
         * @param handle The handle of the WUPSConfigCategory to destroy.
         * @return WUPSConfigAPIStatus The status of the destroy operation.
         *     - WUPSCONFIG_API_RESULT_SUCCESS: If the WUPSConfigCategory was successfully destroyed.
         *     - WUPSCONFIG_API_RESULT_INVALID_ARGUMENT: If the handle was NULL.
         *     - WUPSCONFIG_API_RESULT_NOT_FOUND: If the WUPSConfigCategory was not found.
         */
        WUPSConfigAPIStatus Destroy(WUPSConfigCategoryHandle handle);

        /**
         * @brief Adds a category to the WUPS configuration.
         *
         * This function adds a category to the WUPS configuration. The category is added as a child of
         * the specified parent category. On success the ownership will be passed to the parent.
         *
         * @param parentHandle The handle of the parent category.
         * @param categoryHandle The handle of the category to be added.
         * @return The status of the operation. Possible values are:
         *         - WUPSCONFIG_API_RESULT_SUCCESS: The category was added successfully.
         *         - WUPSCONFIG_API_RESULT_INVALID_ARGUMENT: One or both of the input handles were NULL.
         *         - WUPSCONFIG_API_RESULT_NOT_FOUND: The parent category or the specified category was not found.
         *         - WUPSCONFIG_API_RESULT_UNKNOWN_ERROR: Failed to add the category to the parent.
         */
        WUPSConfigAPIStatus AddCategory(WUPSConfigCategoryHandle parentHandle, WUPSConfigCategoryHandle categoryHandle);

        /**
         * @brief Adds an item to a WUPSConfigCategory.
         *
         * This function adds a WUPSConfigItem to a WUPSConfigCategory. The item will be added to the end of the list
         * of items in the category. This function also holds the responsibility for deleting the created item instance.
         *
         * @param parentHandle The handle of the parent category.
         * @param itemHandle The handle of the item to be added.
         * @return Returns the status of the operation.
         *         - WUPSCONFIG_API_RESULT_SUCCESS: If the item was added successfully.
         *         - WUPSCONFIG_API_RESULT_INVALID_ARGUMENT: If either the parentHandle or itemHandle is nullptr.
         *         - WUPSCONFIG_API_RESULT_NOT_FOUND: If the parent category was not found.
         *         - WUPSCONFIG_API_RESULT_UNKNOWN_ERROR: If an unknown error occurred while adding the item.
         */
        WUPSConfigAPIStatus AddItem(WUPSConfigCategoryHandle parentHandle, WUPSConfigItemHandle itemHandle);
    } // namespace Category

    namespace Item {
        /**
         * @brief Creates a new WUPSConfigItem and adds it to the list of config items.
         *
         * This function creates a new WUPSConfigItem object based on the provided options,
         * and returns a handle to the newly created item.
         *
         * @param options The options for creating the item.
         * @param out Pointer to store the handle to the newly created item.
         * @return The status of the API call.
         *         - WUPSCONFIG_API_RESULT_SUCCESS if the item was created and added successfully.
         *         - WUPSCONFIG_API_RESULT_INVALID_ARGUMENT if the 'out' parameter is nullptr.
         *         - WUPSCONFIG_API_RESULT_UNSUPPORTED_VERSION if the options version is invalid.
         *         - WUPSCONFIG_API_RESULT_OUT_OF_MEMORY if memory allocation failed.
         * @note The caller is responsible for deleting the WUPSConfigItem instance unless it has been transferred to
         * a category
         */
        WUPSConfigAPIStatus Create(WUPSConfigAPICreateItemOptions options, WUPSConfigItemHandle *out);

        /**
         * @brief Destroy a WUPSConfigItem.
         *
         * This function destroys a WUPSConfigItem identified by the given handle. It removes the memory allocated for the item.
         *
         * @param handle The handle of the WUPSConfigItem to destroy.
         * @return The status of the operation, which can be one of the following values:
         *      - WUPSCONFIG_API_RESULT_SUCCESS: The WUPSConfigItem was destroyed successfully.
         *      - WUPSCONFIG_API_RESULT_NOT_FOUND: The WUPSConfigItem with the given handle was not found.
         *      - WUPSCONFIG_API_RESULT_INVALID_ARGUMENT: The handle parameter was NULL.
         *
         * @see WUPSConfigItemHandle, WUPSConfigAPIStatus
         */
        WUPSConfigAPIStatus Destroy(WUPSConfigItemHandle handle);
    } // namespace Item

} // namespace WUPSConfigAPIBackend