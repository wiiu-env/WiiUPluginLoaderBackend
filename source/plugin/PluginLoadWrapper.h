#pragma once

#include <memory>

class PluginData;

class PluginLoadWrapper {
public:
    PluginLoadWrapper(std::shared_ptr<PluginData> pluginData, const bool linkAndLoad) : mPluginData(std::move(pluginData)), mIsLoadAndLink(linkAndLoad) {
    }

    [[nodiscard]] const std::shared_ptr<PluginData> &getPluginData() const {
        return mPluginData;
    }

    [[nodiscard]] bool isLoadAndLink() const {
        return mIsLoadAndLink;
    }

private:
    std::shared_ptr<PluginData> mPluginData;
    bool mIsLoadAndLink = false;
};