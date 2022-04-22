#include <coreinit/cache.h>

#include <memory>

#include "DynamicLinkingHelper.h"
#include "PluginContainer.h"
#include "PluginContainerPersistence.h"
#include "PluginDataPersistence.h"
#include "PluginInformationFactory.h"
#include "PluginMetaInformationFactory.h"

bool PluginContainerPersistence::savePlugin(plugin_information_t *pluginInformation, const std::shared_ptr<PluginContainer> &plugin, MEMHeapHandle heapHandle) {
    int32_t plugin_count = pluginInformation->number_used_plugins;

    auto pluginName = plugin->getMetaInformation()->getName();

    if (plugin_count >= MAXIMUM_PLUGINS - 1) {
        DEBUG_FUNCTION_LINE_ERR("Maximum of %d plugins reached. %s won't be loaded!", MAXIMUM_PLUGINS, pluginName.c_str());
        return false;
    }
    // Copy data to global struct.
    plugin_information_single_t *plugin_data = &(pluginInformation->plugin_data[plugin_count]);

    // Make sure everything is reset.
    memset((void *) plugin_data, 0, sizeof(plugin_information_single_t));

    const auto &pluginMetaInfo = plugin->getMetaInformation();
    auto plugin_meta_data      = &plugin_data->meta;

    if (pluginMetaInfo->getName().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE_ERR("Warning: name will be truncated.");
    }
    strncpy(plugin_meta_data->name, pluginMetaInfo->getName().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);
    if (pluginMetaInfo->getAuthor().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE_ERR("Warning: author will be truncated.");
    }
    strncpy(plugin_meta_data->author, pluginMetaInfo->getAuthor().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);

    if (pluginMetaInfo->getVersion().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE_ERR("Warning: version will be truncated.");
    }
    strncpy(plugin_meta_data->version, pluginMetaInfo->getVersion().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);

    if (pluginMetaInfo->getLicense().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE_ERR("Warning: license will be truncated.");
    }
    strncpy(plugin_meta_data->license, pluginMetaInfo->getLicense().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);

    if (pluginMetaInfo->getBuildTimestamp().size() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE_ERR("Warning: build timestamp will be truncated.");
    }
    strncpy(plugin_meta_data->buildTimestamp, pluginMetaInfo->getBuildTimestamp().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);

    if (pluginMetaInfo->getDescription().size() >= MAXIMUM_PLUGIN_DESCRIPTION_LENGTH) {
        DEBUG_FUNCTION_LINE_ERR("Warning: description will be truncated.");
        DEBUG_FUNCTION_LINE_ERR("%s", pluginMetaInfo->getDescription().c_str());
    }
    strncpy(plugin_meta_data->descripion, pluginMetaInfo->getDescription().c_str(), MAXIMUM_PLUGIN_DESCRIPTION_LENGTH - 1);

    if (pluginMetaInfo->getStorageId().length() >= MAXIMUM_PLUGIN_META_FIELD_LENGTH) {
        DEBUG_FUNCTION_LINE_ERR("Warning: plugin storage id will be truncated.");
    }
    strncpy(plugin_meta_data->storageId, pluginMetaInfo->getStorageId().c_str(), MAXIMUM_PLUGIN_META_FIELD_LENGTH - 1);

    plugin_meta_data->size = pluginMetaInfo->getSize();

    auto pluginInfo = plugin->getPluginInformation();

    // Relocation
    auto relocationData = pluginInfo->getRelocationDataList();
    for (auto &reloc : relocationData) {
        if (!DynamicLinkingHelper::addReloationEntry(&(pluginInformation->linking_data), plugin_data->info.linking_entries, PLUGIN_DYN_LINK_RELOCATION_LIST_LENGTH, reloc)) {
            DEBUG_FUNCTION_LINE_ERR("Failed to add a relocation entry");
            return false;
        }
    }

    auto function_data_list = pluginInfo->getFunctionDataList();
    auto hook_data_list     = pluginInfo->getHookDataList();

    if (function_data_list.size() > MAXIMUM_FUNCTION_PER_PLUGIN) {
        DEBUG_FUNCTION_LINE_ERR("Plugin %s would replace to many function (%d, maximum is %d). It won't be loaded.", pluginName.c_str(), function_data_list.size(), MAXIMUM_FUNCTION_PER_PLUGIN);
        return false;
    }
    if (hook_data_list.size() > MAXIMUM_HOOKS_PER_PLUGIN) {
        DEBUG_FUNCTION_LINE_ERR("Plugin %s would set too many hooks (%d, maximum is %d). It won't be loaded.", pluginName.c_str(), hook_data_list.size(), MAXIMUM_HOOKS_PER_PLUGIN);
        return false;
    }

    if (pluginName.length() > MAXIMUM_PLUGIN_NAME_LENGTH - 1) {
        DEBUG_FUNCTION_LINE_ERR("Name for plugin %s was too long to be stored.", pluginName.c_str());
        return false;
    }

    /* Store function replacement information */
    uint32_t i = 0;
    for (auto &curFunction : pluginInfo->getFunctionDataList()) {
        function_replacement_data_t *function_data = &plugin_data->info.functions[i];
        if (strlen(curFunction->getName().c_str()) > MAXIMUM_FUNCTION_NAME_LENGTH - 1) {
            DEBUG_FUNCTION_LINE_ERR("Could not add function \"%s\" for plugin \"%s\" function name is too long.", curFunction->getName().c_str(), pluginName.c_str());
            continue;
        }

        DEBUG_FUNCTION_LINE_VERBOSE("Adding function \"%s\" for plugin \"%s\"", curFunction->getName().c_str(), pluginName.c_str());

        strncpy(function_data->function_name, curFunction->getName().c_str(), MAXIMUM_FUNCTION_NAME_LENGTH - 1);

        function_data->VERSION       = FUNCTION_REPLACEMENT_DATA_STRUCT_VERSION;
        function_data->library       = (function_replacement_library_type_t) curFunction->getLibrary();
        function_data->replaceAddr   = (uint32_t) curFunction->getReplaceAddress();
        function_data->replaceCall   = (uint32_t) curFunction->getReplaceCall();
        function_data->physicalAddr  = (uint32_t) curFunction->getPhysicalAddress();
        function_data->virtualAddr   = (uint32_t) curFunction->getVirtualAddress();
        function_data->targetProcess = curFunction->getTargetProcess();

        plugin_data->info.number_used_functions++;
        i++;
    }

    i = 0;
    for (auto &curHook : pluginInfo->getHookDataList()) {
        replacement_data_hook_t *hook_data = &plugin_data->info.hooks[i];

        DEBUG_FUNCTION_LINE_VERBOSE("Set hook for plugin \"%s\" of type %08X to target %08X", plugin_data->meta.name, curHook->getType(), (void *) curHook->getFunctionPointer());

        hook_data->func_pointer = (void *) curHook->getFunctionPointer();
        hook_data->type         = curHook->getType();

        plugin_data->info.number_used_hooks++;
        i++;
    }

    /* Saving SectionInfos */
    for (auto &curSection : pluginInfo->getSectionInfoList()) {
        bool foundFreeSlot = false;
        uint32_t slot      = 0;
        for (uint32_t j = 0; j < MAXIMUM_PLUGIN_SECTION_LENGTH; j++) {
            auto *sectionInfo = &(plugin_data->info.sectionInfos[j]);
            if (sectionInfo->addr == 0 && sectionInfo->size == 0) {
                foundFreeSlot = true;
                slot          = j;
                break;
            }
        }
        if (foundFreeSlot) {
            auto *sectionInfo = &(plugin_data->info.sectionInfos[slot]);
            if (curSection.first.length() > MAXIMUM_PLUGIN_SECTION_NAME_LENGTH - 1) {
                DEBUG_FUNCTION_LINE_ERR("Could not add section info \"%s\" for plugin \"%s\" section name is too long.", curSection.first.c_str(), pluginName.c_str());
                break;
            }
            strncpy(sectionInfo->name, curSection.first.c_str(), MAXIMUM_PLUGIN_SECTION_NAME_LENGTH - 1);
            sectionInfo->addr = curSection.second->getAddress();
            sectionInfo->size = curSection.second->getSize();

        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to store SectionInfos");
            return false;
        }
    }
    plugin_data->info.trampolineId               = pluginInfo->getTrampolineId();
    plugin_data->info.allocatedTextMemoryAddress = pluginInfo->allocatedTextMemoryAddress;
    plugin_data->info.allocatedDataMemoryAddress = pluginInfo->allocatedDataMemoryAddress;

    uint32_t entryCount = pluginInfo->getFunctionSymbolDataList().size();
    if (entryCount > 0) {
        // Saving SectionInfos
        uint32_t funcSymStringLen = 1;
        for (auto &curFuncSym : pluginInfo->getFunctionSymbolDataList()) {
            funcSymStringLen += curFuncSym->getName().length() + 1;
        }

        char *stringTable = (char *) MEMAllocFromExpHeapEx(heapHandle, funcSymStringLen, 0x4);
        if (stringTable == nullptr) {
            DEBUG_FUNCTION_LINE_ERR("Failed alloc memory to store string table for function symbol data");
            return false;
        }
        memset(stringTable, 0, funcSymStringLen);
        DEBUG_FUNCTION_LINE("Allocated %d for the function symbol string table", funcSymStringLen);
        auto *entryTable = (plugin_function_symbol_data_t *) MEMAllocFromExpHeapEx(heapHandle, entryCount * sizeof(plugin_function_symbol_data_t), 0x4);
        if (entryTable == nullptr) {
            MEMFreeToExpHeap((MEMHeapHandle) heapHandle, stringTable);
            free(stringTable);
            DEBUG_FUNCTION_LINE_ERR("Failed alloc memory to store function symbol data");
            return false;
        }
        DEBUG_FUNCTION_LINE("Allocated %d for the function symbol data", entryCount * sizeof(plugin_function_symbol_data_t));

        uint32_t curStringOffset = 0;
        uint32_t curEntryIndex   = 0;
        for (auto &curFuncSym : pluginInfo->getFunctionSymbolDataList()) {
            entryTable[curEntryIndex].address = curFuncSym->getAddress();
            entryTable[curEntryIndex].name    = &stringTable[curStringOffset];
            entryTable[curEntryIndex].size    = curFuncSym->getSize();
            auto len                          = curFuncSym->getName().length() + 1;
            memcpy(stringTable + curStringOffset, curFuncSym->getName().c_str(), len);
            curStringOffset += len;
            curEntryIndex++;
        }

        plugin_data->info.allocatedFuncSymStringTableAddress = stringTable;
        plugin_data->info.function_symbol_data               = entryTable;
    }

    plugin_data->info.number_function_symbol_data = entryCount;

    /* Copy plugin data */
    auto pluginData       = plugin->getPluginData();
    auto plugin_data_data = &plugin_data->data;

    PluginDataPersistence::save(plugin_data_data, pluginData);

    pluginInformation->number_used_plugins++;

    DCFlushRange((void *) pluginInformation, sizeof(plugin_information_t));
    ICInvalidateRange((void *) pluginInformation, sizeof(plugin_information_t));

    return true;
}

std::vector<std::shared_ptr<PluginContainer>> PluginContainerPersistence::loadPlugins(plugin_information_t *pluginInformation) {
    std::vector<std::shared_ptr<PluginContainer>> result;
    if (pluginInformation == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("pluginInformation == nullptr");
        return result;
    }
    DCFlushRange((void *) pluginInformation, sizeof(plugin_information_t));
    ICInvalidateRange((void *) pluginInformation, sizeof(plugin_information_t));

    int32_t plugin_count = pluginInformation->number_used_plugins;
    if (plugin_count > MAXIMUM_PLUGINS) {
        DEBUG_FUNCTION_LINE_ERR("pluginInformation->plugin_count was bigger then allowed. %d > %d. Limiting to %d", plugin_count, MAXIMUM_PLUGINS, MAXIMUM_PLUGINS);
        plugin_count = MAXIMUM_PLUGINS;
    }
    for (int32_t i = 0; i < plugin_count; i++) {
        // Copy data from struct.
        plugin_information_single_t *plugin_data = &(pluginInformation->plugin_data[i]);

        auto metaInformation = std::shared_ptr<PluginMetaInformation>(new PluginMetaInformation);

        plugin_meta_info_t *meta = &(plugin_data->meta);
        metaInformation->setAuthor(meta->author);
        metaInformation->setVersion(meta->version);
        metaInformation->setBuildTimestamp(meta->buildTimestamp);
        metaInformation->setLicense(meta->license);
        metaInformation->setDescription(meta->descripion);
        metaInformation->setSize(meta->size);
        metaInformation->setName(meta->name);
        metaInformation->setStorageId(meta->storageId);

        plugin_data_t *data = &(plugin_data->data);

        auto pluginData = PluginDataPersistence::load(data);

        auto curPluginInformation = std::make_shared<PluginInformation>();

        curPluginInformation->setTrampolineId(plugin_data->info.trampolineId);
        curPluginInformation->allocatedTextMemoryAddress = plugin_data->info.allocatedTextMemoryAddress;
        curPluginInformation->allocatedDataMemoryAddress = plugin_data->info.allocatedDataMemoryAddress;

        for (auto &curItem : plugin_data->info.sectionInfos) {
            plugin_section_info_t *sectionInfo = &curItem;
            if (sectionInfo->addr == 0 && sectionInfo->size == 0) {
                continue;
            }
            DEBUG_FUNCTION_LINE_VERBOSE("Add SectionInfo %s", sectionInfo->name);
            std::string name(sectionInfo->name);
            curPluginInformation->addSectionInfo(std::make_shared<SectionInfo>(name, sectionInfo->addr, sectionInfo->size));
        }

        /* load hook data */
        uint32_t hookCount = plugin_data->info.number_used_hooks;

        if (hookCount > MAXIMUM_HOOKS_PER_PLUGIN) {
            DEBUG_FUNCTION_LINE_ERR("number_used_hooks was bigger then allowed. %d > %d. Limiting to %d", hookCount, MAXIMUM_HOOKS_PER_PLUGIN, MAXIMUM_HOOKS_PER_PLUGIN);
            hookCount = MAXIMUM_HOOKS_PER_PLUGIN;
        }

        for (uint32_t j = 0; j < hookCount; j++) {
            replacement_data_hook_t *hook_entry = &(plugin_data->info.hooks[j]);
            curPluginInformation->addHookData(std::make_shared<HookData>(hook_entry->func_pointer, hook_entry->type));
        }

        bool storageHasId = true;
        for (auto const &value : curPluginInformation->getHookDataList()) {
            if (value->getType() == WUPS_LOADER_HOOK_INIT_STORAGE &&
                metaInformation->getStorageId().empty()) {
                storageHasId = false;
            }
        }
        if (!storageHasId) {
            DEBUG_FUNCTION_LINE_ERR("Plugin is using the storage API but has not set an ID");
            continue;
        }

        /* load function replacement data */
        uint32_t functionReplaceCount = plugin_data->info.number_used_functions;

        if (functionReplaceCount > MAXIMUM_FUNCTION_PER_PLUGIN) {
            DEBUG_FUNCTION_LINE_ERR("number_used_functions was bigger then allowed. %d > %d. Limiting to %d", functionReplaceCount, MAXIMUM_FUNCTION_PER_PLUGIN, MAXIMUM_FUNCTION_PER_PLUGIN);
            functionReplaceCount = MAXIMUM_FUNCTION_PER_PLUGIN;
        }

        for (uint32_t j = 0; j < functionReplaceCount; j++) {
            function_replacement_data_t *entry = &(plugin_data->info.functions[j]);
            auto func                          = std::make_shared<FunctionData>((void *) entry->physicalAddr, (void *) entry->virtualAddr, entry->function_name, (function_replacement_library_type_t) entry->library,
                                                       (void *) entry->replaceAddr,
                                                       (void *) entry->replaceCall, entry->targetProcess);
            curPluginInformation->addFunctionData(func);
        }

        /* load relocation data */
        for (auto &linking_entry : plugin_data->info.linking_entries) {
            if (linking_entry.destination == nullptr) {
                break;
            }
            dyn_linking_import_t *importEntry = linking_entry.importEntry;
            if (importEntry == nullptr) {
                DEBUG_FUNCTION_LINE_ERR("importEntry was nullptr, skipping relocation entry");
                continue;
            }
            dyn_linking_function_t *functionEntry = linking_entry.functionEntry;

            if (functionEntry == nullptr) {
                DEBUG_FUNCTION_LINE_ERR("functionEntry was nullptr, skipping relocation entry");
                continue;
            }
            auto rplInfo = std::make_shared<ImportRPLInformation>(importEntry->importName, importEntry->isData);
            std::string functionName(functionEntry->functionName);
            auto reloc = std::make_shared<RelocationData>(linking_entry.type, linking_entry.offset, linking_entry.addend, linking_entry.destination, functionName, rplInfo);
            curPluginInformation->addRelocationData(reloc);
        }

        /* load function symbol data */
        for (uint32_t j = 0; j < plugin_data->info.number_function_symbol_data; j++) {
            auto symbol_data        = &plugin_data->info.function_symbol_data[j];
            std::string symbol_name = symbol_data->name;
            auto funSymbolData      = std::make_shared<FunctionSymbolData>(symbol_name, (void *) symbol_data->address, symbol_data->size);
            curPluginInformation->addFunctionSymbolData(funSymbolData);
        }

        auto container = std::make_shared<PluginContainer>();
        container->setMetaInformation(metaInformation);
        container->setPluginData(pluginData);
        container->setPluginInformation(curPluginInformation);
        result.push_back(container);
    }
    return result;
}
