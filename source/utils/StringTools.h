/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#pragma once

#include "logger.h"
#include "utils.h"

#include <coreinit/debug.h>

#include <memory>
#include <string>

template<typename... Args>
std::string string_format(const std::string_view format, Args... args) {
    const int size_s = std::snprintf(nullptr, 0, format.data(), args...) + 1; // Extra space for '\0'
    const auto size  = static_cast<size_t>(size_s);
    const auto buf   = make_unique_nothrow<char[]>(size);
    if (!buf) {
        DEBUG_FUNCTION_LINE_ERR("string_format failed, not enough memory");
        OSFatal("string_format failed, not enough memory");
        return std::string("");
    }
    std::snprintf(buf.get(), size, format.data(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

class StringTools {
public:
    static std::string truncate(const std::string &str, size_t width, bool show_ellipsis = true);

    static int32_t strtokcmp(const char *string, const char *compare, const char *separator);

    static const char *FullpathToFilename(const char *path);

    static void RemoveDoubleSlashes(std::string &str);
};