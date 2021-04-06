#include <coreinit/cache.h>

#include "PluginContainer.h"
#include "PluginInformationFactory.h"
#include "PluginMetaInformationFactory.h"
#include "PluginContainerPersistence.h"
#include "PluginDataPersistence.h"
#include "DynamicLinkingHelper.h"
#include "../common/plugin_defines.h"
#include "PluginInformation.h"
#include "RelocationData.h"

bool PluginContainerPersistence::savePlugin(plugin_information_t *pluginInformation, PluginContainer &plugin) {
    int32_t plugin_count = pluginInformation->number_used_plugins;

    auto pluginName = plugin.getMetaInformation().getName();

    if (plugin_count >= MAXIMUM_PLUGINS - 1) {
        DEBUG_FUNCTION_LINE("Maximum of %d plugins reached. %s won't be loaded!", MAXIMUM_PLUGINS, pluginName.c_str());
        return false;
    }
    // Copy data to global struct.
    plugin_information_single_t *plugin_data = &(pluginInformation->plugin_data[plugin_count]);

    // Make sure everything is reset.
    memset((void *) plugin_data, 0, sizeof(plugin_information_single_t));

    const auto &pluginMetaInfo = plugin.getMetaInformation();
    auto plugin_meta_data = &plugin_data->meta;

    if (pluginMetaInfo.getName().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE("Warning: name will be truncated.");
    }
    strncpy(plugin_meta_data->name, pluginMetaInfo.getName().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);
    if (pluginMetaInfo.getAuthor().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE("Warning: author will be truncated.");
    }
    strncpy(plugin_meta_data->author, pluginMetaInfo.getAuthor().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);

    if (pluginMetaInfo.getVersion().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE("Warning: version will be truncated.");
    }
    strncpy(plugin_meta_data->version, pluginMetaInfo.getVersion().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);

    if (pluginMetaInfo.getLicense().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE("Warning: license will be truncated.");
    }
    strncpy(plugin_meta_data->license, pluginMetaInfo.getLicense().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);

    if (pluginMetaInfo.getBuildTimestamp().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE("Warning: build timestamp will be truncated.");
    }
    strncpy(plugin_meta_data->buildTimestamp, pluginMetaInfo.getBuildTimestamp().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);

    if (pluginMetaInfo.getDescription().size() >= MAXIMUM_PLUGIN_DESCRIPTION_LENGTH) {
        DEBUG_FUNCTION_LINE("Warning: description will be truncated.");
        DEBUG_FUNCTION_LINE("%s", pluginMetaInfo.getDescription().c_str());
    }
    strncpy(plugin_meta_data->descripion, pluginMetaInfo.getDescription().c_str(), MAXIMUM_PLUGIN_DESCRIPTION_LENGTH - 1);

    if (pluginMetaInfo.getId().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE("Warning: plugin id will be truncated.");
    }
    strncpy(plugin_meta_data->id, pluginMetaInfo.getId().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);

    plugin_meta_data->size = pluginMetaInfo.getSize();

    auto pluginInfo = plugin.getPluginInformation();

    // Relocation
    std::vector<RelocationData> relocationData = pluginInfo.getRelocationDataList();
    for (auto &reloc : relocationData) {
        if (!DynamicLinkingHelper::addReloationEntry(&(pluginInformation->linking_data), plugin_data->info.linking_entries, PLUGIN_DYN_LINK_RELOCATION_LIST_LENGTH, reloc)) {
            DEBUG_FUNCTION_LINE("Failed to add a relocation entry");
            return false;
        }
    }

    std::vector<FunctionData> function_data_list = pluginInfo.getFunctionDataList();
    std::vector<HookData> hook_data_list = pluginInfo.getHookDataList();

    if (function_data_list.size() > MAXIMUM_FUNCTION_PER_PLUGIN) {
        DEBUG_FUNCTION_LINE("Plugin %s would replace to many function (%d, maximum is %d). It won't be loaded.", pluginName.c_str(), function_data_list.size(), MAXIMUM_FUNCTION_PER_PLUGIN);
        return false;
    }
    if (hook_data_list.size() > MAXIMUM_HOOKS_PER_PLUGIN) {
        DEBUG_FUNCTION_LINE("Plugin %s would set too many hooks (%d, maximum is %d). It won't be loaded.", pluginName.c_str(), hook_data_list.size(), MAXIMUM_HOOKS_PER_PLUGIN);
        return false;
    }

    if (pluginName.length() > MAXIMUM_PLUGIN_NAME_LENGTH - 1) {
        DEBUG_FUNCTION_LINE("Name for plugin %s was too long to be stored.", pluginName.c_str());
        return false;
    }

    /* Store function replacement information */
    uint32_t i = 0;
    for (auto &curFunction : pluginInfo.getFunctionDataList()) {
        function_replacement_data_t *function_data = &plugin_data->info.functions[i];
        if (strlen(curFunction.getName().c_str()) > MAXIMUM_FUNCTION_NAME_LENGTH - 1) {
            DEBUG_FUNCTION_LINE("Could not add function \"%s\" for plugin \"%s\" function name is too long.", curFunction.getName().c_str(), pluginName.c_str());
            continue;
        }

        DEBUG_FUNCTION_LINE_VERBOSE("Adding function \"%s\" for plugin \"%s\"", curFunction.getName().c_str(), pluginName.c_str());

        strncpy(function_data->function_name, curFunction.getName().c_str(), MAXIMUM_FUNCTION_NAME_LENGTH - 1);

        function_data->VERSION = FUNCTION_REPLACEMENT_DATA_STRUCT_VERSION;
        function_data->library = (function_replacement_library_type_t) curFunction.getLibrary();
        function_data->replaceAddr = (uint32_t) curFunction.getReplaceAddress();
        function_data->replaceCall = (uint32_t) curFunction.getReplaceCall();
        function_data->physicalAddr = (uint32_t) curFunction.getPhysicalAddress();
        function_data->virtualAddr = (uint32_t) curFunction.getVirtualAddress();
        function_data->targetProcess = curFunction.getTargetProcess();

        plugin_data->info.number_used_functions++;
        i++;
    }

    i = 0;
    for (auto &curHook : pluginInfo.getHookDataList()) {
        replacement_data_hook_t *hook_data = &plugin_data->info.hooks[i];

        DEBUG_FUNCTION_LINE_VERBOSE("Set hook for plugin \"%s\" of type %08X to target %08X", plugin_data->meta.name, curHook.getType(), (void *) curHook.getFunctionPointer());

        hook_data->func_pointer = (void *) curHook.getFunctionPointer();
        hook_data->type = curHook.getType();

        plugin_data->info.number_used_hooks++;
        i++;
    }

    /* Saving SectionInfos */
    for (auto &curSection  : pluginInfo.getSectionInfoList()) {
        bool foundFreeSlot = false;
        uint32_t slot = 0;
        for (uint32_t i = 0; i < MAXIMUM_PLUGIN_SECTION_LENGTH; i++) {
            plugin_section_info_t *sectionInfo = &(plugin_data->info.sectionInfos[i]);
            if (sectionInfo->addr == 0 && sectionInfo->size == 0) {
                foundFreeSlot = true;
                slot = i;
                break;
            }
        }
        if (foundFreeSlot) {
            plugin_section_info_t *sectionInfo = &(plugin_data->info.sectionInfos[slot]);
            if (strlen(curSection.first.c_str()) > MAXIMUM_PLUGIN_SECTION_NAME_LENGTH - 1) {
                DEBUG_FUNCTION_LINE("Could not add section info \"%s\" for plugin \"%s\" section name is too long.", curSection.first.c_str(), pluginName.c_str());
                break;
            }
            strncpy(sectionInfo->name, curSection.first.c_str(), MAXIMUM_PLUGIN_SECTION_NAME_LENGTH - 1);
            sectionInfo->addr = curSection.second.getAddress();
            sectionInfo->size = curSection.second.getSize();

        } else {
            DEBUG_FUNCTION_LINE("Failed to store SectionInfos");
            return false;
        }
    }
    plugin_data->info.trampolinId = pluginInfo.getTrampolinId();
    plugin_data->info.allocatedTextMemoryAddress = pluginInfo.allocatedTextMemoryAddress;
    plugin_data->info.allocatedDataMemoryAddress = pluginInfo.allocatedDataMemoryAddress;


    /* Copy plugin data */
    auto pluginData = plugin.getPluginData();
    auto plugin_data_data = &plugin_data->data;

    PluginDataPersistence::save(plugin_data_data, pluginData);

    pluginInformation->number_used_plugins++;

    DCFlushRange((void *) pluginInformation, sizeof(plugin_information_t));
    ICInvalidateRange((void *) pluginInformation, sizeof(plugin_information_t));

    return true;
}

std::vector<PluginContainer> PluginContainerPersistence::loadPlugins(plugin_information_t *pluginInformation) {
    std::vector<PluginContainer> result;
    if (pluginInformation == nullptr) {
        DEBUG_FUNCTION_LINE("pluginInformation == NULL");
        return result;
    }
    DCFlushRange((void *) pluginInformation, sizeof(plugin_information_t));
    ICInvalidateRange((void *) pluginInformation, sizeof(plugin_information_t));

    int32_t plugin_count = pluginInformation->number_used_plugins;
    if (plugin_count > MAXIMUM_PLUGINS) {
        DEBUG_FUNCTION_LINE("pluginInformation->plugin_count was bigger then allowed. %d > %d. Limiting to %d", plugin_count, MAXIMUM_PLUGINS, MAXIMUM_PLUGINS);
        plugin_count = MAXIMUM_PLUGINS;
    }
    for (int32_t i = 0; i < plugin_count; i++) {
        // Copy data from struct.
        plugin_information_single_t *plugin_data = &(pluginInformation->plugin_data[i]);

        PluginMetaInformation metaInformation;

        plugin_meta_info_t *meta = &(plugin_data->meta);
        metaInformation.setAuthor(meta->author);
        metaInformation.setVersion(meta->version);
        metaInformation.setBuildTimestamp(meta->buildTimestamp);
        metaInformation.setLicense(meta->license);
        metaInformation.setDescription(meta->descripion);
        metaInformation.setSize(meta->size);
        metaInformation.setName(meta->name);

        plugin_data_t *data = &(plugin_data->data);

        PluginData pluginData = PluginDataPersistence::load(data);

        PluginInformation curPluginInformation;

        curPluginInformation.setTrampolinId(plugin_data->info.trampolinId);
        curPluginInformation.allocatedTextMemoryAddress = plugin_data->info.allocatedTextMemoryAddress;
        curPluginInformation.allocatedDataMemoryAddress = plugin_data->info.allocatedDataMemoryAddress;

        for (auto & curItem : plugin_data->info.sectionInfos) {
            plugin_section_info_t *sectionInfo = &curItem;
            if (sectionInfo->addr == 0 && sectionInfo->size == 0) {
                continue;
            }
            DEBUG_FUNCTION_LINE_VERBOSE("Add SectionInfo %s", sectionInfo->name);
            std::string name(sectionInfo->name);
            curPluginInformation.addSectionInfo(SectionInfo(name, sectionInfo->addr, sectionInfo->size));
        }

        /* load hook data */
        uint32_t hookCount = plugin_data->info.number_used_hooks;

        if (hookCount > MAXIMUM_HOOKS_PER_PLUGIN) {
            DEBUG_FUNCTION_LINE("number_used_hooks was bigger then allowed. %d > %d. Limiting to %d", hookCount, MAXIMUM_HOOKS_PER_PLUGIN, MAXIMUM_HOOKS_PER_PLUGIN);
            hookCount = MAXIMUM_HOOKS_PER_PLUGIN;
        }

        for (uint32_t j = 0; j < hookCount; j++) {
            replacement_data_hook_t *hook_entry = &(plugin_data->info.hooks[j]);
            HookData curHook(hook_entry->func_pointer, hook_entry->type);
            curPluginInformation.addHookData(curHook);
        }

        /* load function replacement data */
        uint32_t functionReplaceCount = plugin_data->info.number_used_functions;

        if (functionReplaceCount > MAXIMUM_FUNCTION_PER_PLUGIN) {
            DEBUG_FUNCTION_LINE("number_used_functions was bigger then allowed. %d > %d. Limiting to %d", functionReplaceCount, MAXIMUM_FUNCTION_PER_PLUGIN, MAXIMUM_FUNCTION_PER_PLUGIN);
            functionReplaceCount = MAXIMUM_FUNCTION_PER_PLUGIN;
        }

        for (uint32_t j = 0; j < functionReplaceCount; j++) {
            function_replacement_data_t *entry = &(plugin_data->info.functions[j]);
            FunctionData func((void *) entry->physicalAddr, (void *) entry->virtualAddr, entry->function_name, (function_replacement_library_type_t) entry->library, (void *) entry->replaceAddr, (void *) entry->replaceCall, entry->targetProcess);
            curPluginInformation.addFunctionData(func);
        }

        /* load relocation data */
        for (auto & linking_entrie : plugin_data->info.linking_entries) {
            dyn_linking_relocation_entry_t *linking_entry = &linking_entrie;
            if (linking_entry->destination == nullptr) {
                break;
            }
            dyn_linking_import_t *importEntry = linking_entry->importEntry;
            if (importEntry == nullptr) {
                DEBUG_FUNCTION_LINE("importEntry was NULL, skipping relocation entry");
                continue;
            }
            if (importEntry->importName == nullptr) {
                DEBUG_FUNCTION_LINE("importEntry->importName was NULL, skipping relocation entry");
                continue;
            }
            dyn_linking_function_t *functionEntry = linking_entry->functionEntry;

            if (functionEntry == nullptr) {
                DEBUG_FUNCTION_LINE("functionEntry was NULL, skipping relocation entry");
                continue;
            }
            if (functionEntry->functionName == nullptr) {
                DEBUG_FUNCTION_LINE("functionEntry->functionName was NULL, skipping relocation entry");
                continue;
            }
            ImportRPLInformation rplInfo(importEntry->importName, importEntry->isData);
            std::string functionName(functionEntry->functionName);
            RelocationData reloc(linking_entry->type, linking_entry->offset, linking_entry->addend, linking_entry->destination, functionName, rplInfo);
            curPluginInformation.addRelocationData(reloc);
        }

        PluginContainer container;
        container.setMetaInformation(metaInformation);
        container.setPluginData(pluginData);
        container.setPluginInformation(curPluginInformation);
        result.push_back(container);
    }
    return result;
}
