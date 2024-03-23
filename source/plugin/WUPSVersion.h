#include "utils/StringTools.h"
#include <cstdint>
#include <optional>

class WUPSVersion {
public:
    WUPSVersion(int major, int minor, int revision)
        : mVersion((static_cast<uint64_t>(major) << 32) |
                   (static_cast<uint64_t>(minor) << 16) |
                   static_cast<uint64_t>(revision)) {}

    WUPSVersion(const WUPSVersion &other) = default;

    static std::optional<WUPSVersion> createFromString(const std::string &versionStr) {
        char *end;
        errno = 0; // Initialize errno before calling strtol

        auto major = strtol(versionStr.c_str(), &end, 10);
        if (errno || *end != '.') {
            return std::nullopt;
        }


        auto minor = strtol(end + 1, &end, 10);
        if (errno || *end != '.') {
            return std::nullopt;
        }

        auto revision = strtol(end + 1, &end, 10);
        if (errno || *end != '\0') {
            return std::nullopt;
        }

        return WUPSVersion(static_cast<int>(major), static_cast<int>(minor), static_cast<int>(revision));
    }

    std::strong_ordering operator<=>(const WUPSVersion &other) const {
        return mVersion <=> other.mVersion;
    }

    [[nodiscard]] std::string toString() const {
        return string_format("%d.%d.%d", static_cast<int>((mVersion >> 32) & 0xFFFF),
                             static_cast<int>((mVersion >> 16) & 0xFFFF),
                             static_cast<int>((mVersion) &0xFFFF));
    }

private:
    uint64_t mVersion{};
};
