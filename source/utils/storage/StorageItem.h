#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <cstdint>

enum class StorageItemType { None,
                             Boolean,
                             String,
                             Binary,
                             S64,
                             U64,
                             Double };

class StorageItem {
public:
    explicit StorageItem(std::string_view key);

    [[nodiscard]] uint32_t getHandle() const;

    // Setters for different types
    void setValue(bool value);

    void setValue(const std::string &value);

    void setValue(int32_t value);

    void setValue(int64_t value);

    void setValue(uint64_t value);

    void setValue(uint32_t value);

    void setValue(float value);

    void setValue(double value);

    void setValue(const std::vector<uint8_t> &data);

    void setValue(const uint8_t *data, size_t size);

    bool getValue(bool &result) const;

    bool getValue(int32_t &result) const;

    bool getValue(int64_t &result) const;

    bool getValue(uint32_t &result) const;

    bool getValue(uint64_t &result) const;

    bool getValue(float &result) const;

    bool getValue(double &result) const;

    bool getValue(std::string &result) const;

    bool getValue(std::vector<uint8_t> &result) const;

    [[nodiscard]] StorageItemType getType() const {
        return mType;
    }

    [[nodiscard]] const std::string &getKey() const {
        return mKey;
    }

    bool getItemSizeString(uint32_t &outSize) const;

    bool getItemSizeBinary(uint32_t &outSize) const;

    bool attemptBinaryConversion();

private:
    std::variant<std::monostate, std::string, bool, int64_t, uint64_t, double, std::vector<uint8_t>> mData = std::monostate{};
    StorageItemType mType                                                                                  = StorageItemType::None;
    std::string mKey                                                                                       = {};

    bool mBinaryConversionDone = true;

    std::unique_ptr<uint32_t> mHandle = std::make_unique<uint32_t>();
};
