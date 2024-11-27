#include "FunctionData.h"
#include "utils/logger.h"

#include <function_patcher/function_patching.h>
#include <string>

FunctionData::FunctionData(void *paddress, void *vaddress,
                           const std::string_view name,
                           const function_replacement_library_type_t library,
                           void *replaceAddr, void *replaceCall,
                           const FunctionPatcherTargetProcess targetProcess) {
    this->mPAddress      = paddress;
    this->mVAddress      = vaddress;
    this->mName          = name;
    this->mLibrary       = library;
    this->mTargetProcess = targetProcess;
    this->mReplaceAddr   = replaceAddr;
    this->mReplaceCall   = replaceCall;
}

FunctionData::~FunctionData() {
    if (mHandle != 0) {
        DEBUG_FUNCTION_LINE_WARN("Destroying FunctionData while it was still patched. This should never happen.");
        RemovePatch();
    }
}

const std::string &FunctionData::getName() const {
    return this->mName;
}

function_replacement_library_type_t FunctionData::getLibrary() const {
    return this->mLibrary;
}

const void *FunctionData::getPhysicalAddress() const {
    return mPAddress;
}

const void *FunctionData::getVirtualAddress() const {
    return mVAddress;
}

const void *FunctionData::getReplaceAddress() const {
    return mReplaceAddr;
}

const void *FunctionData::getReplaceCall() const {
    return mReplaceCall;
}

FunctionPatcherTargetProcess FunctionData::getTargetProcess() const {
    return mTargetProcess;
}

bool FunctionData::AddPatch() {
    if (mHandle == 0) {
        function_replacement_data_t functionData = {
                .version       = FUNCTION_REPLACEMENT_DATA_STRUCT_VERSION,
                .type          = FUNCTION_PATCHER_REPLACE_BY_LIB_OR_ADDRESS,
                .physicalAddr  = reinterpret_cast<uint32_t>(this->mPAddress),
                .virtualAddr   = reinterpret_cast<uint32_t>(this->mVAddress),
                .replaceAddr   = reinterpret_cast<uint32_t>(this->mReplaceAddr),
                .replaceCall   = static_cast<uint32_t *>(this->mReplaceCall),
                .targetProcess = this->mTargetProcess,
                .ReplaceInRPL  = {
                         .function_name = this->mName.c_str(),
                         .library       = this->mLibrary,
                }};

        if (FunctionPatcher_AddFunctionPatch(&functionData, &mHandle, nullptr) != FUNCTION_PATCHER_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to add patch for function (\"%s\" PA:%08X VA:%08X)", this->mName.c_str(), this->mPAddress, this->mVAddress);
            return false;
        }
    } else {
        DEBUG_FUNCTION_LINE("Function patch has already been added.");
    }
    return true;
}

bool FunctionData::RemovePatch() {
    if (mHandle != 0) {
        if (FunctionPatcher_RemoveFunctionPatch(mHandle) != FUNCTION_PATCHER_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to remove patch for function");
            return false;
        }
        mHandle = 0;
    } else {
        DEBUG_FUNCTION_LINE_VERBOSE("Was not patched.");
    }

    return true;
}
