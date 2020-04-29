#include "DynamicLinkingHelper.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <coreinit/dynload.h>
#include "utils/logger.h"
#include "common/plugin_defines.h"

dyn_linking_function_t * DynamicLinkingHelper::getOrAddFunctionEntryByName(dyn_linking_relocation_data_t * data, const char* functionName) {
    if(data == NULL) {
        return NULL;
    }
    if(functionName == NULL) {
        return NULL;
    }
    dyn_linking_function_t * result = NULL;
    for(int32_t i = 0; i < DYN_LINK_FUNCTION_LIST_LENGTH; i++) {
        dyn_linking_function_t * curEntry = &(data->functions[i]);
        if(strlen(curEntry->functionName) == 0) {
            if(strlen(functionName) > DYN_LINK_FUNCTION_NAME_LENGTH) {
                DEBUG_FUNCTION_LINE("Failed to add function name, it's too long.\n");
                return NULL;
            }
            strncpy(curEntry->functionName,functionName,DYN_LINK_FUNCTION_NAME_LENGTH);
            result = curEntry;
            break;
        }
        if(strncmp(curEntry->functionName,functionName,DYN_LINK_FUNCTION_NAME_LENGTH) == 0) {
            result = curEntry;
            break;
        }
    }
    return result;
}

dyn_linking_import_t * DynamicLinkingHelper::getOrAddFunctionImportByName(dyn_linking_relocation_data_t * data, const char* importName) {
    return getOrAddImport(data, importName, false);
}

dyn_linking_import_t * DynamicLinkingHelper::getOrAddDataImportByName(dyn_linking_relocation_data_t * data, const char* importName) {
    return getOrAddImport(data, importName, true);
}

dyn_linking_import_t * DynamicLinkingHelper::getOrAddImport(dyn_linking_relocation_data_t * data, const char* importName, bool isData) {
    if(importName == NULL || data == NULL) {
        return NULL;
    }
    dyn_linking_import_t * result = NULL;
    for(int32_t i = 0; i < DYN_LINK_IMPORT_LIST_LENGTH; i++) {
        dyn_linking_import_t * curEntry = &(data->imports[i]);
        if(strlen(curEntry->importName) == 0) {
            if(strlen(importName) > DYN_LINK_IMPORT_NAME_LENGTH) {
                DEBUG_FUNCTION_LINE("Failed to add Import, it's too long.\n");
                return NULL;
            }
            strncpy(curEntry->importName,importName,DYN_LINK_IMPORT_NAME_LENGTH);
            curEntry->isData = isData;
            result = curEntry;
            break;
        }
        if(strncmp(curEntry->importName,importName,DYN_LINK_IMPORT_NAME_LENGTH) == 0 && (curEntry->isData == isData)) {
            return curEntry;
        }
    }
    return result;
}

bool DynamicLinkingHelper::addReloationEntry(dyn_linking_relocation_data_t * linking_data, dyn_linking_relocation_entry_t * linking_entries, uint32_t linking_entry_length, const RelocationData& relocationData) {
    return addReloationEntry(linking_data, linking_entries, linking_entry_length, relocationData.getType(), relocationData.getOffset(), relocationData.getAddend(), relocationData.getDestination(), relocationData.getName(), relocationData.getImportRPLInformation());
}

bool DynamicLinkingHelper::addReloationEntry(dyn_linking_relocation_data_t * linking_data, dyn_linking_relocation_entry_t * linking_entries, uint32_t linking_entry_length, char type, size_t offset, int32_t addend, const void *destination, const std::string& name, const ImportRPLInformation& rplInfo) {
    dyn_linking_import_t * importInfoGbl = DynamicLinkingHelper::getOrAddImport(linking_data, rplInfo.getName().c_str(),rplInfo.isData());
    if(importInfoGbl == NULL) {
        DEBUG_FUNCTION_LINE("Getting import info failed. Probably maximum of %d rpl files to import reached.\n",DYN_LINK_IMPORT_LIST_LENGTH);
        return false;
    }

    dyn_linking_function_t * functionInfo = DynamicLinkingHelper::getOrAddFunctionEntryByName(linking_data, name.c_str());
    if(functionInfo == NULL) {
        DEBUG_FUNCTION_LINE("Getting import info failed. Probably maximum of %d function to be relocated reached.\n",DYN_LINK_FUNCTION_LIST_LENGTH);
        return false;
    }

    return addReloationEntry(linking_entries, linking_entry_length, type, offset, addend, destination, functionInfo, importInfoGbl);
}

bool DynamicLinkingHelper::addReloationEntry(dyn_linking_relocation_entry_t * linking_entries, uint32_t linking_entry_length, char type, size_t offset, int32_t addend, const void *destination, dyn_linking_function_t * functionName, dyn_linking_import_t * importInfo) {
    for(uint32_t i = 0; i < linking_entry_length; i++) {
        dyn_linking_relocation_entry_t * curEntry = &(linking_entries[i]);
        if(curEntry->functionEntry != NULL) {
            continue;
        }
        curEntry->type = type;
        curEntry->offset = offset;
        curEntry->addend = addend;
        curEntry->destination = (void*) destination;
        curEntry->functionEntry = functionName;
        curEntry->importEntry = importInfo;

        return true;
    }
    return false;
}
