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
        std::vector<std::string> sInactivePlugins;
    }

#define INACTIVE_PLUGINS_KEY "inactive_plugins"

    bool LoadSettings() {
        nlohmann::json j       = nlohmann::json::object();
        std::string folderPath = getModulePath() + "/configs/";
        std::string filePath   = folderPath + "wupsbackend.json";

        if (!ParseJsonFromFile(filePath, j)) {
            sInactivePlugins.clear();
            return false;
        }

        if (j.contains(INACTIVE_PLUGINS_KEY) && j[INACTIVE_PLUGINS_KEY].is_array()) {
            for (auto &cur : j[INACTIVE_PLUGINS_KEY]) {
                if (cur.is_string()) {
                    sInactivePlugins.push_back(cur);
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

    void SetInactivePluginFilenames(std::span<std::string> filenames) {
        sInactivePlugins.clear();
        for (const auto &filename : filenames) {
            sInactivePlugins.emplace_back(filename);
        }
    }

    const std::vector<std::string> &GetInactivePluginFilenames() {
        return sInactivePlugins;
    }

} // namespace WUPSBackendSettings