/****************************************************************************
 * Copyright (C) 2018 Maschell
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

#include <string>
#include <vector>
#include <map>
#include <coreinit/cache.h>
#include <coreinit/memexpheap.h>
#include <wups.h>
#include <whb/file.h>
#include "PluginData.h"
#include "PluginInformationFactory.h"
#include "HookData.h"
#include "SectionInfo.h"
#include "elfio/elfio.hpp"
#include "utils/utils.h"
#include "utils/ElfUtils.h"
#include "utils/StringTools.h"

using namespace ELFIO;

std::optional<PluginInformation> PluginInformationFactory::load(const PluginData &pluginData, MEMHeapHandle heapHandle, relocation_trampolin_entry_t *trampolin_data, uint32_t trampolin_data_length, uint8_t trampolinId) {
    auto readerOpt = pluginData.getReader();
    if (!readerOpt) {
        DEBUG_FUNCTION_LINE("Can't find or process ELF file");
        return std::nullopt;
    }
    auto reader = readerOpt.value();
    PluginInformation pluginInfo;

    uint32_t sec_num = reader->sections.size();
    uint8_t **destinations = (uint8_t **) malloc(sizeof(uint8_t *) * sec_num);

    uint32_t totalSize = 0;

    uint32_t text_size = 0;
    uint32_t data_size = 0;

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader->sections[i];
        if (psec->get_type() == 0x80000002) {
            continue;
        }

        if ((psec->get_type() == SHT_PROGBITS || psec->get_type() == SHT_NOBITS) && (psec->get_flags() & SHF_ALLOC)) {
            uint32_t sectionSize = psec->get_size();
            uint32_t address = (uint32_t) psec->get_address();
            if ((address >= 0x02000000) && address < 0x10000000) {
                text_size += sectionSize;
            } else if ((address >= 0x10000000) && address < 0xC0000000) {
                data_size += sectionSize;
            }
            if (psec->get_name().rfind(".wups.", 0) == 0) {
                data_size += sectionSize;
            }
        }
    }

    void *text_data = MEMAllocFromExpHeapEx(heapHandle, text_size, 0x1000);

    if (text_data == NULL) {
        DEBUG_FUNCTION_LINE("Failed to alloc memory for the .text section (%d bytes)\n", text_size);

        return std::nullopt;
    }
    DEBUG_FUNCTION_LINE("Allocated %d kb from ExpHeap", text_size / 1024);
    void *data_data = MEMAllocFromExpHeapEx(heapHandle, data_size, 0x1000);
    if (data_data == NULL) {
        DEBUG_FUNCTION_LINE("Failed to alloc memory for the .data section (%d bytes)\n", data_size);

        MEMFreeToExpHeap(heapHandle, text_data);
        return std::nullopt;
    }
    DEBUG_FUNCTION_LINE("Allocated %d kb from ExpHeap", data_size / 1024);

    uint32_t entrypoint = (uint32_t) text_data + (uint32_t) reader->get_entry() - 0x02000000;

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader->sections[i];
        if (psec->get_type() == 0x80000002) {
            continue;
        }

        if ((psec->get_type() == SHT_PROGBITS || psec->get_type() == SHT_NOBITS) && (psec->get_flags() & SHF_ALLOC)) {
            uint32_t sectionSize = psec->get_size();
            uint32_t address = (uint32_t) psec->get_address();

            uint32_t destination = address;
            if ((address >= 0x02000000) && address < 0x10000000) {
                destination += (uint32_t) text_data;
                destination -= 0x02000000;
                destinations[psec->get_index()] = (uint8_t *) text_data;
            } else if ((address >= 0x10000000) && address < 0xC0000000) {
                destination += (uint32_t) data_data;
                destination -= 0x10000000;
                destinations[psec->get_index()] = (uint8_t *) data_data;
            } else if (address >= 0xC0000000) {
                destination += (uint32_t) data_data;
                destination -= 0xC0000000;
                //destinations[psec->get_index()] = (uint8_t *) data_data;
                //destinations[psec->get_index()] -= 0xC0000000;
            } else {
                DEBUG_FUNCTION_LINE("Unhandled case");
                free(destinations);
                MEMFreeToExpHeap(heapHandle, text_data);
                MEMFreeToExpHeap(heapHandle, data_data);
                return std::nullopt;
            }

            const char *p = psec->get_data();

            if (psec->get_type() == SHT_NOBITS) {
                DEBUG_FUNCTION_LINE("memset section %s %08X to 0 (%d bytes)", psec->get_name().c_str(), destination, sectionSize);
                memset((void *) destination, 0, sectionSize);
            } else if (psec->get_type() == SHT_PROGBITS) {
                DEBUG_FUNCTION_LINE("Copy section %s %08X -> %08X (%d bytes)", psec->get_name().c_str(), p, destination, sectionSize);
                memcpy((void *) destination, p, sectionSize);
            }

            pluginInfo.addSectionInfo(SectionInfo(psec->get_name(), destination, sectionSize));
            DEBUG_FUNCTION_LINE("Saved %s section info. Location: %08X size: %08X", psec->get_name().c_str(), destination, sectionSize);

            totalSize += sectionSize;

            DCFlushRange((void *) destination, sectionSize);
            ICInvalidateRange((void *) destination, sectionSize);
        }
    }

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader->sections[i];
        if ((psec->get_type() == SHT_PROGBITS || psec->get_type() == SHT_NOBITS) && (psec->get_flags() & SHF_ALLOC)) {
            DEBUG_FUNCTION_LINE("Linking (%d)... %s at %08X", i, psec->get_name().c_str(), destinations[psec->get_index()]);

            if (!linkSection(pluginData, psec->get_index(), (uint32_t) destinations[psec->get_index()], (uint32_t) text_data, (uint32_t) data_data, trampolin_data, trampolin_data_length, trampolinId)) {
                DEBUG_FUNCTION_LINE("elfLink failed");
                free(destinations);
                MEMFreeToExpHeap(heapHandle, text_data);
                MEMFreeToExpHeap(heapHandle, data_data);
                return std::nullopt;
            }
        }
    }
    std::vector<RelocationData> relocationData = getImportRelocationData(pluginData, destinations);

    for (auto const &reloc : relocationData) {
        pluginInfo.addRelocationData(reloc);
    }

    DCFlushRange((void *) text_data, text_size);
    ICInvalidateRange((void *) text_data, text_size);
    DCFlushRange((void *) data_data, data_size);
    ICInvalidateRange((void *) data_data, data_size);

    free(destinations);

    pluginInfo.setTrampolinId(trampolinId);

    DEBUG_FUNCTION_LINE("Saved entrypoint as %08X", entrypoint);

    std::optional<SectionInfo> secInfo = pluginInfo.getSectionInfo(".wups.hooks");
    if (secInfo && secInfo->getSize() > 0) {
        size_t entries_count = secInfo->getSize() / sizeof(wups_loader_hook_t);
        wups_loader_hook_t *entries = (wups_loader_hook_t *) secInfo->getAddress();
        if (entries != NULL) {
            for (size_t j = 0; j < entries_count; j++) {
                wups_loader_hook_t *hook = &entries[j];
                DEBUG_FUNCTION_LINE("Saving hook of plugin Type: %08X, target: %08X"/*,pluginData.getPluginInformation()->getName().c_str()*/, hook->type, (void *) hook->target);
                HookData hook_data((void *) hook->target, hook->type);
                pluginInfo.addHookData(hook_data);
            }
        }
    }

    secInfo = pluginInfo.getSectionInfo(".wups.load");
    if (secInfo && secInfo->getSize() > 0) {
        size_t entries_count = secInfo->getSize() / sizeof(wups_loader_entry_t);
        wups_loader_entry_t *entries = (wups_loader_entry_t *) secInfo->getAddress();
        if (entries != NULL) {
            for (size_t j = 0; j < entries_count; j++) {
                wups_loader_entry_t *cur_function = &entries[j];
                DEBUG_FUNCTION_LINE("Saving function \"%s\" of plugin . PA:%08X VA:%08X Library: %08X, target: %08X, call_addr: %08X", cur_function->_function.name/*,pluginData.getPluginInformation()->getName().c_str()*/,
                                    cur_function->_function.physical_address, cur_function->_function.virtual_address, cur_function->_function.library, cur_function->_function.target, (void *) cur_function->_function.call_addr);
                FunctionData function_data((void *) cur_function->_function.physical_address, (void *) cur_function->_function.virtual_address, cur_function->_function.name, cur_function->_function.library,
                                           (void *) cur_function->_function.target, (void *) cur_function->_function.call_addr);
                pluginInfo.addFunctionData(function_data);
            }
        }
    }

    // Save the addresses for the allocated. This way we can free it again :)
    pluginInfo.allocatedDataMemoryAddress = data_data;
    pluginInfo.allocatedTextMemoryAddress = text_data;

    return pluginInfo;
}

std::vector<RelocationData> PluginInformationFactory::getImportRelocationData(const PluginData &pluginData, uint8_t **destinations) {
    auto readerOpt = pluginData.getReader();

    std::vector<RelocationData> result;
    if (!readerOpt) {
        return result;
    }
    auto reader = readerOpt.value();
    std::map<uint32_t, std::string> infoMap;

    uint32_t sec_num = reader->sections.size();

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader->sections[i];
        if (psec->get_type() == 0x80000002) {
            infoMap[i] = psec->get_name();
        }
    }

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader->sections[i];
        if (psec->get_type() == SHT_RELA || psec->get_type() == SHT_REL) {
            DEBUG_FUNCTION_LINE("Found relocation section %s", psec->get_name().c_str());
            relocation_section_accessor rel(*reader, psec);
            for (uint32_t j = 0; j < (uint32_t) rel.get_entries_num(); ++j) {
                Elf64_Addr offset;
                Elf_Word type;
                Elf_Sxword addend;
                std::string sym_name;
                Elf64_Addr sym_value;
                Elf_Half sym_section_index;

                if (!rel.get_entry(j, offset, sym_value, sym_name, type, addend, sym_section_index)) {
                    DEBUG_FUNCTION_LINE("Failed to get relocation");
                    break;
                }

                uint32_t adjusted_sym_value = (uint32_t) sym_value;
                if (adjusted_sym_value < 0xC0000000) {
                    continue;
                }

                std::string fimport = ".fimport_";
                std::string dimport = ".dimport_";

                bool isData = false;

                std::string rplName = "";
                std::string rawSectionName = infoMap[sym_section_index];

                if (rawSectionName.size() < fimport.size()) {
                    DEBUG_FUNCTION_LINE("Section name was shorter than expected, skipping this relocation");
                    continue;
                } else if (std::equal(fimport.begin(), fimport.end(), rawSectionName.begin())) {
                    rplName = rawSectionName.substr(fimport.size());
                } else if (std::equal(dimport.begin(), dimport.end(), rawSectionName.begin())) {
                    rplName = rawSectionName.substr(dimport.size());
                    isData = true;
                } else {
                    DEBUG_FUNCTION_LINE("invalid section name\n");
                    continue;
                }

                ImportRPLInformation rplInfo(rplName, isData);

                uint32_t section_index = psec->get_info();

                result.push_back(RelocationData(type, offset - 0x02000000, addend, (void *) (destinations[section_index]), sym_name, rplInfo));
            }
        }
    }
    return result;
}

bool PluginInformationFactory::linkSection(const PluginData &pluginData, uint32_t section_index, uint32_t destination, uint32_t base_text, uint32_t base_data, relocation_trampolin_entry_t *trampolin_data, uint32_t trampolin_data_length,
                                           uint8_t trampolinId) {
    auto readerOpt = pluginData.getReader();
    if (!readerOpt) {
        return false;
    }
    auto reader = readerOpt.value();
    uint32_t sec_num = reader->sections.size();

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader->sections[i];
        if (psec->get_info() == section_index) {
            DEBUG_FUNCTION_LINE("Found relocation section %s", psec->get_name().c_str());
            relocation_section_accessor rel(*reader, psec);
            for (uint32_t j = 0; j < (uint32_t) rel.get_entries_num(); ++j) {
                Elf64_Addr offset;
                Elf_Word type;
                Elf_Sxword addend;
                std::string sym_name;
                Elf64_Addr sym_value;
                Elf_Half sym_section_index;

                if (!rel.get_entry(j, offset, sym_value, sym_name, type, addend, sym_section_index)) {
                    DEBUG_FUNCTION_LINE("Failed to get relocation");
                    break;
                }

                uint32_t adjusted_sym_value = (uint32_t) sym_value;
                if ((adjusted_sym_value >= 0x02000000) && adjusted_sym_value < 0x10000000) {
                    adjusted_sym_value -= 0x02000000;
                    adjusted_sym_value += base_text;
                } else if ((adjusted_sym_value >= 0x10000000) && adjusted_sym_value < 0xC0000000) {
                    adjusted_sym_value -= 0x10000000;
                    adjusted_sym_value += base_data;
                } else if (adjusted_sym_value >= 0xC0000000) {
                    //DEBUG_FUNCTION_LINE("Skip imports");
                    // Skip imports
                    continue;
                } else if (adjusted_sym_value == 0x0) {
                    //
                } else {
                    DEBUG_FUNCTION_LINE("Unhandled case %08X", adjusted_sym_value);
                    return false;
                }

                uint32_t adjusted_offset = (uint32_t) offset;
                if ((offset >= 0x02000000) && offset < 0x10000000) {
                    adjusted_offset -= 0x02000000;
                } else if ((adjusted_offset >= 0x10000000) && adjusted_offset < 0xC0000000) {
                    adjusted_offset -= 0x10000000;
                } else if (adjusted_offset >= 0xC0000000) {
                    adjusted_offset -= 0xC0000000;
                }

                if (sym_section_index == SHN_ABS) {
                    //
                } else if (sym_section_index > SHN_LORESERVE) {
                    DEBUG_FUNCTION_LINE("NOT IMPLEMENTED: %04X", sym_section_index);
                    return false;
                }
                if (false) {
                    DEBUG_FUNCTION_LINE("sym_value %08X adjusted_sym_value %08X offset %08X adjusted_offset %08X", (uint32_t) sym_value, adjusted_sym_value, (uint32_t) offset, adjusted_offset);
                }
                if (!ElfUtils::elfLinkOne(type, adjusted_offset, addend, destination, adjusted_sym_value, trampolin_data, trampolin_data_length, RELOC_TYPE_FIXED, trampolinId)) {
                    DEBUG_FUNCTION_LINE("Link failed");
                    return false;
                }
            }
            DEBUG_FUNCTION_LINE("done");
            return true;
        }
    }
    DEBUG_FUNCTION_LINE("Failed to find relocation section");
    return true;
}
