#include "FunctionSymbolData.h"

FunctionSymbolData::FunctionSymbolData(const FunctionSymbolData &o2) = default;

FunctionSymbolData::FunctionSymbolData(std::string_view name, void *address, uint32_t size) : mName(name),
                                                                                              mAddress(address),
                                                                                              mSize(size) {
}

bool FunctionSymbolData::operator<(const FunctionSymbolData &rhs) const {
    return reinterpret_cast<uint32_t>(mAddress) < reinterpret_cast<uint32_t>(rhs.mAddress);
}

FunctionSymbolData::~FunctionSymbolData() = default;

[[nodiscard]] const std::string &FunctionSymbolData::getName() const {
    return mName;
}

[[nodiscard]] void *FunctionSymbolData::getAddress() const {
    return mAddress;
}

[[nodiscard]] uint32_t FunctionSymbolData::getSize() const {
    return mSize;
}