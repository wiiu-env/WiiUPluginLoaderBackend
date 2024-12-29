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

#include "PluginMetaInformationFactory.h"

#include "PluginData.h"
#include "PluginMetaInformation.h"
#include "elfio/elfio.hpp"
#include "fs/FSUtils.h"
#include "utils/logger.h"
#include "utils/wiiu_zlib.hpp"

#include <optional>

std::optional<PluginMetaInformation> PluginMetaInformationFactory::loadPlugin(const PluginData &pluginData, PluginParseErrors &error) {
    return loadPlugin(pluginData.getBuffer(), error);
}

std::optional<PluginMetaInformation> PluginMetaInformationFactory::loadPlugin(std::string_view filePath, PluginParseErrors &error) {
    std::vector<uint8_t> buffer;
    if (FSUtils::LoadFileToMem(filePath, buffer) < 0) {
        DEBUG_FUNCTION_LINE_ERR("Failed to load file to memory");
        error = PLUGIN_PARSE_ERROR_IO_ERROR;
        return {};
    }
    return loadPlugin(buffer, error);
}

std::optional<PluginMetaInformation> PluginMetaInformationFactory::loadPlugin(std::span<const uint8_t> buffer, PluginParseErrors &error) {
    if (buffer.empty()) {
        error = PLUGIN_PARSE_ERROR_BUFFER_EMPTY;
        DEBUG_FUNCTION_LINE_ERR("Buffer is empty");
        return {};
    }
    ELFIO::elfio reader(new wiiu_zlib);

    if (!reader.load(reinterpret_cast<const char *>(buffer.data()), buffer.size())) {
        error = PLUGIN_PARSE_ERROR_ELFIO_PARSE_FAILED;
        DEBUG_FUNCTION_LINE_ERR("Can't find or process ELF file");
        return {};
    }

    size_t pluginSize = 0;

    PluginMetaInformation pluginInfo;

    uint32_t sec_num = reader.sections.size();

    bool hasMetaSection = false;
    for (uint32_t i = 0; i < sec_num; ++i) {
        ELFIO::section *psec = reader.sections[i];

        // Calculate total size:
        if ((psec->get_type() == ELFIO::SHT_PROGBITS || psec->get_type() == ELFIO::SHT_NOBITS) && (psec->get_flags() & ELFIO::SHF_ALLOC)) {
            uint32_t sectionSize = psec->get_size();
            auto address         = (uint32_t) psec->get_address();
            if ((address >= 0x02000000) && address < 0x10000000) {
                pluginSize += sectionSize;
            } else if ((address >= 0x10000000) && address < 0xC0000000) {
                pluginSize += sectionSize;
            }
        }

        // Get meta information and check WUPS version:
        if (psec->get_name() == ".wups.meta") {
            hasMetaSection          = true;
            const void *sectionData = psec->get_data();
            uint32_t sectionSize    = psec->get_size();

            char *curEntry = (char *) sectionData;
            while ((uint32_t) curEntry < (uint32_t) sectionData + sectionSize) {
                if (*curEntry == '\0') {
                    curEntry++;
                    continue;
                }

                auto firstFound = std::string(curEntry).find_first_of('=');
                if (firstFound != std::string::npos) {
                    curEntry[firstFound] = '\0';
                    std::string key(curEntry);
                    std::string value(curEntry + firstFound + 1);

                    if (key == "name") {
                        pluginInfo.setName(value);
                    } else if (key == "author") {
                        pluginInfo.setAuthor(value);
                    } else if (key == "version") {
                        pluginInfo.setVersion(value);
                    } else if (key == "license") {
                        pluginInfo.setLicense(value);
                    } else if (key == "buildtimestamp") {
                        pluginInfo.setBuildTimestamp(value);
                    } else if (key == "description") {
                        pluginInfo.setDescription(value);
                    } else if (key == "storage_id") {
                        pluginInfo.setStorageId(value);
                    } else if (key == "wups") {
                        if (value == "0.7.1") {
                            pluginInfo.setWUPSVersion(0, 7, 1);
                        } else if (value == "0.8.1") {
                            pluginInfo.setWUPSVersion(0, 8, 1);
                        } else if (value == "0.8.2") {
                            pluginInfo.setWUPSVersion(0, 8, 2);
                        } else {
                            error = PLUGIN_PARSE_ERROR_INCOMPATIBLE_VERSION;
                            DEBUG_FUNCTION_LINE_ERR("Warning: Ignoring plugin - Unsupported WUPS version: %s.", value.c_str());
                            return {};
                        }
                    }
                }
                curEntry += strlen(curEntry) + 1;
            }
        }
    }
    if (!hasMetaSection) {
        DEBUG_FUNCTION_LINE_ERR("File has no \".wups.meta\" section");
        error = PLUGIN_PARSE_ERROR_NO_PLUGIN;
        return {};
    }

    pluginInfo.setSize(pluginSize);

    error = PLUGIN_PARSE_ERROR_NONE;

    return pluginInfo;
}
