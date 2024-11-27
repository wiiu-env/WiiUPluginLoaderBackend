#include "PluginData.h"

PluginData::PluginData(std::vector<uint8_t> &&buffer, const std::string_view source) : mBuffer(std::move(buffer)), mSource(source) {
}

PluginData::PluginData(std::span<uint8_t> buffer, const std::string_view source) : mBuffer(buffer.begin(), buffer.end()), mSource(source) {
}


PluginData::PluginData(PluginData &&src) : mBuffer(std::move(src.mBuffer)),
                                           mSource(std::move(src.mSource)) {
}

PluginData &PluginData::operator=(PluginData &&src) noexcept {
    if (this != &src) {
        this->mBuffer = std::move(src.mBuffer);
        this->mSource = std::move(src.mSource);
    }
    return *this;
}

uint32_t PluginData::getHandle() const {
    return reinterpret_cast<uint32_t>(this);
}

std::span<const uint8_t> PluginData::getBuffer() const {
    return mBuffer;
}

const std::string &PluginData::getSource() const {
    return mSource;
}
