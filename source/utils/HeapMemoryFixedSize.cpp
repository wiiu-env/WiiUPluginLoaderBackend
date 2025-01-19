#include "HeapMemoryFixedSize.h"

#include "logger.h"
#include "utils.h"

HeapMemoryFixedSizePool::MemorySegmentInfo::MemorySegmentInfo(void *data, const size_t size) : mData(data),
                                                                                               mSize(size) {}

void *HeapMemoryFixedSizePool::MemorySegmentInfo::data() const {
    return mData;
}

size_t HeapMemoryFixedSizePool::MemorySegmentInfo::size() const {
    return mSize;
}

HeapMemoryFixedSizePool::HeapMemoryFixedSizePool() = default;

HeapMemoryFixedSizePool::HeapMemoryFixedSizePool(const std::initializer_list<size_t> segmentSizes) : HeapMemoryFixedSizePool(std::span(segmentSizes.begin(), segmentSizes.size())) {}

HeapMemoryFixedSizePool::HeapMemoryFixedSizePool(std::span<const std::size_t> segmentSizes) {
    assert(!segmentSizes.empty());
    size_t totalSize = 0;
    for (const auto size : segmentSizes) {
        totalSize += size + 0x40; // add 0x40 bytes overhead for each entry to ensure padding to 0x40
    }

    mData      = make_unique_nothrow<uint8_t[]>(totalSize);
    mTotalSize = (mData ? totalSize : 0);
    if (mData) {
        auto address = reinterpret_cast<uint32_t>(mData.get());
        for (const auto size : segmentSizes) {
            address = ROUNDUP(address, 0x40);
            assert(address >= reinterpret_cast<uint32_t>(mData.get()) && address < reinterpret_cast<uint32_t>(mData.get()) + totalSize);
            mSegmentInfos.emplace_back(reinterpret_cast<void *>(address), size);
            address += size;
        }
    }
}

HeapMemoryFixedSizePool::HeapMemoryFixedSizePool(HeapMemoryFixedSizePool &&other) noexcept
    : mData(std::move(other.mData)), mTotalSize(other.mTotalSize), mSegmentInfos(std::move(other.mSegmentInfos)) {
    other.mTotalSize = 0;
}

HeapMemoryFixedSizePool &HeapMemoryFixedSizePool::operator=(HeapMemoryFixedSizePool &&other) noexcept {
    if (this != &other) {
        mData            = std::move(other.mData);
        mTotalSize       = other.mTotalSize;
        mSegmentInfos    = std::move(other.mSegmentInfos);
        other.mTotalSize = 0;
    }
    return *this;
}

uint32_t HeapMemoryFixedSizePool::numberOfSegments() const {
    return mSegmentInfos.size();
}

HeapMemoryFixedSizePool::operator bool() const {
    return mData != nullptr && mTotalSize > 0;
}

HeapMemoryFixedSizePool::MemorySegmentInfo HeapMemoryFixedSizePool::operator[](const int idx) const {
    if (idx < 0 || idx >= static_cast<int>(mSegmentInfos.size())) {
        DEBUG_FUNCTION_LINE_ERR("Out of bounce access (tried to access index %d; size is", idx, mSegmentInfos.size());
    }
    return mSegmentInfos[idx];
}