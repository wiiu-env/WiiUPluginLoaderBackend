#include "SectionInfo.h"

SectionInfo::SectionInfo(std::string name,
                         const uint32_t address,
                         const uint32_t sectionSize) : mName(std::move(name)),
                                                       mAddress(address),
                                                       mSectionSize(sectionSize) {
}

[[nodiscard]] const std::string &SectionInfo::getName() const {
    return mName;
}

[[nodiscard]] uint32_t SectionInfo::getAddress() const {
    return mAddress;
}

[[nodiscard]] uint32_t SectionInfo::getSize() const {
    return mSectionSize;
}

[[nodiscard]] uint32_t SectionInfo::isInSection(uint32_t addr) const {
    return addr >= mAddress && addr < mAddress + mSectionSize;
}