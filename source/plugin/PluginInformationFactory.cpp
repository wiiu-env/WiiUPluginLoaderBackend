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

#include "PluginInformationFactory.h"
#include "../utils/ElfUtils.h"
#include "../utils/utils.h"
#include "utils/membuf.hpp"
#include "utils/wiiu_zlib.hpp"
#include <coreinit/cache.h>
#include <map>
#include <memory>
#include <string>
#include <wups/function_patching.h>

using namespace ELFIO;

std::optional<std::unique_ptr<PluginInformation>>
PluginInformationFactory::load(const std::shared_ptr<PluginData> &pluginData, relocation_trampoline_entry_t *trampoline_data, uint32_t trampoline_data_length,
                               uint8_t trampolineId) {
    if (!pluginData->buffer) {
        DEBUG_FUNCTION_LINE_ERR("Buffer was nullptr");
        return {};
    }
    elfio reader(new wiiu_zlib);
    membuf sbuf((char *) pluginData->buffer.get(), (char *) pluginData->buffer.get() + pluginData->length);
    std::istream in(&sbuf);

    if (!reader.load(in)) {
        DEBUG_FUNCTION_LINE_ERR("Can't process PluginData in elfio");
        return {};
    }

    auto pluginInfo = make_unique_nothrow<PluginInformation>();
    if (!pluginInfo) {
        DEBUG_FUNCTION_LINE_ERR("Failed to allocate PluginInformation");
        return {};
    }

    uint32_t sec_num = reader.sections.size();

    auto destinations = make_unique_nothrow<uint8_t *[]>(sec_num);
    if (!destinations) {
        DEBUG_FUNCTION_LINE_ERR("Failed alloc memory for destinations array");
        return {};
    }

    uint32_t totalSize = 0;

    uint32_t text_size = 0;
    uint32_t data_size = 0;

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader.sections[i];
        if (psec->get_type() == 0x80000002) {
            continue;
        }

        if ((psec->get_type() == SHT_PROGBITS || psec->get_type() == SHT_NOBITS) && (psec->get_flags() & SHF_ALLOC)) {
            uint32_t sectionSize = psec->get_size();
            auto address         = (uint32_t) psec->get_address();
            if ((address >= 0x02000000) && address < 0x10000000) {
                text_size += sectionSize;
            } else if ((address >= 0x10000000) && address < 0xC0000000) {
                data_size += sectionSize;
            }
            if (psec->get_name().starts_with(".wups.")) {
                data_size += sectionSize;
            }
        }
    }

    auto text_data = make_unique_nothrow<uint8_t[]>(text_size);
    if (!text_data) {
        DEBUG_FUNCTION_LINE_ERR("Failed to alloc memory for the .text section (%d bytes)", text_size);
        return {};
    }

    DEBUG_FUNCTION_LINE_VERBOSE("Allocated %d kb", text_size / 1024);

    auto data_data = make_unique_nothrow<uint8_t[]>(data_size);
    if (!data_data) {
        DEBUG_FUNCTION_LINE_ERR("Failed to alloc memory for the .data section (%d bytes)", data_size);
        return {};
    }

    DEBUG_FUNCTION_LINE_VERBOSE("Allocated %d kb", data_size / 1024);

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader.sections[i];
        if (psec->get_type() == 0x80000002 || psec->get_name() == ".wut_load_bounds") {
            continue;
        }

        if ((psec->get_type() == SHT_PROGBITS || psec->get_type() == SHT_NOBITS) && (psec->get_flags() & SHF_ALLOC)) {
            uint32_t sectionSize = psec->get_size();
            auto address         = (uint32_t) psec->get_address();

            uint32_t destination = address;
            if ((address >= 0x02000000) && address < 0x10000000) {
                destination += (uint32_t) text_data.get();
                destination -= 0x02000000;
                destinations[psec->get_index()] = (uint8_t *) text_data.get();

                if (destination + sectionSize > (uint32_t) text_data.get() + text_size) {
                    DEBUG_FUNCTION_LINE_ERR("Tried to overflow .text buffer. %08X > %08X", destination + sectionSize, (uint32_t) text_data.get() + text_size);
                    OSFatal("WUPSLoader: Tried to overflow buffer");
                }
            } else if ((address >= 0x10000000) && address < 0xC0000000) {
                destination += (uint32_t) data_data.get();
                destination -= 0x10000000;
                destinations[psec->get_index()] = (uint8_t *) data_data.get();

                if (destination + sectionSize > (uint32_t) data_data.get() + data_size) {
                    DEBUG_FUNCTION_LINE_ERR("Tried to overflow .data buffer. %08X > %08X", destination + sectionSize, (uint32_t) data_data.get() + data_size);
                    OSFatal("WUPSLoader: Tried to overflow buffer");
                }
            } else if (address >= 0xC0000000) {
                DEBUG_FUNCTION_LINE_ERR("Loading section from 0xC0000000 is NOT supported");
                return std::nullopt;
            } else {
                DEBUG_FUNCTION_LINE_ERR("Unhandled case");
                return std::nullopt;
            }

            const char *p = psec->get_data();

            if (psec->get_type() == SHT_NOBITS) {
                DEBUG_FUNCTION_LINE_VERBOSE("memset section %s %08X to 0 (%d bytes)", psec->get_name().c_str(), destination, sectionSize);
                memset((void *) destination, 0, sectionSize);
            } else if (psec->get_type() == SHT_PROGBITS) {
                DEBUG_FUNCTION_LINE_VERBOSE("Copy section %s %08X -> %08X (%d bytes)", psec->get_name().c_str(), p, destination, sectionSize);
                memcpy((void *) destination, p, sectionSize);
            }

            auto sectionInfo = make_unique_nothrow<SectionInfo>(psec->get_name(), destination, sectionSize);
            if (!sectionInfo) {
                DEBUG_FUNCTION_LINE_ERR("Failed to allocat SectionInfo");
                return {};
            }

            pluginInfo->addSectionInfo(std::move(sectionInfo));
            DEBUG_FUNCTION_LINE_VERBOSE("Saved %s section info. Location: %08X size: %08X", psec->get_name().c_str(), destination, sectionSize);

            totalSize += sectionSize;

            DCFlushRange((void *) destination, sectionSize);
            ICInvalidateRange((void *) destination, sectionSize);
        }
    }

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader.sections[i];
        if ((psec->get_type() == SHT_PROGBITS || psec->get_type() == SHT_NOBITS) && (psec->get_flags() & SHF_ALLOC)) {
            DEBUG_FUNCTION_LINE_VERBOSE("Linking (%d)... %s at %08X", i, psec->get_name().c_str(), destinations[psec->get_index()]);
            if (!linkSection(reader, psec->get_index(), (uint32_t) destinations[psec->get_index()], (uint32_t) text_data.get(), (uint32_t) data_data.get(), trampoline_data, trampoline_data_length,
                             trampolineId)) {
                DEBUG_FUNCTION_LINE_ERR("linkSection failed");
                return {};
            }
        }
    }

    if (!PluginInformationFactory::addImportRelocationData(pluginInfo, reader, destinations)) {
        DEBUG_FUNCTION_LINE_ERR("addImportRelocationData failed");
        return {};
    }

    DCFlushRange((void *) text_data.get(), text_size);
    ICInvalidateRange((void *) text_data.get(), text_size);
    DCFlushRange((void *) data_data.get(), data_size);
    ICInvalidateRange((void *) data_data.get(), data_size);

    pluginInfo->setTrampolineId(trampolineId);

    auto secInfo = pluginInfo->getSectionInfo(".wups.hooks");
    if (secInfo && secInfo.value()->getSize() > 0) {
        size_t entries_count = secInfo.value()->getSize() / sizeof(wups_loader_hook_t);
        auto *entries        = (wups_loader_hook_t *) secInfo.value()->getAddress();
        if (entries != nullptr) {
            for (size_t j = 0; j < entries_count; j++) {
                wups_loader_hook_t *hook = &entries[j];
                DEBUG_FUNCTION_LINE_VERBOSE("Saving hook of plugin Type: %08X, target: %08X", hook->type, (void *) hook->target);
                auto hookData = make_unique_nothrow<HookData>((void *) hook->target, hook->type);
                if (!hookData) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to allocate HookData");
                    return {};
                }
                pluginInfo->addHookData(std::move(hookData));
            }
        }
    }

    secInfo = pluginInfo->getSectionInfo(".wups.load");
    if (secInfo && secInfo.value()->getSize() > 0) {
        size_t entries_count = secInfo.value()->getSize() / sizeof(wups_loader_entry_t);
        auto *entries        = (wups_loader_entry_t *) secInfo.value()->getAddress();
        if (entries != nullptr) {
            for (size_t j = 0; j < entries_count; j++) {
                wups_loader_entry_t *cur_function = &entries[j];
                DEBUG_FUNCTION_LINE_VERBOSE("Saving function \"%s\" of plugin . PA:%08X VA:%08X Library: %08X, target: %08X, call_addr: %08X",
                                            cur_function->_function.name /*,pluginData->getPluginInformation()->getName().c_str()*/,
                                            cur_function->_function.physical_address, cur_function->_function.virtual_address, cur_function->_function.library, cur_function->_function.target,
                                            (void *) cur_function->_function.call_addr);

                auto functionData = make_unique_nothrow<FunctionData>((void *) cur_function->_function.physical_address,
                                                                      (void *) cur_function->_function.virtual_address,
                                                                      cur_function->_function.name,
                                                                      (function_replacement_library_type_t) cur_function->_function.library,
                                                                      (void *) cur_function->_function.target,
                                                                      (void *) cur_function->_function.call_addr,
                                                                      (FunctionPatcherTargetProcess) cur_function->_function.targetProcess);
                if (!functionData) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to allocate FunctionData");
                    return {};
                }
                pluginInfo->addFunctionData(std::move(functionData));
            }
        }
    }

    // Get the symbol for functions.
    Elf_Half n = reader.sections.size();
    for (Elf_Half i = 0; i < n; ++i) {
        section *sec = reader.sections[i];
        if (SHT_SYMTAB == sec->get_type()) {
            symbol_section_accessor symbols(reader, sec);
            auto sym_no = (uint32_t) symbols.get_symbols_num();
            if (sym_no > 0) {
                for (Elf_Half j = 0; j < sym_no; ++j) {
                    std::string name;
                    Elf64_Addr value    = 0;
                    Elf_Xword size      = 0;
                    unsigned char bind  = 0;
                    unsigned char type  = 0;
                    Elf_Half section    = 0;
                    unsigned char other = 0;
                    if (symbols.get_symbol(j, name, value, size, bind, type, section, other)) {

                        if (type == STT_FUNC) { // We only care about functions.
                            auto sectionVal = reader.sections[section];
                            auto offsetVal  = value - sectionVal->get_address();
                            auto sectionOpt = pluginInfo->getSectionInfo(sectionVal->get_name());
                            if (!sectionOpt.has_value()) {
                                continue;
                            }

                            auto finalAddress       = offsetVal + sectionOpt.value()->getAddress();
                            auto functionSymbolData = make_unique_nothrow<FunctionSymbolData>(name, (void *) finalAddress, (uint32_t) size);
                            if (!functionSymbolData) {
                                DEBUG_FUNCTION_LINE_ERR("Failed to allocate FunctionSymbolData");
                                return {};
                            }
                            pluginInfo->addFunctionSymbolData(std::move(functionSymbolData));
                        }
                    }
                }
                break;
            }
        }
    }

    if (totalSize > text_size + data_size) {
        DEBUG_FUNCTION_LINE_ERR("We didn't allocate enough memory!!");
        return std::nullopt;
    }

    // Save the addresses for the allocated memory. This way we can free it again :)
    pluginInfo->allocatedDataMemoryAddress = std::move(data_data);
    pluginInfo->allocatedTextMemoryAddress = std::move(text_data);

    return pluginInfo;
}

bool PluginInformationFactory::addImportRelocationData(const std::unique_ptr<PluginInformation> &pluginInfo, const elfio &reader, const std::unique_ptr<uint8_t *[]> &destinations) {
    std::map<uint32_t, std::shared_ptr<ImportRPLInformation>> infoMap;

    uint32_t sec_num = reader.sections.size();

    for (uint32_t i = 0; i < sec_num; ++i) {
        auto *psec = reader.sections[i];
        if (psec->get_type() == 0x80000002) {
            auto info = make_shared_nothrow<ImportRPLInformation>(psec->get_name());
            if (!info) {
                return false;
            }
            infoMap[i] = std::move(info);
        }
    }

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader.sections[i];
        if (psec->get_type() == SHT_RELA || psec->get_type() == SHT_REL) {
            DEBUG_FUNCTION_LINE_VERBOSE("Found relocation section %s", psec->get_name().c_str());
            relocation_section_accessor rel(reader, psec);
            for (uint32_t j = 0; j < (uint32_t) rel.get_entries_num(); ++j) {
                Elf_Word symbol = 0;
                Elf64_Addr offset;
                Elf_Word type;
                Elf_Sxword addend;
                std::string sym_name;
                Elf64_Addr sym_value;

                if (!rel.get_entry(j, offset, symbol, type, addend)) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to get relocation");
                    return false;
                }
                symbol_section_accessor symbols(reader, reader.sections[(Elf_Half) psec->get_link()]);

                // Find the symbol
                Elf_Xword size;
                unsigned char bind;
                unsigned char symbolType;
                Elf_Half sym_section_index;
                unsigned char other;

                if (!symbols.get_symbol(symbol, sym_name, sym_value, size,
                                        bind, symbolType, sym_section_index, other)) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to get symbol");
                    return false;
                }

                auto adjusted_sym_value = (uint32_t) sym_value;
                if (adjusted_sym_value < 0xC0000000) {
                    continue;
                }

                uint32_t section_index = psec->get_info();
                if (!infoMap.contains(sym_section_index)) {
                    DEBUG_FUNCTION_LINE_ERR("Relocation is referencing a unknown section. %d destination: %08X sym_name %s", section_index, destinations[section_index], sym_name.c_str());
                    return false;
                }

                auto relocationData = make_unique_nothrow<RelocationData>(type,
                                                                          offset - 0x02000000,
                                                                          addend,
                                                                          (void *) (destinations[section_index]),
                                                                          sym_name,
                                                                          infoMap[sym_section_index]);
                if (!relocationData) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to allocate RelocationData");
                    return false;
                }

                pluginInfo->addRelocationData(std::move(relocationData));
            }
        }
    }
    return true;
}

bool PluginInformationFactory::linkSection(const elfio &reader, uint32_t section_index, uint32_t destination, uint32_t base_text, uint32_t base_data, relocation_trampoline_entry_t *trampoline_data,
                                           uint32_t trampoline_data_length,
                                           uint8_t trampolineId) {
    uint32_t sec_num = reader.sections.size();

    for (uint32_t i = 0; i < sec_num; ++i) {
        section *psec = reader.sections[i];
        if (psec->get_info() == section_index) {
            DEBUG_FUNCTION_LINE_VERBOSE("Found relocation section %s", psec->get_name().c_str());
            relocation_section_accessor rel(reader, psec);
            for (uint32_t j = 0; j < (uint32_t) rel.get_entries_num(); ++j) {
                Elf_Word symbol = 0;
                Elf64_Addr offset;
                Elf_Word type;
                Elf_Sxword addend;
                std::string sym_name;
                Elf64_Addr sym_value;

                if (!rel.get_entry(j, offset, symbol, type, addend)) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to get relocation");
                    return false;
                }
                symbol_section_accessor symbols(reader, reader.sections[(Elf_Half) psec->get_link()]);

                // Find the symbol
                Elf_Xword size;
                unsigned char bind;
                unsigned char symbolType;
                Elf_Half sym_section_index;
                unsigned char other;

                if (!symbols.get_symbol(symbol, sym_name, sym_value, size,
                                        bind, symbolType, sym_section_index, other)) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to get symbol");
                    return false;
                }

                auto adjusted_sym_value = (uint32_t) sym_value;
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
                    DEBUG_FUNCTION_LINE_ERR("Unhandled case %08X", adjusted_sym_value);
                    return false;
                }

                auto adjusted_offset = (uint32_t) offset;
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
                    DEBUG_FUNCTION_LINE_ERR("NOT IMPLEMENTED: %04X", sym_section_index);
                    return false;
                }
                // DEBUG_FUNCTION_LINE_VERBOSE("sym_value %08X adjusted_sym_value %08X offset %08X adjusted_offset %08X", (uint32_t) sym_value, adjusted_sym_value, (uint32_t) offset, adjusted_offset);

                if (!ElfUtils::elfLinkOne(type, adjusted_offset, addend, destination, adjusted_sym_value, trampoline_data, trampoline_data_length, RELOC_TYPE_FIXED, trampolineId)) {
                    DEBUG_FUNCTION_LINE_ERR("Link failed");
                    return false;
                }
            }
            DEBUG_FUNCTION_LINE_VERBOSE("done");
            return true;
        }
    }
    return true;
}
