#include "ImportRPLInformation.h"
#include <cstring>

ImportRPLInformation::ImportRPLInformation(std::string_view name) {
    this->mName = name;
}

ImportRPLInformation::~ImportRPLInformation() = default;

[[nodiscard]] const std::string &ImportRPLInformation::getName() const {
    return mName;
}

[[nodiscard]] std::string ImportRPLInformation::getRPLName() const {
    return mName.substr(strlen(".dimport_"));
}

[[nodiscard]] bool ImportRPLInformation::isData() const {
    return mName.starts_with(".dimport_");
}