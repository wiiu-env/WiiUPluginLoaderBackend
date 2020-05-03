/****************************************************************************
 * Copyright (C) 2018-2020 Maschell
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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <whb/file.h>
#include "utils/StringTools.h"
#include "PluginMetaInformationFactory.h"
#include "PluginMetaInformation.h"
#include "elfio/elfio.hpp"

using namespace ELFIO;

std::optional<PluginMetaInformation> PluginMetaInformationFactory::loadPlugin(const PluginData &pluginData) {
    auto readerOpt = pluginData.getReader();

    // Load ELF data
    if (!readerOpt) {
        DEBUG_FUNCTION_LINE("Can't find or process ELF file");
        return std::nullopt;
    }
    auto reader = readerOpt.value();

    DEBUG_FUNCTION_LINE("Found elfio reader");

    size_t pluginSize = 0;

    PluginMetaInformation pluginInfo;
    uint32_t sec_num = reader->sections.size();

    DEBUG_FUNCTION_LINE("%d number of sections", sec_num);

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader->sections[i];

        // Calculate total size:
        if ((psec->get_type() == SHT_PROGBITS || psec->get_type() == SHT_NOBITS) && (psec->get_flags() & SHF_ALLOC)) {
            uint32_t sectionSize = psec->get_size();
            uint32_t address = (uint32_t) psec->get_address();
            if ((address >= 0x02000000) && address < 0x10000000) {
                pluginSize += sectionSize;
            } else if ((address >= 0x10000000) && address < 0xC0000000) {
                pluginSize += sectionSize;
            }
        }

        // Get meta information and check WUPS version:
        if (psec->get_name().compare(".wups.meta") == 0) {
            const void *sectionData = psec->get_data();
            uint32_t sectionSize = psec->get_size();

            char *curEntry = (char *) sectionData;
            while ((uint32_t) curEntry < (uint32_t) sectionData + sectionSize) {
                if (*curEntry == '\0') {
                    curEntry++;
                    continue;
                }

                auto firstFound = std::string(curEntry).find_first_of("=");
                if (firstFound != std::string::npos) {
                    curEntry[firstFound] = '\0';
                    std::string key(curEntry);
                    std::string value(curEntry + firstFound + 1);

                    if (key.compare("name") == 0) {
                        DEBUG_FUNCTION_LINE("Name = %s", value.c_str());
                        pluginInfo.setName(value);
                    } else if (key.compare("author") == 0) {
                        pluginInfo.setAuthor(value);
                    } else if (key.compare("version") == 0) {
                        pluginInfo.setVersion(value);
                    } else if (key.compare("license") == 0) {
                        pluginInfo.setLicense(value);
                    } else if (key.compare("buildtimestamp") == 0) {
                        pluginInfo.setBuildTimestamp(value);
                    } else if (key.compare("description") == 0) {
                        pluginInfo.setDescription(value);
                    } else if (key.compare("wups") == 0) {
                        if (value.compare("0.2") != 0) {
                            DEBUG_FUNCTION_LINE("Warning: Ignoring plugin - Unsupported WUPS version: %s.\n", value);
                            return std::nullopt;
                        }
                    }
                }
                curEntry += strlen(curEntry) + 1;
            }
        }
    }

    pluginInfo.setSize(pluginSize);

    return pluginInfo;
}
