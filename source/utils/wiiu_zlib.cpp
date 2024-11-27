#include "wiiu_zlib.hpp"
#include "utils.h"

#include <cstring>
#include <zlib.h>

std::unique_ptr<char[]> wiiu_zlib::inflate(const char *data, const ELFIO::endianess_convertor *convertor, const ELFIO::Elf_Xword compressed_size, ELFIO::Elf_Xword &uncompressed_size) const {
    read_uncompressed_size(data, convertor, uncompressed_size);
    auto result = make_unique_nothrow<char[]>(static_cast<uint32_t>(uncompressed_size + 1));
    if (result == nullptr) {
        return nullptr;
    }

    z_stream s = {};

    s.zalloc = Z_NULL;
    s.zfree  = Z_NULL;
    s.opaque = Z_NULL;

    if (inflateInit_(&s, ZLIB_VERSION, sizeof(s)) != Z_OK) {
        return nullptr;
    }

    s.avail_in  = compressed_size - 4;
    s.next_in   = (Bytef *) data;
    s.avail_out = uncompressed_size;
    s.next_out  = (Bytef *) result.get();

    const int z_ret = ::inflate(&s, Z_FINISH);
    inflateEnd(&s);

    if (z_ret != Z_OK && z_ret != Z_STREAM_END) {
        return nullptr;
    }

    result[uncompressed_size] = '\0';
    return result;
}

std::unique_ptr<char[]> wiiu_zlib::deflate(const char *data, const ELFIO::endianess_convertor *convertor, ELFIO::Elf_Xword decompressed_size, ELFIO::Elf_Xword &compressed_size) const {
    auto result = make_unique_nothrow<char[]>(static_cast<uint32_t>(decompressed_size));
    if (result == nullptr) {
        return nullptr;
    }

    int z_ret;
    z_stream s = {};

    s.zalloc = Z_NULL;
    s.zfree  = Z_NULL;
    s.opaque = Z_NULL;

    if ((z_ret = deflateInit(&s, Z_DEFAULT_COMPRESSION)) != Z_OK) {
        return nullptr;
    }

    s.avail_in  = decompressed_size;
    s.next_in   = (Bytef *) data;
    s.avail_out = decompressed_size - 4;
    s.next_out  = (Bytef *) result.get() + 4;

    z_ret           = ::deflate(&s, Z_FINISH);
    compressed_size = decompressed_size - s.avail_out;
    deflateEnd(&s);

    if (z_ret != Z_OK && z_ret != Z_STREAM_END) {
        compressed_size = 0;
        return nullptr;
    }

    write_compressed_size(result, convertor, compressed_size);
    result[compressed_size] = '\0';
    return result;
}

void wiiu_zlib::read_uncompressed_size(const char *&data, const ELFIO::endianess_convertor *convertor, ELFIO::Elf_Xword &uncompressed_size) {
    union _int32buffer {
        uint32_t word;
        char bytes[4];
    } int32buffer = {};
    memcpy(int32buffer.bytes, data, 4);
    data += 4;
    uncompressed_size = (*convertor)(int32buffer.word);
}

void wiiu_zlib::write_compressed_size(std::unique_ptr<char[]> &result, const ELFIO::endianess_convertor *convertor, const ELFIO::Elf_Xword compressed_size) {
    union _int32buffer {
        uint32_t word;
        char bytes[4];
    } int32buffer = {};

    int32buffer.word = (*convertor)(compressed_size);
    memcpy(result.get(), int32buffer.bytes, 4);
}