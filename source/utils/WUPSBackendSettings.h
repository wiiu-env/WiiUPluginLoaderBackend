#pragma once

#include <span>
#include <string>
#include <vector>

namespace WUPSBackendSettings {
    bool LoadSettings();

    bool SaveSettings();

    void SetInactivePluginFilenames(std::span<std::string> filenames);

    const std::vector<std::string> &GetInactivePluginFilenames();
}; // namespace WUPSBackendSettings
