cmake_minimum_required(VERSION 3.16)

# Job pool configuration to prevent system crashes
# Limit compilation to -j4 and linking to -j2 to reduce memory pressure
set_property(GLOBAL APPEND PROPERTY JOB_POOLS compile_pool=4 link_pool=2)
set(CMAKE_JOB_POOL_COMPILE compile_pool)
set(CMAKE_JOB_POOL_LINK link_pool)

# Let AUTOMOC and AUTOUIC process GENERATED files.
cmake_policy(SET CMP0071 NEW)

# FindOpenGL prefers GLVND by default when available.
cmake_policy(SET CMP0072 NEW)

# `target_link_libraries()` allows use with targets in other directories.
cmake_policy(SET CMP0079 NEW)

# Let parent project set child project options (used to skip python lookup from manifold & force TBB)
cmake_policy(SET CMP0079 NEW)

if(POLICY CMP0082) # Since CMake 3.14
    # Install rules from add_subdirectory() calls are interleaved with those in caller.
    cmake_policy(SET CMP0082 NEW)
endif()

if(POLICY CMP0167) # Since CMake 3.30
    # Use upstream's FindBoost.cmake to find Boost libraries.
    # this project does not use boost, but this may be needed
    # in future for compatibiltiy with OpenSCAD
    cmake_policy(SET CMP0167 NEW)
endif()

if(POLICY CMP0200) # Since CMake 3.30
    # Location and configuration selection for imported targets is more consistent
    cmake_policy(SET CMP0200 NEW)
endif()

# ============================================================================
# Application Metadata - Load from AppInfo.cmake
# ============================================================================
include(${CMAKE_SOURCE_DIR}/AppInfo.cmake)

# Optional: Override application name for test executables
# Usage: cmake -DTESTAPPNAME="OpenSCAD" ...
set(TESTAPPNAME "" CACHE STRING "Override app name for test executables (e.g., 'OpenSCAD')")

# Get Git commit count for auto-incrementing minor version and short hash for display
find_package(Git QUIET)
set(GIT_COMMIT_HASH "unknown")

if(Git_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_COUNT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE GIT_RESULT
    )

    if(NOT GIT_RESULT EQUAL 0)
        set(GIT_COMMIT_COUNT 0)
    endif()

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short=8 HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE GIT_HASH_RESULT
    )

    if(NOT GIT_HASH_RESULT EQUAL 0)
        set(GIT_COMMIT_HASH "unknown")
    endif()
else()
    set(GIT_COMMIT_COUNT 0)
endif()

# Auto-increment major version every 100 commits, minor wraps 0-99
math(EXPR MAJOR_INCREMENT "${GIT_COMMIT_COUNT} / 100")
math(EXPR APP_VERSION_MINOR "${GIT_COMMIT_COUNT} % 100")
math(EXPR APP_VERSION_MAJOR "${APP_VERSION_MAJOR} + ${MAJOR_INCREMENT}")

# Compose full version string
set(APP_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_PATCH}")

# Build display suffix (add space and parentheses if suffix is provided)
if(APP_SUFFIX)
    set(APP_DISPLAY_SUFFIX " (${APP_SUFFIX})")
    set(APP_DISPLAY_NAME "${APP_NAME}${APP_DISPLAY_SUFFIX}")
else()
    set(APP_DISPLAY_SUFFIX "")
    set(APP_DISPLAY_NAME "${APP_NAME}")
endif()

# Create safe filename versions (lowercase, spaces/parens → dashes)
string(TOLOWER "${APP_NAME}" APP_NAME_SAFE)
string(TOLOWER "${APP_DISPLAY_NAME}" APP_DISPLAY_SAFE)
string(REPLACE " " "-" APP_DISPLAY_SAFE "${APP_DISPLAY_SAFE}")
string(REPLACE "(" "" APP_DISPLAY_SAFE "${APP_DISPLAY_SAFE}")
string(REPLACE ")" "" APP_DISPLAY_SAFE "${APP_DISPLAY_SAFE}")

# ============================================================================
project(scadtemplates
    VERSION ${APP_VERSION}
    DESCRIPTION "A code template solution for OpenSCAD"
    HOMEPAGE_URL "https://jartisan.com/"
    LANGUAGES CXX
)

message(STATUS "${APP_DISPLAY_NAME} version: ${APP_VERSION} (Git: ${GIT_COMMIT_HASH})")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Options
option(INFO "Display build configuration info at end of cmake config" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build test suite" OFF)
option(BUILD_APP "Build Qt application" OFF)  # Disabled: GUI still uses Phase 1 architecture
option(USE_QSCINTILLA "Enable QScintilla editor integration" OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Auto-detect Qt6 path based on compiler
if(NOT DEFINED CMAKE_PREFIX_PATH OR CMAKE_PREFIX_PATH STREQUAL "")
    if(MSVC)
        set(QT6_DEFAULT_PATH "C:/bin/Qt/6.10.0/msvc2022_64")
    elseif(MINGW)
        set(QT6_DEFAULT_PATH "C:/bin/Qt/6.10.0/mingw_64")
    else()
        set(QT6_DEFAULT_PATH "")
    endif()

    if(EXISTS "${QT6_DEFAULT_PATH}/lib/cmake/Qt6/Qt6Config.cmake")
        set(CMAKE_PREFIX_PATH "${QT6_DEFAULT_PATH}" CACHE PATH "Path to Qt6 installation")
        message(STATUS "Auto-detected Qt6 at: ${QT6_DEFAULT_PATH}")
    else()
        message(WARNING "Qt6 not found at default path: ${QT6_DEFAULT_PATH}")
        message(WARNING "Please set CMAKE_PREFIX_PATH to your Qt6 installation")
    endif()
endif()

# Find Qt6 Core and Gui (required for library - QString, QVector, QSysInfo, QScreen)
find_package(Qt6 COMPONENTS Core Gui REQUIRED)
message(STATUS "Found Qt6::Core at ${Qt6_DIR}")
message(STATUS "Found Qt6::Gui at ${Qt6Gui_DIR}")

# Find Qt6::Test for QTest-based tests (optional)
if(BUILD_TESTS)
    find_package(Qt6 COMPONENTS Test QUIET)
    if(Qt6Test_FOUND)
        message(STATUS "Found Qt6::Test - QTest-based tests will be built")
    else()
        message(WARNING "Qt6::Test not found - QTest-based tests will be skipped")
    endif()
endif()

# Find JSON libraries (nlohmann-json required by json-schema-validator)
find_package(nlohmann_json REQUIRED)
message(STATUS "Found nlohmann_json at ${nlohmann_json_DIR}")

find_package(nlohmann_json_schema_validator REQUIRED)
message(STATUS "Found nlohmann_json_schema_validator at ${nlohmann_json_schema_validator_DIR}")

# Find Qt6 Widgets for application (optional)
if(BUILD_APP)
    find_package(Qt6 COMPONENTS Widgets QUIET)

    if(NOT Qt6Widgets_FOUND)
        message(WARNING "Qt6::Widgets not found - Qt application will not be built")
        set(BUILD_APP OFF)
    else()
        set(DEPLOYMENT_TARGET "11.0")
        set(QT_VERSION 6)
        set(QT_LIBRARIES Qt6::Core Qt6::Widgets)
        message(STATUS "Found Qt6::Widgets - application will be built")
    endif()

    if(BUILD_APP)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)

        # Optional QScintilla support
        if(USE_QSCINTILLA)
            find_package(QScintilla QUIET)

            if(NOT QScintilla_FOUND)
                # Try pkg-config as fallback
                find_package(PkgConfig QUIET)

                if(PkgConfig_FOUND)
                    pkg_check_modules(QSCINTILLA qscintilla2-qt6)
                endif()
            endif()

            if(QScintilla_FOUND OR QSCINTILLA_FOUND)
                message(STATUS "Found QScintilla - editor integration enabled")
                add_compile_definitions(HAS_QSCINTILLA)
            else()
                message(WARNING "QScintilla not found - editor integration disabled")
                message(WARNING "Install via: vcpkg install qscintilla:x64-windows")
                message(WARNING "Or download from: https://www.riverbankcomputing.com/software/qscintilla/")
                set(USE_QSCINTILLA OFF)
            endif()
        endif()
    endif()
endif()

# Use clang-tidy if run with -DCLANG_TIDY=1
set(CLANG_TIDY ${CLANG_TIDY} CACHE BOOL "Enable clang-tidy")

if(CLANG_TIDY)
    if(APPLE)
        # clang-tidy isn't directly available on Homebrew, but exists inside the llvm package
        set(CLANG_TIDY_HINTS "/opt/homebrew/opt/llvm/bin" "/usr/local/opt/llvm/bin")
    endif()

    find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED HINTS ${CLANG_TIDY_HINTS})
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    include("cmake/Modules/RegexUtils.cmake")
    escape_string_as_regex(regex "${CMAKE_SOURCE_DIR}/src/")

    # /src/ext/.clang-tidy disables all checks for that dir.  Copy into build dir to ignore generated sources.
    configure_file(${CMAKE_SOURCE_DIR}/src/ext/.clang-tidy ${CMAKE_BINARY_DIR} COPYONLY)

    # Regex hack below since negative lookahead is not supported (POSIX regex only)
    # (?!ext/) becomes ([^e]...|e[^x]..|ex[^t].|ext[^/]|.{0,3}$)
    # CMAKE_CXX_CLANG_TIDY must be set before target is created (with add_executable())
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--header-filter=${regex}([^e]...|e[^x]..|ex[^t].|ext[^/]|.{0,3}$)")

    # append ";--fix" to arguments above to apply automatic fix (if the given checks support it)
endif(CLANG_TIDY)

# Include directories
# Note: src/include is removed - headers are now co-located with .cpp files
# Add src/ to include path so headers in all subdirectories can be found
# New structure: platformInfo, resourceMetadata, pathDiscovery, resourceDiscovery, resourceScanning, resourceInventory
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/generated)

# Generate application metadata header from template
# If TESTAPPNAME is set, use it; otherwise use APP_NAME
if(TESTAPPNAME)
    set(EFFECTIVE_APP_NAME "${TESTAPPNAME}")
else()
    set(EFFECTIVE_APP_NAME "${APP_NAME}")
endif()

configure_file(
    ${CMAKE_SOURCE_DIR}/src/app/applicationNameInfo.hpp.in
    ${CMAKE_BINARY_DIR}/generated/applicationNameInfo.hpp
    @ONLY
)

# Generate Windows version resource file (WIN32 only)
if(WIN32)
    configure_file(
        ${CMAKE_SOURCE_DIR}/src/app/version.rc.in
        ${CMAKE_BINARY_DIR}/generated/version.rc
        @ONLY
    )
endif()

# Library source files
set(LIB_SOURCES
    src/jsonreader/JsonReader.cpp
    src/jsonwriter/jsonwriter-portable/src/JsonWriter.cc
    src/scadtemplates/template_parser.cpp
    src/scadtemplates/template_manager.cpp
    src/scadtemplates/legacy_template_converter.cpp
    src/scadtemplates/scad_template_session.cpp
    src/platformInfo/platformInfo.cpp
    src/platformInfo/ResourceLocation.cpp
    src/resourceMetadata/ResourceTypeInfo.cpp
    src/pathDiscovery/ResourcePaths.cpp
    src/resourceInventory/resourceItem.cpp
    src/resourceInventory/ResourceIndexer.cpp
    src/resourceInventory/ExamplesInventory.cpp
    src/resourceInventory/TemplatesInventory.cpp
    src/resourceInventory/FontsInventory.cpp
    src/resourceInventory/ShadersInventory.cpp
    src/resourceInventory/TranslationsInventory.cpp
    src/resourceInventory/TestsInventory.cpp
    src/resourceScanning/ResourceScanner.cpp
)

set(LIB_HEADERS
    src/jsonreader/JsonReader.hpp
    src/jsonwriter/jsonwriter-portable/include/JsonWriter/JsonWriter.h
    src/scadtemplates/scadtemplates.hpp
    src/scadtemplates/template_parser.hpp
    src/scadtemplates/template_manager.hpp
    src/scadtemplates/scad_template_session.hpp
    src/scadtemplates/export.hpp
    src/scadtemplates/editsubtype.hpp
    src/scadtemplates/edittype.hpp
    src/platformInfo/export.hpp
    src/platformInfo/platformInfo.hpp
    src/platformInfo/ResourceLocation.hpp
    src/pathDiscovery/ResourcePaths.hpp
    src/resourceInventory/resourceItem.hpp
    src/resourceInventory/ResourceIndexer.hpp
    src/resourceInventory/ExamplesInventory.hpp
    src/resourceInventory/TemplatesInventory.hpp
    src/resourceScanning/ResourceScanner.hpp
    src/resourceMetadata/ResourceTypeInfo.hpp
)

# Build the shared/dynamic library
add_library(scadtemplates_lib ${LIB_SOURCES} ${LIB_HEADERS})
target_include_directories(scadtemplates_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/jsonwriter/jsonwriter-portable/include>
    $<INSTALL_INTERFACE:include>
)

# Link Qt6::Core and Qt6::Gui for QString, QVector, QSysInfo, QScreen support in library
# JsonReader is integrated directly into library, no separate dependency needed
# JsonWriter is integrated directly into library
# Link nlohmann_json and json-schema-validator for template validation
target_link_libraries(scadtemplates_lib PUBLIC 
    Qt6::Core 
    Qt6::Gui
    nlohmann_json::nlohmann_json
    nlohmann_json_schema_validator::validator
)

# Set library properties for Windows DLL export
if(BUILD_SHARED_LIBS)
    target_compile_definitions(scadtemplates_lib PRIVATE 
        SCADTEMPLATES_EXPORTS 
        PLATFORMINFO_EXPORTS
        RESOURCESCANNING_EXPORTS
    )
endif()

# Set output name and versioning
set_target_properties(scadtemplates_lib PROPERTIES
    OUTPUT_NAME ${APP_DISPLAY_SAFE}
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Build Qt application
if(BUILD_APP)
    set(APP_SOURCES
        src/app/main.cpp
        src/app/mainwindow.cpp
        src/app/mainwindow.hpp
        src/gui/preferencesdialog.cpp
        src/gui/preferencesdialog.hpp
        src/gui/resourcelocationwidget.cpp
        src/gui/resourceLocationWidget.hpp
        src/gui/locationInputWidget.cpp
        src/gui/locationInputWidget.hpp
        src/gui/platformInfoWidget.cpp
        src/gui/platformInfoWidget.hpp
        src/gui/dialogButtonBar.cpp
        src/gui/dialogButtonBar.hpp
        src/gui/installationTab.cpp
        src/gui/installationTab.hpp
        src/gui/machineTab.cpp
        src/gui/machineTab.hpp
        src/gui/userTab.cpp
        src/gui/userTab.hpp
        src/gui/aboutDialog.cpp
        src/gui/aboutDialog.hpp
        src/gui/sysInfoDialog.cpp
        src/gui/sysInfoDialog.hpp

        # Resource inventory GUI components (require Qt Widgets)
        src/resourceInventory/resourceTreeWidget.cpp
        src/resourceInventory/resourceTreeWidget.hpp
    )

    # Add Windows version resource to executable
    if(WIN32)
        list(APPEND APP_SOURCES ${CMAKE_BINARY_DIR}/generated/version.rc)
    endif()

    add_executable(scadtemplates_app ${APP_SOURCES})
    target_link_libraries(scadtemplates_app PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(scadtemplates_app PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    )
    set_target_properties(scadtemplates_app PROPERTIES
        OUTPUT_NAME ${APP_DISPLAY_SAFE}
        WIN32_EXECUTABLE ON
    )

    # Environment variable expansion test utility (standalone)
    add_executable(test_sa_env_expansion EXCLUDE_FROM_ALL src/tools/test_sa_env_expansion.cpp)
    target_link_libraries(test_sa_env_expansion PRIVATE Qt6::Core)
    target_include_directories(test_sa_env_expansion PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    )
    set_target_properties(test_sa_env_expansion PROPERTIES
        OUTPUT_NAME test_sa_env_expansion
        WIN32_EXECUTABLE OFF # Console app
    )

    # Settings generator utility for user-designated paths
    add_executable(settings_generator EXCLUDE_FROM_ALL src/tools/settings_generator.cpp)
    target_link_libraries(settings_generator PRIVATE Qt6::Core)
    set_target_properties(settings_generator PROPERTIES
        OUTPUT_NAME settings_generator
        WIN32_EXECUTABLE OFF # Console app
    )

    # Path discovery test app (resourceMetadata → pathDiscovery workflow)
    # Path discovery test - compiles its own ResourcePaths with test header
    # This avoids DLL boundary issues with runtime app name override
    add_executable(test_sa_path_discovery EXCLUDE_FROM_ALL
        tests/test_sa_path_discovery.cpp
        src/pathDiscovery/ResourcePaths.cpp
        src/platformInfo/platformInfo.cpp
        src/platformInfo/ResourceLocation.cpp
        src/resourceMetadata/ResourceTypeInfo.cpp
    )
    
    target_link_libraries(test_sa_path_discovery PRIVATE ${QT_LIBRARIES})
    target_include_directories(test_sa_path_discovery PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>
    )

    # Override applicationNameInfo.hpp with test-specific version
    # Also suppress DLL export macros since we're compiling ResourcePaths directly
    target_compile_definitions(test_sa_path_discovery PRIVATE 
        USE_TEST_APP_INFO
        PLATFORMINFO_STATIC_DEFINE
        RESOURCEMETADATA_STATIC_DEFINE
    )
    set_target_properties(test_sa_path_discovery PROPERTIES
        OUTPUT_NAME test_sa_path_discovery
        WIN32_EXECUTABLE OFF
    )

    # Tier standalone test (minimal test for ResourceTier)
    add_executable(test_sa_tier EXCLUDE_FROM_ALL
        tests/test_sa_tier.cpp
        src/pathDiscovery/ResourcePaths.cpp
        src/platformInfo/platformInfo.cpp
        src/platformInfo/ResourceLocation.cpp
        src/resourceMetadata/ResourceTypeInfo.cpp
    )
    
    target_link_libraries(test_sa_tier PRIVATE ${QT_LIBRARIES})
    target_include_directories(test_sa_tier PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>
    )

    # Override applicationNameInfo.hpp with test-specific version
    # Also suppress DLL export macros since we're compiling ResourcePaths directly
    target_compile_definitions(test_sa_tier PRIVATE 
        USE_TEST_APP_INFO
        PLATFORMINFO_STATIC_DEFINE
        RESOURCEMETADATA_STATIC_DEFINE
    )
    set_target_properties(test_sa_tier PROPERTIES
        OUTPUT_NAME test_sa_tier
        WIN32_EXECUTABLE OFF
    )

    # Location discovery standalone test (demonstrates PathElement → ResourceLocation workflow)
    add_executable(test_sa_location_discovery EXCLUDE_FROM_ALL
        tests/test_sa_location_discovery.cpp
        src/pathDiscovery/ResourcePaths.cpp
        src/platformInfo/platformInfo.cpp
        src/platformInfo/ResourceLocation.cpp
        src/resourceMetadata/ResourceTypeInfo.cpp
    )
    
    target_link_libraries(test_sa_location_discovery PRIVATE ${QT_LIBRARIES})
    target_include_directories(test_sa_location_discovery PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>
    )

    # Override applicationNameInfo.hpp with test-specific version
    # Also suppress DLL export macros since we're compiling ResourcePaths directly
    target_compile_definitions(test_sa_location_discovery PRIVATE 
        USE_TEST_APP_INFO
        PLATFORMINFO_STATIC_DEFINE
        RESOURCEMETADATA_STATIC_DEFINE
    )
    set_target_properties(test_sa_location_discovery PROPERTIES
        OUTPUT_NAME test_sa_location_discovery
        WIN32_EXECUTABLE OFF
    )

    # Standalone test: Display templates inventory as a tree
    add_executable(test_sa_inventory_tree EXCLUDE_FROM_ALL
        tests/test_sa_inventory_tree.cpp
    )
    target_link_libraries(test_sa_inventory_tree PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(test_sa_inventory_tree PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/platformInfo
        ${CMAKE_SOURCE_DIR}/src/pathDiscovery
        ${CMAKE_SOURCE_DIR}/src/resourceScanning
        ${CMAKE_SOURCE_DIR}/src/resourceInventory
        ${CMAKE_SOURCE_DIR}/src/resourceMetadata
    )
    target_compile_definitions(test_sa_inventory_tree PRIVATE 
        USE_TEST_APP_INFO
        $<$<CONFIG:Debug>:DEBUG_BUILD>
    )
    set_target_properties(test_sa_inventory_tree PROPERTIES
        OUTPUT_NAME test_sa_inventory_tree
        WIN32_EXECUTABLE OFF
    )

    # Standalone test: Display examples inventory as a tree with attachments
    add_executable(test_sa_examples_tree EXCLUDE_FROM_ALL
        tests/test_sa_examples_tree.cpp
    )
    target_link_libraries(test_sa_examples_tree PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(test_sa_examples_tree PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/platformInfo
        ${CMAKE_SOURCE_DIR}/src/pathDiscovery
        ${CMAKE_SOURCE_DIR}/src/resourceScanning
        ${CMAKE_SOURCE_DIR}/src/resourceInventory
        ${CMAKE_SOURCE_DIR}/src/resourceMetadata
    )
    target_compile_definitions(test_sa_examples_tree PRIVATE 
        USE_TEST_APP_INFO
        $<$<CONFIG:Debug>:DEBUG_BUILD>
    )
    set_target_properties(test_sa_examples_tree PROPERTIES
        OUTPUT_NAME test_sa_examples_tree
        WIN32_EXECUTABLE OFF
    )

    # Standalone test: Display fonts inventory as a tree
    add_executable(test_sa_fonts_tree EXCLUDE_FROM_ALL
        tests/test_sa_fonts_tree.cpp
    )
    target_link_libraries(test_sa_fonts_tree PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(test_sa_fonts_tree PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/platformInfo
        ${CMAKE_SOURCE_DIR}/src/pathDiscovery
        ${CMAKE_SOURCE_DIR}/src/resourceScanning
        ${CMAKE_SOURCE_DIR}/src/resourceInventory
        ${CMAKE_SOURCE_DIR}/src/resourceMetadata
    )
    target_compile_definitions(test_sa_fonts_tree PRIVATE 
        USE_TEST_APP_INFO
        $<$<CONFIG:Debug>:DEBUG_BUILD>
    )
    set_target_properties(test_sa_fonts_tree PROPERTIES
        OUTPUT_NAME test_sa_fonts_tree
        WIN32_EXECUTABLE OFF
    )

    # Standalone test: Scanner output verification
    add_executable(test_sa_scanner_output EXCLUDE_FROM_ALL
        tests/test_sa_scanner_output.cpp
        src/pathDiscovery/ResourcePaths.cpp
        src/platformInfo/platformInfo.cpp
        src/platformInfo/ResourceLocation.cpp
        src/resourceMetadata/ResourceTypeInfo.cpp
        src/resourceScanning/ResourceScanner.cpp
        src/resourceInventory/ExamplesInventory.cpp
        src/resourceInventory/TemplatesInventory.cpp
        src/resourceInventory/FontsInventory.cpp
        src/resourceInventory/ShadersInventory.cpp
        src/resourceInventory/TranslationsInventory.cpp
        src/resourceInventory/TestsInventory.cpp
        src/resourceInventory/resourceItem.cpp
        src/scadtemplates/template_parser.cpp
        src/scadtemplates/legacy_template_converter.cpp
        src/jsonreader/JsonReader.cpp
        src/jsonwriter/jsonwriter-portable/src/JsonWriter.cc
    )
    target_link_libraries(test_sa_scanner_output PRIVATE 
        ${QT_LIBRARIES}
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator::validator
    )
    target_include_directories(test_sa_scanner_output PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/jsonwriter/jsonwriter-portable/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>
    )
    target_compile_definitions(test_sa_scanner_output PRIVATE 
        USE_TEST_APP_INFO
        PLATFORMINFO_STATIC_DEFINE
        RESOURCEMETADATA_STATIC_DEFINE
        PLATFORMINFO_EXPORTS
        SCADTEMPLATES_EXPORTS
        RESOURCESCANNING_EXPORTS
        $<$<CONFIG:Debug>:DEBUG_BUILD>
    )
    set_target_properties(test_sa_scanner_output PROPERTIES
        OUTPUT_NAME test_sa_scanner_output
        WIN32_EXECUTABLE OFF
    )

    # Model indexing example (console test) - Google Test unit test
    add_executable(test_unit_g_model_indexing
        tests/test_unit_g_model_indexing.cpp
    )

    target_link_libraries(test_unit_g_model_indexing PRIVATE
        Qt6::Core
        Qt6::Gui
    )

    set_target_properties(test_unit_g_model_indexing PROPERTIES
        OUTPUT_NAME test_unit_g_model_indexing
        WIN32_EXECUTABLE OFF
    )

    # Phase 1: QVariant registration and storage tests - Google Test unit test
    add_executable(test_unit_g_qvariant_phase1
        tests/test_unit_g_qvariant_phase1.cpp
        src/resourceInventory/resourceItem.cpp
        src/resourceInventory/ResourceIndexer.cpp
        src/resourceMetadata/ResourceTypeInfo.cpp
    )

    # Phase 2: End-to-end pipeline tests - Google Test unit test
    add_executable(test_unit_g_phase2_inventory_pipeline
        tests/test_unit_g_phase2_inventory_pipeline.cpp
        src/resourceInventory/ExamplesInventory.cpp
        src/resourceInventory/TemplatesInventory.cpp
        src/resourceInventory/resourceItem.cpp
        src/resourceInventory/ResourceIndexer.cpp
        src/resourceMetadata/ResourceTypeInfo.cpp
        src/scadtemplates/legacy_template_converter.cpp
        src/platformInfo/ResourceLocation.cpp
        src/pathDiscovery/ResourcePaths.cpp
        src/jsonreader/JsonReader.cpp
        src/jsonwriter/jsonwriter-portable/src/JsonWriter.cc
    )

    # Phase 2: ExamplesInventory class tests - Google Test unit test
    add_executable(test_unit_g_examples_inventory
        tests/test_unit_g_examples_inventory.cpp
        src/resourceInventory/ExamplesInventory.cpp
        src/resourceInventory/resourceItem.cpp
        src/resourceInventory/ResourceIndexer.cpp
        src/resourceMetadata/ResourceTypeInfo.cpp
    )

    # Phase 4: TemplatesInventory class tests - Google Test unit test
    add_executable(test_unit_g_templates_inventory
        tests/test_unit_g_templates_inventory.cpp
        src/resourceInventory/TemplatesInventory.cpp
        src/resourceInventory/resourceItem.cpp
        src/resourceInventory/ResourceIndexer.cpp
        src/resourceMetadata/ResourceTypeInfo.cpp
        src/scadtemplates/legacy_template_converter.cpp
        src/jsonreader/JsonReader.cpp
        src/jsonwriter/jsonwriter-portable/src/JsonWriter.cc
    )

    # Phase 3: ResourceScanner orchestration tests - Google Test unit test
    add_executable(test_unit_g_resource_scanner EXCLUDE_FROM_ALL
        tests/test_unit_g_resource_scanner.cpp
    )

    target_link_libraries(test_unit_g_qvariant_phase1 PRIVATE
        Qt6::Core
        Qt6::Gui
        scadtemplates_lib  # Link to scadtemplates library for TemplateParser
        GTest::gtest
        GTest::gtest_main
    )

    # Standalone test - compile sources directly, not using DLL
    target_include_directories(test_unit_g_qvariant_phase1 PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>
    )
    
    # Neutralize DLL export macros for standalone compilation
    target_compile_definitions(test_unit_g_qvariant_phase1 PRIVATE
        PLATFORMINFO_EXPORTS
        SCADTEMPLATES_EXPORTS
        RESOURCESCANNING_EXPORTS
    )

    target_link_libraries(test_unit_g_phase2_inventory_pipeline PRIVATE
        Qt6::Core
        Qt6::Gui
        scadtemplates_lib  # Link to scadtemplates library for TemplateParser
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator::validator
        GTest::gtest
        GTest::gtest_main
    )

    target_include_directories(test_unit_g_phase2_inventory_pipeline PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/jsonwriter/jsonwriter-portable/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>
    )
    
    target_compile_definitions(test_unit_g_phase2_inventory_pipeline PRIVATE
        PLATFORMINFO_EXPORTS
        SCADTEMPLATES_EXPORTS
        RESOURCESCANNING_EXPORTS
    )

    target_link_libraries(test_unit_g_examples_inventory PRIVATE
        Qt6::Core
        Qt6::Gui
        scadtemplates_lib  # Link to scadtemplates library for TemplateParser
        GTest::gtest
        GTest::gtest_main
    )

    # Standalone test - compile sources directly
    target_include_directories(test_unit_g_examples_inventory PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>
    )
    
    target_compile_definitions(test_unit_g_examples_inventory PRIVATE
        PLATFORMINFO_EXPORTS
        SCADTEMPLATES_EXPORTS
        RESOURCESCANNING_EXPORTS
    )

    target_link_libraries(test_unit_g_templates_inventory PRIVATE
        Qt6::Core
        Qt6::Gui
        scadtemplates_lib  # Link to scadtemplates library for TemplateParser
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator::validator
        GTest::gtest
        GTest::gtest_main
    )

    target_include_directories(test_unit_g_templates_inventory PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/jsonwriter/jsonwriter-portable/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>
    )
    
    target_compile_definitions(test_unit_g_templates_inventory PRIVATE
        PLATFORMINFO_EXPORTS
        SCADTEMPLATES_EXPORTS
        RESOURCESCANNING_EXPORTS
    )

    target_link_libraries(test_unit_g_resource_scanner PRIVATE
        scadtemplates_lib
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        GTest::gtest
        GTest::gtest_main
    )

    target_include_directories(test_unit_g_resource_scanner PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests>
    )

    # Copy Qt platform plugins to app output directory
    if(WIN32)
        add_custom_command(TARGET scadtemplates_app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${Qt6_DIR}/../../../plugins/platforms"
            "$<TARGET_FILE_DIR:scadtemplates_app>/platforms"
            COMMENT "Copying Qt platform plugins..."
        )

        # Copy Qt DLLs to app output directory
        add_custom_command(TARGET scadtemplates_app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:scadtemplates_app>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:scadtemplates_app>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Widgets> $<TARGET_FILE_DIR:scadtemplates_app>
            COMMENT "Copying Qt DLLs to app directory..."
        )
    endif()
endif()

# GoogleTest integration
if(BUILD_TESTS)
    enable_testing()

    # Fetch GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Work around MSVC 14.44 internal compiler errors in gtest/gmock amalgamated sources
    # Disable optimization on these third-party translation units to avoid ICEs.
    if(MSVC)
        foreach(t gmock gmock_main gtest gtest_main)
            if(TARGET ${t})
                target_compile_options(${t} PRIVATE /Od /d2SSAOptimizer-)
            endif()
        endforeach()
    endif()

    include(GoogleTest)

    # Test executable
    add_executable(scadtemplates_tests EXCLUDE_FROM_ALL
        tests/test_main.cpp
        tests/test_unit_g_editsubtype.cpp
        tests/test_unit_g_edittype.cpp
        tests/test_unit_g_resource_metadata.cpp
        tests/test_unit_g_resourcelocation.cpp
    )

    target_link_libraries(scadtemplates_tests PRIVATE
        scadtemplates_lib
        ${QT_LIBRARIES}
        GTest::gtest
        GTest::gtest_main
    )

    # Ensure GTest built shared when desired (already in your PR maybe).
    # After target_link_libraries(cppsnippets_tests PRIVATE GTest::gtest_main ...)
    if(WIN32 AND BUILD_SHARED_LIBS)
        # Copy GTest, Qt, and project library DLLs next to the test executable so the Windows loader finds them
        add_custom_command(TARGET scadtemplates_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime DLLs to test output dir..."
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GTest::gtest> $<TARGET_FILE_DIR:scadtemplates_tests>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GTest::gtest_main> $<TARGET_FILE_DIR:scadtemplates_tests>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:scadtemplates_lib> $<TARGET_FILE_DIR:scadtemplates_tests>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:scadtemplates_tests>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:scadtemplates_tests>
            COMMENT "Copy GTest, Qt, and project DLLs to test binary directory")
        
        # Copy Qt platform plugins for tests
        add_custom_command(TARGET scadtemplates_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${Qt6_DIR}/../../../plugins/platforms"
            "$<TARGET_FILE_DIR:scadtemplates_tests>/platforms"
            COMMENT "Copying Qt platform plugins for tests...")
    endif()

    # Discover tests for CTest
    # Use PRE_TEST discovery mode to defer test discovery until ctest runs,
    # avoiding DLL loading issues during build when using Ninja generator
    gtest_discover_tests(scadtemplates_tests
        DISCOVERY_MODE PRE_TEST
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# Convenience targets for building tests
if(BUILD_TESTS)
    # Build all unit tests (Google Test suite)
    add_custom_target(build-all-unit-tests
        DEPENDS scadtemplates_tests
        COMMENT "Building all unit tests (Google Test suite)"
    )

    # Build all standalone test utilities
    add_custom_target(build-all-standalone-tests
        DEPENDS test_env_expansion test_path_discovery test_tier_standalone settings_generator
        COMMENT "Building all standalone test utilities"
    )

    # Build everything test-related
    add_custom_target(build-all-tests
        DEPENDS scadtemplates_tests test_env_expansion test_path_discovery test_tier_standalone settings_generator
        COMMENT "Building all tests and utilities"
    )
endif()

# NOTE: To run all unit tests, use:
#   cd build && ctest -C Debug --output-on-failure
# Or run the test executable directly:
#   cd build && .\bin\Debug\scadtemplates_tests.exe

# Installation rules
include(GNUInstallDirs)

install(TARGETS scadtemplates_lib
    EXPORT scadtemplates-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY src/scadtemplates/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scadtemplates
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

if(BUILD_APP)
    install(TARGETS scadtemplates_app
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Export targets for find_package support
install(EXPORT scadtemplates-targets
    FILE scadtemplates-targets.cmake
    NAMESPACE scadtemplates::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scadtemplates
)

# Generate package config
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/scadtemplates-config.cmake.in
    ${CMAKE_BINARY_DIR}/scadtemplates-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scadtemplates
)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/scadtemplates-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_BINARY_DIR}/scadtemplates-config.cmake
    ${CMAKE_BINARY_DIR}/scadtemplates-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scadtemplates
)
