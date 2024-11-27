#include "HookData.h"

HookData::HookData(void *functionPointer, const wups_loader_hook_type_t type) {
    this->mFunctionPointer = functionPointer;
    this->mType            = type;
}

HookData::~HookData() = default;

[[nodiscard]] void *HookData::getFunctionPointer() const {
    return mFunctionPointer;
}

[[nodiscard]] wups_loader_hook_type_t HookData::getType() const {
    return mType;
}