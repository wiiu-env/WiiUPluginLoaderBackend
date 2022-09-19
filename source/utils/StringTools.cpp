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

#include <cstring>
#include <string>
#include <strings.h>
#include <utils/StringTools.h>
#include <wut_types.h>

std::string StringTools::truncate(const std::string &str, size_t width, bool show_ellipsis) {
    if (str.length() > width - 3) {
        if (show_ellipsis) {
            return str.substr(0, width - 3) + "...";
        } else {
            return str.substr(0, width - 3);
        }
    }
    return str;
}

int32_t StringTools::strtokcmp(const char *string, const char *compare, const char *separator) {
    if (!string || !compare)
        return -1;

    char TokCopy[512];
    strncpy(TokCopy, compare, sizeof(TokCopy));
    TokCopy[511] = '\0';

    char *strTok = strtok(TokCopy, separator);

    while (strTok != nullptr) {
        if (strcasecmp(string, strTok) == 0) {
            return 0;
        }
        strTok = strtok(nullptr, separator);
    }

    return -1;
}
