#pragma once

#include <cstdint>
#include <memory>

class HeapMemoryFixedSize {
public:
    HeapMemoryFixedSize();

    explicit HeapMemoryFixedSize(std::size_t size);

    // Delete the copy constructor and copy assignment operator
    HeapMemoryFixedSize(const HeapMemoryFixedSize &) = delete;
    HeapMemoryFixedSize &operator=(const HeapMemoryFixedSize &) = delete;

    HeapMemoryFixedSize(HeapMemoryFixedSize &&other) noexcept;

    HeapMemoryFixedSize &operator=(HeapMemoryFixedSize &&other) noexcept;

    explicit operator bool() const;

    [[nodiscard]] const void *data() const;

    [[nodiscard]] std::size_t size() const;

private:
    std::unique_ptr<uint8_t[]> mData{};
    std::size_t mSize{};
};