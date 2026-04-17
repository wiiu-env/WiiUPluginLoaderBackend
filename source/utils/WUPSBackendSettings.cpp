#include "WUPSBackendSettings.h"

#include "fs/CFile.hpp"
#include "fs/FSUtils.h"
#include "utils/logger.h"
#include "utils/utils.h"

#include <set>
#include <string>

namespace WUPSBackendSettings {
    namespace {
        std::set<std::string> sInactivePlugins;
        std::set<std::string> sBrokenReentPlugins;
        bool isDirty = false;
    } // namespace

#define INACTIVE_PLUGINS_KEY     "inactive_plugins"
#define BROKEN_REENT_PLUGINS_KEY "possibly_broken_reent_plugins"

    bool IsDirty() {
        return isDirty;
    }

    bool LoadSettings() {
        nlohmann::json j       = nlohmann::json::object();
        std::string folderPath = getModulePath() + "/configs/";
        std::string filePath   = folderPath + "wupsbackend.json";

        if (ParseJsonFromFile(filePath, j) != UTILS_IO_ERROR_SUCCESS) {
            return false;
        }

        sInactivePlugins.clear();
        if (j.contains(INACTIVE_PLUGINS_KEY) && j[INACTIVE_PLUGINS_KEY].is_array()) {
            for (auto &cur : j[INACTIVE_PLUGINS_KEY]) {
                if (cur.is_string()) {
                    sInactivePlugins.insert(cur.get<std::string>());
                }
            }
        }

        sBrokenReentPlugins.clear();
        if (j.contains(BROKEN_REENT_PLUGINS_KEY) && j[BROKEN_REENT_PLUGINS_KEY].is_array()) {
            for (auto &cur : j[BROKEN_REENT_PLUGINS_KEY]) {
                if (cur.is_string()) {
                    sBrokenReentPlugins.insert(cur.get<std::string>());
                }
            }
        }

        isDirty = false;
        return true;
    }

    bool SaveSettings() {
        if (!isDirty) {
            return true;
        }

        std::string folderPath = getModulePath() + "/configs/";
        std::string filePath   = folderPath + "wupsbackend.json";
        if (!FSUtils::CreateSubfolder(folderPath)) {
            return false;
        }

        CFile file(filePath, CFile::WriteOnly);
        if (!file.isOpen()) {
            DEBUG_FUNCTION_LINE_ERR("Cannot create file %s", filePath.c_str());
            return false;
        }

        nlohmann::json j            = nlohmann::json::object();
        j[INACTIVE_PLUGINS_KEY]     = sInactivePlugins;
        j[BROKEN_REENT_PLUGINS_KEY] = sBrokenReentPlugins;

        std::string jsonString = j.dump(4, ' ', false, nlohmann::json::error_handler_t::ignore);
        auto writeResult       = file.write((const uint8_t *) jsonString.c_str(), jsonString.size());

        file.close();

        if (writeResult != (int32_t) jsonString.size()) {
            return false;
        }

        isDirty = false;
        return true;
    }

    void ClearInactivePluginFilenames() {
        if (!sInactivePlugins.empty()) {
            sInactivePlugins.clear();
            isDirty = true;
        }
    }

    void AddInactivePluginFilename(const std::string &filename) {
        if (sInactivePlugins.insert(filename).second) {
            isDirty = true;
        }
    }

    void AddBrokenReentPluginFilename(const std::string &filename) {
        if (sBrokenReentPlugins.insert(filename).second) {
            isDirty = true;
        }
    }

    void RemoveBrokenReentPluginFilename(const std::string &filename) {
        if (sBrokenReentPlugins.erase(filename) > 0) {
            isDirty = true;
        }
    }

    const std::set<std::string> &GetInactivePluginFilenames() {
        return sInactivePlugins;
    }

    const std::set<std::string> &GetBrokenReentPluginFilenames() {
        return sBrokenReentPlugins;
    }

} // namespace WUPSBackendSettings