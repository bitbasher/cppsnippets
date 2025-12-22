/**
 * @file scadtemplates.h
 * @brief Main header file for the scadtemplates library
 * 
 * This is the main include file for the scadtemplates library.
 * Include this file to access all library functionality.
 * 
 * @version 1.1.0
 * @author Jeff Hayes
 * @copyright MIT License
 * 
 * @section usage Usage
 * 
 * @code{.cpp}
 * #include <scadtemplates/scadtemplates.h>
 * #include <iostream>
 * 
 * int main() {
 *     scadtemplates::SnippetManager manager;
 *     
 *     // Create and add a template
 *     scadtemplates::Template tmpl("cube", "cube([$1, $2, $3]);", "Create a cube");
 *     manager.addTemplate(tmpl);
 *     
 *     // Find a template
 *     auto found = manager.findByPrefix("cube");
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

#pragma once

#include "export.h"
#include "template.h"
#include "template_parser.h"
#include "template_manager.h"

/**
 * @namespace scadtemplates
 * @brief Main namespace for the scadtemplates library
 */
namespace scadtemplates {

/**
 * @brief Get the library version string
 * @return Version string in format "major.minor.patch"
 */
SCADTEMPLATES_API const char* getVersion();

/**
 * @brief Get the library version major number
 * @return Major version number
 */
SCADTEMPLATES_API int getVersionMajor();

/**
 * @brief Get the library version minor number
 * @return Minor version number
 */
SCADTEMPLATES_API int getVersionMinor();

/**
 * @brief Get the library version patch number
 * @return Patch version number
 */
SCADTEMPLATES_API int getVersionPatch();

} // namespace scadtemplates
