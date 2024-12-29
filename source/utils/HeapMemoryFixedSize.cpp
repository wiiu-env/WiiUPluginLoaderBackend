#include "HeapMemoryFixedSize.h"

#include "utils.h"

HeapMemoryFixedSize::HeapMemoryFixedSize() = default;

HeapMemoryFixedSize::HeapMemoryFixedSize(const std::size_t size) : mData(make_unique_nothrow<uint8_t[]>(size)), mSize(mData ? size : 0) {}

HeapMemoryFixedSize::HeapMemoryFixedSize(HeapMemoryFixedSize &&other) noexcept
    : mData(std::move(other.mData)), mSize(other.mSize) {
    other.mSize = 0;
}

HeapMemoryFixedSize &HeapMemoryFixedSize::operator=(HeapMemoryFixedSize &&other) noexcept {
    if (this != &other) {
        mData       = std::move(other.mData);
        mSize       = other.mSize;
        other.mSize = 0;
    }
    return *this;
}

HeapMemoryFixedSize::operator bool() const {
    return mData != nullptr;
}

[[nodiscard]] const void *HeapMemoryFixedSize::data() const {
    return mData.get();
}

[[nodiscard]] std::size_t HeapMemoryFixedSize::size() const {
    return mSize;
}