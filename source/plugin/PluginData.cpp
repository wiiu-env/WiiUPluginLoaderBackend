#include "PluginData.h"

uint32_t PluginData::getHandle() const {
    return (uint32_t) this;
}

std::span<const uint8_t> PluginData::getBuffer() const {
    return mBuffer;
}

const std::string &PluginData::getSource() const {
    return mSource;
}
