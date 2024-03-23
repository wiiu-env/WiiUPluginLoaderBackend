#include "StorageItem.h"

void StorageItem::setValue(const std::string &value) {
    mData                 = value;
    mType                 = StorageItemType::String;
    mBinaryConversionDone = false;
}

void StorageItem::setValue(bool value) {
    mData                 = value;
    mType                 = StorageItemType::Boolean;
    mBinaryConversionDone = true;
}

void StorageItem::setValue(int32_t value) {
    mData                 = (int64_t) value;
    mType                 = StorageItemType::S64;
    mBinaryConversionDone = true;
}

void StorageItem::setValue(int64_t value) {
    mData                 = value;
    mType                 = StorageItemType::S64;
    mBinaryConversionDone = true;
}

void StorageItem::setValue(uint64_t value) {
    mData                 = value;
    mType                 = StorageItemType::U64;
    mBinaryConversionDone = true;
}

void StorageItem::setValue(uint32_t value) {
    mData                 = (uint64_t) value;
    mType                 = StorageItemType::U64;
    mBinaryConversionDone = true;
}

void StorageItem::setValue(float value) {
    mData                 = (double) value;
    mType                 = StorageItemType::Double;
    mBinaryConversionDone = true;
}

void StorageItem::setValue(double value) {
    mData                 = value;
    mType                 = StorageItemType::Double;
    mBinaryConversionDone = true;
}

void StorageItem::setValue(const std::vector<uint8_t> &data) {
    mData                 = data;
    mType                 = StorageItemType::Binary;
    mBinaryConversionDone = true;
}

void StorageItem::setValue(const uint8_t *data, size_t size) {
    setValue(std::vector<uint8_t>(data, data + size));
}

bool StorageItem::getValue(bool &result) const {
    if (mType == StorageItemType::Boolean) {
        result = std::get<bool>(mData);
        return true;
    } else if (mType == StorageItemType::S64) {
        result = !!(std::get<int64_t>(mData));
        return true;
    } else if (mType == StorageItemType::U64) {
        result = !!(std::get<uint64_t>(mData));
        return true;
    }
    return false;
}

bool StorageItem::getValue(int32_t &result) const {
    if (mType == StorageItemType::S64) {
        result = (int32_t) std::get<int64_t>(mData);
        return true;
    } else if (mType == StorageItemType::U64) {
        result = (int32_t) std::get<uint64_t>(mData);
        return true;
    }
    return false;
}

bool StorageItem::getValue(std::vector<uint8_t> &result) const {
    if (mType == StorageItemType::Binary) {
        result = std::get<std::vector<uint8_t>>(mData);
        return true;
    }
    return false;
}

bool StorageItem::getValue(std::string &result) const {
    if (mType == StorageItemType::String) {
        result = std::get<std::string>(mData);
        return true;
    }
    return false;
}

bool StorageItem::getValue(double &result) const {
    if (mType == StorageItemType::Double) {
        result = std::get<double>(mData);
        return true;
    }
    return false;
}

bool StorageItem::getValue(float &result) const {
    if (mType == StorageItemType::Double) {
        result = (float) std::get<double>(mData);
        return true;
    }
    return false;
}

bool StorageItem::getValue(uint64_t &result) const {
    if (mType == StorageItemType::U64) {
        result = std::get<uint64_t>(mData);
        return true;
    } else if (mType == StorageItemType::S64) {
        result = (uint64_t) std::get<int64_t>(mData);
        return true;
    }
    return false;
}

bool StorageItem::getValue(uint32_t &result) const {
    if (mType == StorageItemType::U64) {
        result = (uint32_t) std::get<uint64_t>(mData);
        return true;
    } else if (mType == StorageItemType::S64) {
        result = (uint32_t) std::get<int64_t>(mData);
        return true;
    }
    return false;
}

bool StorageItem::getValue(int64_t &result) const {
    if (mType == StorageItemType::S64) {
        result = std::get<int64_t>(mData);
        return true;
    } else if (mType == StorageItemType::U64) {
        result = (int64_t) std::get<uint64_t>(mData);
        return true;
    }
    return false;
}

bool StorageItem::getItemSizeString(uint32_t &outSize) const {
    if (mType == StorageItemType::String) {
        outSize = (std::get<std::string>(mData).length() + 1);
        return true;
    }
    return false;
}

bool StorageItem::getItemSizeBinary(uint32_t &outSize) const {
    if (mType == StorageItemType::Binary) {
        outSize = std::get<std::vector<uint8_t>>(mData).size();
        return true;
    }
    return false;
}

bool StorageItem::attemptBinaryConversion() {
    if (mBinaryConversionDone) {
        return true;
    }
    if (mType == StorageItemType::String) {
        auto &tmp     = std::get<std::string>(mData);
        auto dec_size = b64_decoded_size(tmp.c_str());
        if (dec_size > 0) {
            auto *dec = (uint8_t *) malloc(dec_size);
            if (dec) {
                if (b64_decode(tmp.c_str(), dec, dec_size)) {
                    setValue(dec, dec_size);
                }
                free(dec);
            } else {
                DEBUG_FUNCTION_LINE_WARN("Malloc failed for string->binary parsing");
                return false;
            }
        }
    }
    mBinaryConversionDone = true;
    return true;
}
