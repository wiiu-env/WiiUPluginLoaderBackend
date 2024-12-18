#include "PluginData.h"

PluginData::PluginData(std::vector<uint8_t> &&buffer, const std::string_view source) : mBuffer(std::move(buffer)), mSource(source) {
    // Abuse this as a stable handle that references itself and survives std::move
    *mHandle = reinterpret_cast<uint32_t>(mHandle.get());
}

PluginData::PluginData(std::span<uint8_t> buffer, const std::string_view source) : PluginData(std::vector(buffer.begin(), buffer.end()), source) {
}

PluginData::PluginData(PluginData &&src) : mBuffer(std::move(src.mBuffer)),
                                           mSource(std::move(src.mSource)),
                                           mHandle(std::move(src.mHandle)) {
}

PluginData &PluginData::operator=(PluginData &&src) noexcept {
    if (this != &src) {
        this->mBuffer = std::move(src.mBuffer);
        this->mSource = std::move(src.mSource);
        this->mHandle = std::move(src.mHandle);
    }
    return *this;
}

uint32_t PluginData::getHandle() const {
    return *mHandle;
}

std::span<const uint8_t> PluginData::getBuffer() const {
    return mBuffer;
}

const std::string &PluginData::getSource() const {
    return mSource;
}
