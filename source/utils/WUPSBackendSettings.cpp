#include "WUPSBackendSettings.h"
#include "fs/CFile.hpp"
#include "fs/FSUtils.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include <span>
#include <string>
#include <vector>

namespace WUPSBackendSettings {
    namespace {
        std::set<std::string> sInactivePlugins;
    }

#define INACTIVE_PLUGINS_KEY "inactive_plugins"

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
                    sInactivePlugins.insert(cur);
                }
            }
        }

        return true;
    }

    bool SaveSettings() {
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

        nlohmann::json j        = nlohmann::json::object();
        j[INACTIVE_PLUGINS_KEY] = sInactivePlugins;

        std::string jsonString = j.dump(4, ' ', false, nlohmann::json::error_handler_t::ignore);
        auto writeResult       = file.write((const uint8_t *) jsonString.c_str(), jsonString.size());

        file.close();

        if (writeResult != (int32_t) jsonString.size()) {
            return false;
        }

        return true;
    }

    void ClearInactivePluginFilenames() {
        sInactivePlugins.clear();
    }

    void AddInactivePluginFilename(const std::string &filename) {
        sInactivePlugins.insert(filename);
    }

    const std::set<std::string> &GetInactivePluginFilenames() {
        return sInactivePlugins;
    }

} // namespace WUPSBackendSettings