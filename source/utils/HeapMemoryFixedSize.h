#pragma once
#include "utils.h"
#include <cstdint>
#include <memory>

class HeapMemoryFixedSize {
public:
    HeapMemoryFixedSize() = default;

    explicit HeapMemoryFixedSize(std::size_t size) : mData(make_unique_nothrow<uint8_t[]>(size)), mSize(mData ? size : 0) {}

    // Delete the copy constructor and copy assignment operator
    HeapMemoryFixedSize(const HeapMemoryFixedSize &) = delete;
    HeapMemoryFixedSize &operator=(const HeapMemoryFixedSize &) = delete;

    HeapMemoryFixedSize(HeapMemoryFixedSize &&other) noexcept
        : mData(std::move(other.mData)), mSize(other.mSize) {
        other.mSize = 0;
    }

    HeapMemoryFixedSize &operator=(HeapMemoryFixedSize &&other) noexcept {
        if (this != &other) {
            mData       = std::move(other.mData);
            mSize       = other.mSize;
            other.mSize = 0;
        }
        return *this;
    }

    explicit operator bool() const {
        return mData != nullptr;
    }

    [[nodiscard]] const void *data() const {
        return mData.get();
    }

    [[nodiscard]] std::size_t size() const {
        return mSize;
    }

private:
    std::unique_ptr<uint8_t[]> mData{};
    std::size_t mSize{};
};