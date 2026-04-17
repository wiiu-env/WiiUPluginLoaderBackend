#pragma once

#include <set>
#include <string>

namespace WUPSBackendSettings {
    bool LoadSettings();

    bool SaveSettings();

    void ClearInactivePluginFilenames();

    void AddInactivePluginFilename(const std::string &filename);

    template<typename Iterable>
    void SetInactivePluginFilenames(const Iterable &filenames) {
        ClearInactivePluginFilenames();
        for (const auto &cur : filenames) {
            AddInactivePluginFilename(cur);
        }
    }

    const std::set<std::string> &GetInactivePluginFilenames();

    const std::set<std::string> &GetBrokenReentPluginFilenames();
    void AddBrokenReentPluginFilename(const std::string &filename);
    void RemoveBrokenReentPluginFilename(const std::string &filename);
}; // namespace WUPSBackendSettings
