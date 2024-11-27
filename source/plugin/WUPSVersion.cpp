#include "WUPSVersion.h"
#include "utils/StringTools.h"

WUPSVersion::WUPSVersion(const int major, const int minor, const int revision)
    : mMajor(major), mMinor(minor), mRevision(revision) {
}

std::optional<WUPSVersion> WUPSVersion::createFromString(const std::string &versionStr) {
    char *end;
    errno = 0; // Initialize errno before calling strtol

    const auto major = strtol(versionStr.c_str(), &end, 10);
    if (errno || *end != '.') {
        return std::nullopt;
    }


    const auto minor = strtol(end + 1, &end, 10);
    if (errno || *end != '.') {
        return std::nullopt;
    }

    const auto revision = strtol(end + 1, &end, 10);
    if (errno || *end != '\0') {
        return std::nullopt;
    }

    return WUPSVersion(static_cast<int>(major), static_cast<int>(minor), static_cast<int>(revision));
}

std::strong_ordering WUPSVersion::operator<=>(const WUPSVersion &other) const {
    if (const auto cmp = mMajor <=> other.mMajor; cmp != std::strong_ordering::equal) return cmp;
    if (const auto cmp = mMinor <=> other.mMinor; cmp != std::strong_ordering::equal) return cmp;
    return mRevision <=> other.mRevision;
}

[[nodiscard]] std::string WUPSVersion::toString() const {
    return string_format("%d.%d.%d", mMajor,
                         mMinor,
                         mRevision);
}