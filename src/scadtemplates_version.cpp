/**
 * @file scadtemplates_version.cpp
 * @brief Library version and utility functions implementation
 */

#include "scadtemplates/scadtemplates.h"

namespace scadtemplates {

static const char* VERSION_STRING = "1.1.1";
static const int VERSION_MAJOR = 1;
static const int VERSION_MINOR = 1;
static const int VERSION_PATCH = 1;

const char* getVersion() {
    return VERSION_STRING;
}

int getVersionMajor() {
    return VERSION_MAJOR;
}

int getVersionMinor() {
    return VERSION_MINOR;
}

int getVersionPatch() {
    return VERSION_PATCH;
}

} // namespace scadtemplates
