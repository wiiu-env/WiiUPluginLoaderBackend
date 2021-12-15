#include "PluginInformation.h"

PluginInformation::PluginInformation(const PluginInformation &other) {
    for (const auto &i: other.hook_data_list) {
        hook_data_list.push_back(i);
    }
    for (const auto &i: other.function_data_list) {
        function_data_list.push_back(i);
    }
    for (const auto &i: other.relocation_data_list) {
        relocation_data_list.push_back(i);
    }
    for (const auto &i: other.symbol_data_list) {
        symbol_data_list.insert(i);
    }
    section_info_list = other.section_info_list;
    trampolinId = other.trampolinId;
    allocatedTextMemoryAddress = other.allocatedTextMemoryAddress;
    allocatedDataMemoryAddress = other.allocatedDataMemoryAddress;
}