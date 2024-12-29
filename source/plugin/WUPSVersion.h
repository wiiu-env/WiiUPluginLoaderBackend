#pragma once

#include <optional>
#include <string>

#include <cstdint>

class WUPSVersion {
public:
    WUPSVersion(int major, int minor, int revision);

    static std::optional<WUPSVersion> createFromString(const std::string &versionStr);

    std::strong_ordering operator<=>(const WUPSVersion &other) const;

    [[nodiscard]] std::string toString() const;

private:
    uint32_t mMajor;
    uint32_t mMinor;
    uint32_t mRevision;
};
