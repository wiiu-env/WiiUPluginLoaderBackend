#include "PluginData.h"
#include "utils/logger.h"
#include "utils/utils.h"

PluginData::PluginData(const std::vector<uint8_t> &input) : length(input.size()) {
    auto data_copy = make_unique_nothrow<uint8_t[]>(length);
    if (!data_copy) {
        DEBUG_FUNCTION_LINE_ERR("Failed to allocate space on default heap");
        this->length = 0;
    } else {
        DEBUG_FUNCTION_LINE_VERBOSE("Allocated %d kb on default heap", length / 1024);
        memcpy(data_copy.get(), &input[0], length);
        this->buffer = std::move(data_copy);
    }
}
