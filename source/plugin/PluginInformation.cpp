#include "PluginInformation.h"

PluginInformation::PluginInformation(const PluginInformation &other) {
    for (size_t i = 0; i < other.hook_data_list.size(); i++) {
        hook_data_list.push_back(other.hook_data_list[i]);
    }
    for (size_t i = 0; i < other.function_data_list.size(); i++) {
        function_data_list.push_back(other.function_data_list[i]);
    }
    for (size_t i = 0; i < other.relocation_data_list.size(); i++) {
        relocation_data_list.push_back(other.relocation_data_list[i]);
    }
    section_info_list = other.section_info_list;
    trampolinId = other.trampolinId;
    allocatedTextMemoryAddress = other.allocatedTextMemoryAddress;
    allocatedDataMemoryAddress = other.allocatedDataMemoryAddress;
}