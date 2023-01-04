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
#include "elfio/elfio.hpp"
#include "fs/FSUtils.h"
#include "utils/logger.h"
#include "utils/wiiu_zlib.hpp"
#include <memory>

std::optional<std::unique_ptr<PluginMetaInformation>> PluginMetaInformationFactory::loadPlugin(const std::shared_ptr<PluginData> &pluginData) {
    if (!pluginData->buffer) {
        DEBUG_FUNCTION_LINE_ERR("Buffer is empty");
        return {};
    }
    ELFIO::elfio reader(new wiiu_zlib);
    if (!reader.load(reinterpret_cast<const char *>(pluginData->buffer.get()), pluginData->length)) {
        DEBUG_FUNCTION_LINE_ERR("Can't process PluginData in elfio");
        return {};
    }
    return loadPlugin(reader);
}

std::optional<std::unique_ptr<PluginMetaInformation>> PluginMetaInformationFactory::loadPlugin(const std::string &filePath) {
    ELFIO::elfio reader(new wiiu_zlib);

    uint8_t *buffer = nullptr;
    uint32_t length = 0;
    if (FSUtils::LoadFileToMem(filePath.c_str(), &buffer, &length) < 0) {
        DEBUG_FUNCTION_LINE_ERR("Failed to load file to memory");
        return {};
    }

    if (!reader.load(reinterpret_cast<const char *>(buffer), length)) {
        DEBUG_FUNCTION_LINE_ERR("Can't process PluginData in elfio");
        return {};
    }
    auto res = loadPlugin(reader);
    free(buffer);
    return res;
}

std::optional<std::unique_ptr<PluginMetaInformation>> PluginMetaInformationFactory::loadPlugin(char *buffer, size_t size) {
    ELFIO::elfio reader(new wiiu_zlib);

    if (!reader.load(reinterpret_cast<const char *>(buffer), size)) {
        DEBUG_FUNCTION_LINE_ERR("Can't find or process ELF file");
        return std::nullopt;
    }

    return loadPlugin(reader);
}

std::optional<std::unique_ptr<PluginMetaInformation>> PluginMetaInformationFactory::loadPlugin(const ELFIO::elfio &reader) {
    size_t pluginSize = 0;

    auto pluginInfo = std::unique_ptr<PluginMetaInformation>(new PluginMetaInformation);

    uint32_t sec_num = reader.sections.size();

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
                        pluginInfo->setName(value);
                    } else if (key == "author") {
                        pluginInfo->setAuthor(value);
                    } else if (key == "version") {
                        pluginInfo->setVersion(value);
                    } else if (key == "license") {
                        pluginInfo->setLicense(value);
                    } else if (key == "buildtimestamp") {
                        pluginInfo->setBuildTimestamp(value);
                    } else if (key == "description") {
                        pluginInfo->setDescription(value);
                    } else if (key == "storage_id") {
                        pluginInfo->setStorageId(value);
                    } else if (key == "wups") {
                        if (value != "0.7.1") {
                            DEBUG_FUNCTION_LINE_ERR("Warning: Ignoring plugin - Unsupported WUPS version: %s.", value.c_str());
                            return std::nullopt;
                        }
                    }
                }
                curEntry += strlen(curEntry) + 1;
            }
        }
    }

    pluginInfo->setSize(pluginSize);

    return pluginInfo;
}
