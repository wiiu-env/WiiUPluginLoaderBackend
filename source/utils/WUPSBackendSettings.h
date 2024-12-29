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
}; // namespace WUPSBackendSettings
