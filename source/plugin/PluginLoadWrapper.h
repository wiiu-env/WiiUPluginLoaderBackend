#pragma once

#include "PluginMetaInformation.h"


#include <memory>

class PluginData;

class PluginLoadWrapper {
public:
    PluginLoadWrapper(std::shared_ptr<PluginData> pluginData, const bool linkAndLoad, const bool heapTrackingEnabled = false)
        : mPluginData(std::move(pluginData)), mIsLoadAndLink(linkAndLoad), mIsHeapTrackingEnabled(heapTrackingEnabled) {
    }

    [[nodiscard]] const std::shared_ptr<PluginData> &getPluginData() const {
        return mPluginData;
    }

    [[nodiscard]] bool isLoadAndLink() const {
        return mIsLoadAndLink;
    }

    [[nodiscard]] bool isHeapTrackingEnabled() const {
        return mIsHeapTrackingEnabled;
    }

    [[nodiscard]] std::optional<PluginMetaInformation::HeapTrackingOptions> getHeapTrackingOptions() const {
        if (mIsHeapTrackingEnabled) {
            return PluginMetaInformation::HeapTrackingOptions::TRACK_HEAP_OPTIONS_TRACK_SIZE_AND_COLLECT_STACK_TRACES;
        }
        return std::nullopt;
    }

private:
    std::shared_ptr<PluginData> mPluginData;
    bool mIsLoadAndLink         = false;
    bool mIsHeapTrackingEnabled = false;
};