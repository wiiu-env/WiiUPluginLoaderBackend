#pragma once
#include "common/dynamic_linking_defines.h"
#include "utils/logger.h"
#include <string>
#include <vector>
#include "RelocationData.h"

class DynamicLinkingHelper {
public:
    /**
        Gets the function entry for a given function name. If the function name is not present in the list, it will be added.

        \param functionName Name of the function
        \return Returns a pointer to the entry which contains the functionName. Null on error or if the list full.
    **/
    static dyn_linking_function_t * getOrAddFunctionEntryByName(dyn_linking_relocation_data_t * data, const char * functionName);

    /**
        Gets the function import entry for a given function name. If the import is not present in the list, it will be added.
        This will return entries for _function_ imports.

        \param importName Name of the function
        \return Returns a pointer to the function import entry which contains the importName. Null on error or if the list full.
    **/
    static dyn_linking_import_t * getOrAddFunctionImportByName(dyn_linking_relocation_data_t * data, const char * importName);


    /**
        Gets the data import entry for a given data name. If the import is not present in the list, it will be added.
        This will return entries for _data_ imports.

        \param importName Name of the data
        \return Returns a pointer to the data import entry which contains the importName. Null on error or if the list full.
    **/
    static dyn_linking_import_t * getOrAddDataImportByName(dyn_linking_relocation_data_t * data, const char * importName);


    /**
        Gets the import entry for a given data name and type. If the import is not present in the list, it will be added.
        This will return entries for _data_ and _function_ imports, depending on the isData parameter.

        \param importName Name of the data
        \param isData Set this to true to return a data import

        \return Returns a pointer to the data import entry which contains the importName. Null on error or if the list full.
    **/
    static dyn_linking_import_t * getOrAddImport(dyn_linking_relocation_data_t * data, const char * importName, bool isData);

    static bool addReloationEntry(dyn_linking_relocation_data_t * linking_data, dyn_linking_relocation_entry_t * linking_entries, uint32_t linking_entry_length, const RelocationData& relocationData);

    static bool addReloationEntry(dyn_linking_relocation_data_t * linking_data, dyn_linking_relocation_entry_t * linking_entries, uint32_t linking_entry_length, char type, size_t offset, int32_t addend, const void *destination, const std::string& name, const ImportRPLInformation& rplInfo);

    static bool addReloationEntry(dyn_linking_relocation_entry_t * linking_entries, uint32_t linking_entry_length, char type, size_t offset, int32_t addend, const void *destination, dyn_linking_function_t * functionName, dyn_linking_import_t * importInfo);
private:
    DynamicLinkingHelper() {
    }

    ~DynamicLinkingHelper() {
    }
};
