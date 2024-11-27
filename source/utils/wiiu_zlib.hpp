#pragma once

/****************************************************************************
 * Copyright (C) 2018-2020 Nathan Strong
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include <memory>

#include <elfio/elfio_utils.hpp>

class wiiu_zlib final : public ELFIO::compression_interface {
public:
    std::unique_ptr<char[]> inflate(const char *data, const ELFIO::endianess_convertor *convertor, ELFIO::Elf_Xword compressed_size, ELFIO::Elf_Xword &uncompressed_size) const override;

    std::unique_ptr<char[]> deflate(const char *data, const ELFIO::endianess_convertor *convertor, ELFIO::Elf_Xword decompressed_size, ELFIO::Elf_Xword &compressed_size) const override;

private:
    static void read_uncompressed_size(const char *&data, const ELFIO::endianess_convertor *convertor, ELFIO::Elf_Xword &uncompressed_size);

    static void write_compressed_size(std::unique_ptr<char[]> &result, const ELFIO::endianess_convertor *convertor, ELFIO::Elf_Xword compressed_size);
};