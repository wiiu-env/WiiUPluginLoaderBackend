#include "RelocationData.h"

RelocationData::RelocationData(const char type,
                               const size_t offset,
                               const int32_t addend,
                               void *destination,
                               std::string name,
                               std::shared_ptr<ImportRPLInformation> rplInfo) : mType(type),
                                                                                mOffset(offset),
                                                                                mAddend(addend),
                                                                                mDestination(destination),
                                                                                mName(std::move(name)),
                                                                                mRPLInfo(std::move(rplInfo)) {
}

RelocationData::RelocationData(const RelocationData &o2) = default;

RelocationData::~RelocationData() = default;

[[nodiscard]] char RelocationData::getType() const {
    return mType;
}

[[nodiscard]] size_t RelocationData::getOffset() const {
    return mOffset;
}

[[nodiscard]] int32_t RelocationData::getAddend() const {
    return mAddend;
}

[[nodiscard]] const void *RelocationData::getDestination() const {
    return mDestination;
}

[[nodiscard]] const std::string &RelocationData::getName() const {
    return mName;
}

[[nodiscard]] const ImportRPLInformation &RelocationData::getImportRPLInformation() const {
    return *mRPLInfo;
}