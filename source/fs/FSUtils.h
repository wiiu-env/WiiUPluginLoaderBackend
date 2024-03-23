#pragma once

#include <string>
#include <vector>
#include <wut_types.h>

class FSUtils {
public:
    static int32_t LoadFileToMem(std::string_view filepath, std::vector<uint8_t> &buffer);

    static bool CreateSubfolder(std::string_view fullpath);
};
