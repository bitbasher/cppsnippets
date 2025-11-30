/**
 * @file cppsnippets.h
 * @brief Main header file for the cppsnippets library
 * 
 * This is the main include file for the cppsnippets library.
 * Include this file to access all library functionality.
 * 
 * @version 1.0.0
 * @author Jeff Hayes
 * @copyright MIT License
 * 
 * @section usage Usage
 * 
 * @code{.cpp}
 * #include <cppsnippets/cppsnippets.h>
 * #include <iostream>
 * 
 * int main() {
 *     cppsnippets::SnippetManager manager;
 *     
 *     // Create and add a snippet
 *     cppsnippets::Snippet snippet("cout", "std::cout << $1 << std::endl;", "Output to console");
 *     manager.addSnippet(snippet);
 *     
 *     // Find a snippet
 *     auto found = manager.findByPrefix("cout");
 *     if (found) {
 *         std::cout << found->getBody() << std::endl;
 *     }
 *     
 *     return 0;
 * }
 * @endcode
 * 
 * @section building Building
 * 
 * The library can be built using CMake:
 * @code{.sh}
 * mkdir build && cd build
 * cmake ..
 * cmake --build .
 * @endcode
 * 
 * Build options:
 * - BUILD_SHARED_LIBS: Build as shared library (default: ON)
 * - BUILD_TESTS: Build test suite (default: ON)
 * - BUILD_APP: Build Qt application (default: ON)
 */

#ifndef CPPSNIPPETS_H
#define CPPSNIPPETS_H

#include "export.h"
#include "snippet.h"
#include "snippet_parser.h"
#include "snippet_manager.h"

/**
 * @namespace cppsnippets
 * @brief Main namespace for the cppsnippets library
 */
namespace cppsnippets {

/**
 * @brief Get the library version string
 * @return Version string in format "major.minor.patch"
 */
CPPSNIPPETS_API const char* getVersion();

/**
 * @brief Get the library version major number
 * @return Major version number
 */
CPPSNIPPETS_API int getVersionMajor();

/**
 * @brief Get the library version minor number
 * @return Minor version number
 */
CPPSNIPPETS_API int getVersionMinor();

/**
 * @brief Get the library version patch number
 * @return Patch version number
 */
CPPSNIPPETS_API int getVersionPatch();

} // namespace cppsnippets

#endif // CPPSNIPPETS_H
