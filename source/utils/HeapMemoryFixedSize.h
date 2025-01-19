#pragma once

#include <memory>
#include <span>
#include <vector>

#include <cstdint>

class HeapMemoryFixedSizePool {
public:
    class MemorySegmentInfo {
    public:
        MemorySegmentInfo(void *data, size_t size);

        [[nodiscard]] void *data() const;
        [[nodiscard]] size_t size() const;

    private:
        void *mData;
        size_t mSize;
    };
    HeapMemoryFixedSizePool();

    HeapMemoryFixedSizePool(std::initializer_list<size_t> segmentSizes);
    HeapMemoryFixedSizePool(std::span<const std::size_t> segmentSizes);

    // Delete the copy constructor and copy assignment operator
    HeapMemoryFixedSizePool(const HeapMemoryFixedSizePool &) = delete;
    HeapMemoryFixedSizePool &operator=(const HeapMemoryFixedSizePool &) = delete;

    HeapMemoryFixedSizePool(HeapMemoryFixedSizePool &&other) noexcept;

    HeapMemoryFixedSizePool &operator=(HeapMemoryFixedSizePool &&other) noexcept;

    [[nodiscard]] uint32_t numberOfSegments() const;

    explicit operator bool() const;

    MemorySegmentInfo operator[](int idx) const;

private:
    std::unique_ptr<uint8_t[]> mData{};
    std::size_t mTotalSize{};
    std::vector<MemorySegmentInfo> mSegmentInfos;
};
