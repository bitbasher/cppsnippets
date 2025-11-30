/**
 * @file snippets.cpp
 * @brief Library version and utility functions implementation
 */

#include "cppsnippets/cppsnippets.h"

namespace cppsnippets {

static const char* VERSION_STRING = "1.0.0";
static const int VERSION_MAJOR = 1;
static const int VERSION_MINOR = 0;
static const int VERSION_PATCH = 0;

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

} // namespace cppsnippets
