# ConsumerCMakeLists.cmake
#
# Purpose: Provide copy-pasteable templates for consuming the JsonReaderPortable library
# in different scenarios. This file is intentionally not a working build; it's a
# reference for agents or developers to copy into their own projects.
#
# Choose ONE of the three options below and adapt paths/names as needed.

# -----------------------------------------------------------------------------
# Option 1: Use as a subdirectory (preferred in monorepos or when vendoring)
# -----------------------------------------------------------------------------
# Assumes JsonReaderPortable lives at contrib/jsonreader-portable relative to your root.
#
# add_subdirectory(contrib/jsonreader-portable)
# target_link_libraries(your_target PRIVATE JsonReaderPortable)
#
# Notes:
# - Requires Qt Core in your project toolchain.
# - Tests for JsonReader can be disabled with -DJSONREADER_BUILD_TESTS=OFF.

# -----------------------------------------------------------------------------
# Option 2: Use an installed package (system or prefix)
# -----------------------------------------------------------------------------
# find_package(JsonReaderPortable CONFIG REQUIRED)
# target_link_libraries(your_target PRIVATE JsonReaderPortable::JsonReaderPortable)
#
# To install locally from the source project:
#   cmake -S contrib/jsonreader-portable -B contrib/jsonreader-portable/build -G Ninja
#   cmake --build contrib/jsonreader-portable/build -j
#   cmake --install contrib/jsonreader-portable/build --prefix /path/to/prefix
# Then ensure CMAKE_PREFIX_PATH includes /path/to/prefix.

# -----------------------------------------------------------------------------
# Option 3: Vendor the two source files directly
# -----------------------------------------------------------------------------
# add_library(JsonReaderPortable STATIC
#   ${CMAKE_SOURCE_DIR}/contrib/jsonreader-portable/src/JsonReader.cc)
# target_include_directories(JsonReaderPortable PUBLIC
#   ${CMAKE_SOURCE_DIR}/contrib/jsonreader-portable/include)
# find_package(Qt6 QUIET COMPONENTS Core)
# if(NOT Qt6_FOUND)
#   find_package(Qt5 REQUIRED COMPONENTS Core)
# endif()
# if(Qt6_FOUND)
#   target_link_libraries(JsonReaderPortable PUBLIC Qt6::Core)
# else()
#   target_link_libraries(JsonReaderPortable PUBLIC Qt5::Core)
# endif()
#
# target_link_libraries(your_target PRIVATE JsonReaderPortable)

# -----------------------------------------------------------------------------
# Minimal usage in your C++ code
# -----------------------------------------------------------------------------
# #include <QJsonObject>
# #include "JsonReader/JsonReader.h"
# JsonErrorInfo err; QJsonObject obj;
# if (!JsonReader::readObject("/path/to/template.json", obj, err)) {
#   // Handle parse error: err.formatError() → "file:line:column: message"
# }
