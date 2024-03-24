#include "FunctionData.h"

FunctionData::FunctionData(void *paddress, void *vaddress, std::string_view name, function_replacement_library_type_t library, void *replaceAddr, void *replaceCall, FunctionPatcherTargetProcess targetProcess) {
    this->paddress      = paddress;
    this->vaddress      = vaddress;
    this->name          = name;
    this->library       = library;
    this->targetProcess = targetProcess;
    this->replaceAddr   = replaceAddr;
    this->replaceCall   = replaceCall;
}

FunctionData::~FunctionData() {
    if (handle != 0) {
        DEBUG_FUNCTION_LINE_WARN("Destroying FunctionData while it was still patched. This should never happen.");
        RemovePatch();
    }
}

const std::string &FunctionData::getName() const {
    return this->name;
}

function_replacement_library_type_t FunctionData::getLibrary() const {
    return this->library;
}

const void *FunctionData::getPhysicalAddress() const {
    return paddress;
}

const void *FunctionData::getVirtualAddress() const {
    return vaddress;
}

const void *FunctionData::getReplaceAddress() const {
    return replaceAddr;
}

const void *FunctionData::getReplaceCall() const {
    return replaceCall;
}

FunctionPatcherTargetProcess FunctionData::getTargetProcess() const {
    return targetProcess;
}

bool FunctionData::AddPatch() {
    if (handle == 0) {
        function_replacement_data_t functionData = {
                .version       = FUNCTION_REPLACEMENT_DATA_STRUCT_VERSION,
                .type          = FUNCTION_PATCHER_REPLACE_BY_LIB_OR_ADDRESS,
                .physicalAddr  = reinterpret_cast<uint32_t>(this->paddress),
                .virtualAddr   = reinterpret_cast<uint32_t>(this->vaddress),
                .replaceAddr   = reinterpret_cast<uint32_t>(this->replaceAddr),
                .replaceCall   = static_cast<uint32_t *>(this->replaceCall),
                .targetProcess = this->targetProcess,
                .ReplaceInRPL  = {
                         .function_name = this->name.c_str(),
                         .library       = this->library,
                }};

        if (FunctionPatcher_AddFunctionPatch(&functionData, &handle, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to add patch for function (\"%s\" PA:%08X VA:%08X)", this->name.c_str(), this->paddress, this->vaddress);
            return false;
        }
    } else {
        DEBUG_FUNCTION_LINE("Function patch has already been added.");
    }
    return true;
}

bool FunctionData::RemovePatch() {
    if (handle != 0) {
        if (FunctionPatcher_RemoveFunctionPatch(handle) != FUNCTION_PATCHER_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to remove patch for function");
            return false;
        }
        handle = 0;
    } else {
        DEBUG_FUNCTION_LINE_VERBOSE("Was not patched.");
    }

    return true;
}
