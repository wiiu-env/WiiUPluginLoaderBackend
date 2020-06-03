#include "PluginData.h"


PluginData::PluginData(const PluginData &obj) {
    this->buffer = obj.buffer;
    this->heapHandle = obj.heapHandle;
    this->memoryType = obj.memoryType;
    this->length = obj.length;
}

void PluginData::freeMemory() {
    if (buffer == NULL) {
        return;
    }

    switch (memoryType) {
        default:
        case eMemTypeExpHeap:
            MEMFreeToExpHeap(this->heapHandle, buffer);
            this->buffer = NULL;
            break;
        case eMemTypeMEM2:
            free(this->buffer);
            this->buffer = NULL;
            break;
    }
}

PluginData::PluginData(std::vector<uint8_t> buffer) : PluginData(buffer, 0, eMemTypeMEM2) {
}

PluginData::PluginData(std::vector<uint8_t> input, MEMHeapHandle heapHandle, eMemoryTypes memoryType) :
        heapHandle(heapHandle),
        memoryType(memoryType),
        length(input.size()) {
    void *data_copy = NULL;
    switch (memoryType) {
        default:
        case eMemTypeExpHeap:
            data_copy = MEMAllocFromExpHeapEx(heapHandle, length, 4);
            DEBUG_FUNCTION_LINE("Allocated %d kb from ExpHeap", length / 1024);
            if (data_copy == NULL) {
                DEBUG_FUNCTION_LINE("Failed to allocate space on exp heap");
            } else {
                memcpy(data_copy, &input[0], length);
            }
            this->buffer = data_copy;
            DEBUG_FUNCTION_LINE("copied data to exp heap");
            break;
        case eMemTypeMEM2:
            data_copy = memalign(length, 4);
            if (data_copy == NULL) {
                DEBUG_FUNCTION_LINE("Failed to allocate space on default heap");
            } else {
                memcpy(data_copy, &input[0], length);
            }
            this->buffer = data_copy;
            break;
    }
}

std::optional<PluginData> PluginData::createFromExistingData(const void *buffer, MEMHeapHandle heapHandle, eMemoryTypes memoryType, const size_t length) {
    return std::nullopt;
}
