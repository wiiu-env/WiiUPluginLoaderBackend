#include "PluginMetaInformation.h"

PluginMetaInformation::PluginMetaInformation(const PluginMetaInformation &other) {
    this->name = other.name;
    this->author = other.author;
    this->version = other.version;
    this->license = other.license;
    this->buildtimestamp = other.buildtimestamp;
    this->description = other.description;
    this->size = other.size;
    this->storageId = other.storageId;
}