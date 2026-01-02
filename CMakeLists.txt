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
#`target_link_libraries()` allows use with targets in other directories.
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
  #  in future for compatibiltiy with OpenSCAD
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

# Get Git commit count for auto-incrementing minor version
find_package(Git QUIET)
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
else()
    set(GIT_COMMIT_COUNT 0)
endif()
set(APP_VERSION_MINOR ${GIT_COMMIT_COUNT})

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

# Legacy compatibility - keep SCADTEMPLATES_* variables
set(SCADTEMPLATES_VERSION_MAJOR ${APP_VERSION_MAJOR})
set(SCADTEMPLATES_VERSION_MINOR ${APP_VERSION_MINOR})
set(SCADTEMPLATES_VERSION_PATCH ${APP_VERSION_PATCH})
set(SCADTEMPLATES_VERSION ${APP_VERSION})

project(scadtemplates
  VERSION ${APP_VERSION}
  DESCRIPTION "A code template solution for OpenSCAD"
  HOMEPAGE_URL "https://jartisan.com/"
  LANGUAGES CXX
)

message(STATUS "${APP_DISPLAY_NAME} version: ${APP_VERSION} (Git commits: ${GIT_COMMIT_COUNT})")

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
option(BUILD_APP "Build Qt application" ON)
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
  #  (?!ext/) becomes ([^e]...|e[^x]..|ex[^t].|ext[^/]|.{0,3}$)
  # CMAKE_CXX_CLANG_TIDY must be set before target is created (with add_executable())
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--header-filter=${regex}([^e]...|e[^x]..|ex[^t].|ext[^/]|.{0,3}$)")
  #  append ";--fix" to arguments above to apply automatic fix (if the given checks support it)
endif(CLANG_TIDY)


# Include directories
# Note: src/include is removed - headers are now co-located with .cpp files
# Add src/ to include path so headers in src/resInventory, src/gui, src/platformInfo can be found
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/generated)

# Generate application metadata header from template
configure_file(
    ${CMAKE_SOURCE_DIR}/src/app/applicationNameInfo.hpp.in
    ${CMAKE_BINARY_DIR}/generated/applicationNameInfo.hpp
    @ONLY
)

# Generate version source file from template
configure_file(
    ${CMAKE_SOURCE_DIR}/src/scadtemplates_version.cpp.in
    ${CMAKE_BINARY_DIR}/generated/scadtemplates_version.cpp
    @ONLY
)

# Library source files
set(LIB_SOURCES
    ${CMAKE_BINARY_DIR}/generated/scadtemplates_version.cpp
    src/jsonreader/JsonReader.cpp
    src/scadtemplates/template.cpp
    src/scadtemplates/template_parser.cpp
    src/scadtemplates/template_manager.cpp
    src/scadtemplates/legacy_template_converter.cpp
    src/scadtemplates/scad_template_session.cpp
    src/scadtemplates/editsubtype.cpp
    src/scadtemplates/edittype.cpp
    src/scadtemplates/filesubtype.cpp
    src/platformInfo/platformInfo.cpp
    src/platformInfo/resourcePaths.cpp
    src/platformInfo/resourceLocationManager.cpp
)

set(LIB_HEADERS
    src/jsonreader/JsonReader.hpp
    src/scadtemplates/scadtemplates.hpp
    src/scadtemplates/template.hpp
    src/scadtemplates/template_parser.hpp
    src/scadtemplates/template_manager.hpp
    src/scadtemplates/scad_template_session.hpp
    src/scadtemplates/export.hpp
    src/scadtemplates/editsubtype.hpp
    src/scadtemplates/edittype.hpp
    src/scadtemplates/filesubtype.hpp
    src/platformInfo/export.hpp
    src/platformInfo/platformInfo.hpp
    src/platformInfo/resourcePaths.hpp
    src/platformInfo/resourceLocationManager.hpp
)

# Build the shared/dynamic library
add_library(scadtemplates_lib ${LIB_SOURCES} ${LIB_HEADERS})
target_include_directories(scadtemplates_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    $<INSTALL_INTERFACE:include>
)

# Link Qt6::Core and Qt6::Gui for QString, QVector, QSysInfo, QScreen support in library
# JsonReader is integrated directly into library, no separate dependency needed
target_link_libraries(scadtemplates_lib PUBLIC Qt6::Core Qt6::Gui)

# Set library properties for Windows DLL export
if(BUILD_SHARED_LIBS)
    target_compile_definitions(scadtemplates_lib PRIVATE SCADTEMPLATES_EXPORTS PLATFORMINFO_EXPORTS)
endif()

# Set output name and versioning
set_target_properties(scadtemplates_lib PROPERTIES
    OUTPUT_NAME scadtemplates
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Build Qt application
if(BUILD_APP)
    set(APP_SOURCES
        src/app/main.cpp
        src/app/mainwindow.cpp
        src/app/mainwindow.h
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
        # Resource inventory classes
        src/resInventory/resourceItem.cpp
        src/resInventory/resourceItem.hpp
        src/resInventory/resourceTreeWidget.cpp
        src/resInventory/resourceTreeWidget.hpp
        src/resInventory/resourceScanner.cpp
        src/resInventory/resourceScanner.hpp
    )
    
    add_executable(scadtemplates_app ${APP_SOURCES})
    target_link_libraries(scadtemplates_app PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(scadtemplates_app PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    )
    set_target_properties(scadtemplates_app PROPERTIES
        OUTPUT_NAME scadtemplates
        WIN32_EXECUTABLE ON
    )
    
    # Inventory test console app (non-GUI)
    set(INVENTORY_TEST_SOURCES
        src/app/inventory_test.cpp
        # GUI classes needed for env var helpers
        src/gui/machineTab.cpp
        src/gui/machineTab.hpp
        src/gui/userTab.cpp
        src/gui/userTab.hpp
        src/gui/resourcelocationwidget.cpp
        src/gui/resourceLocationWidget.hpp
        src/gui/locationInputWidget.cpp
        src/gui/locationInputWidget.hpp
        # Resource inventory classes
        src/resInventory/resourceItem.cpp
        src/resInventory/resourceItem.hpp
        src/resInventory/resourceTreeWidget.cpp
        src/resInventory/resourceTreeWidget.hpp
        src/resInventory/resourceScanner.cpp
        src/resInventory/resourceScanner.hpp
    )
    
    add_executable(inventory_test EXCLUDE_FROM_ALL ${INVENTORY_TEST_SOURCES})
    target_link_libraries(inventory_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(inventory_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    )
    set_target_properties(inventory_test PROPERTIES
        OUTPUT_NAME inventory_test
        WIN32_EXECUTABLE OFF  # Console app, not GUI
    )
    
    # Template converter test app
    add_executable(convert_templates_test EXCLUDE_FROM_ALL src/app/convert_templates_test.cpp)
    target_link_libraries(convert_templates_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(convert_templates_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    )
    set_target_properties(convert_templates_test PROPERTIES
        OUTPUT_NAME convert_templates_test
        WIN32_EXECUTABLE OFF
    )

    # Template discovery test app
    add_executable(template_discovery_test EXCLUDE_FROM_ALL src/app/template_discovery_test.cpp)
    target_link_libraries(template_discovery_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(template_discovery_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    )
    set_target_properties(template_discovery_test PROPERTIES
        OUTPUT_NAME template_discovery_test
        WIN32_EXECUTABLE OFF
    )

    # Library discovery test app
    add_executable(library_discovery_test EXCLUDE_FROM_ALL
        src/app/library_discovery_test.cpp
        src/resInventory/resourceScanner.cpp
        src/resInventory/resourceItem.cpp
        src/resInventory/resourceTreeWidget.cpp
        src/resInventory/resourceScanner.hpp
        src/resInventory/resourceItem.hpp
        src/resInventory/resourceTreeWidget.hpp
    )
    target_link_libraries(library_discovery_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(library_discovery_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    )
    set_target_properties(library_discovery_test PROPERTIES
        OUTPUT_NAME library_discovery_test
        WIN32_EXECUTABLE OFF
    )

    # Copy Qt platform plugins so the console app can find qwindows at runtime
    if(WIN32)
        add_custom_command(TARGET inventory_test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${Qt6_DIR}/../../../plugins/platforms"
                "$<TARGET_FILE_DIR:inventory_test>/platforms"
            COMMENT "Copying Qt platform plugins for inventory_test...")
    endif()
    
    # Copy Qt platform plugins to app output directory
    if(WIN32)
        add_custom_command(TARGET scadtemplates_app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${Qt6_DIR}/../../../plugins/platforms"
                "$<TARGET_FILE_DIR:scadtemplates_app>/platforms"
            COMMENT "Copying Qt platform plugins..."
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
        tests/test_template.cpp
        tests/test_template_parser.cpp
        tests/test_template_manager.cpp
        tests/test_scad_template_session.cpp
        tests/test_editsubtype.cpp
        tests/test_edittype.cpp
        tests/test_filesubtype.cpp
        tests/test_resource_discovery.cpp
        # Resource inventory sources needed by discovery tests
        src/resInventory/resourceScanner.cpp
        src/resInventory/resourceItem.cpp
        src/resInventory/resourceTreeWidget.cpp
        src/resInventory/resourceScanner.hpp
        src/resInventory/resourceItem.hpp
        src/resInventory/resourceTreeWidget.hpp
    )
    
    target_link_libraries(scadtemplates_tests PRIVATE
        scadtemplates_lib
        ${QT_LIBRARIES}
        GTest::gtest
        GTest::gtest_main
    )

    # Ensure GTest built shared when desired (already in your PR maybe).
    # After target_link_libraries(cppsnippets_tests PRIVATE GTest::gtest_main ...)

    if (WIN32 AND BUILD_SHARED_LIBS)
      # Copy GTest, Qt, and project library DLLs next to the test executable so the Windows loader finds them
      add_custom_command(TARGET scadtemplates_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime DLLs to test output dir..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GTest::gtest> $<TARGET_FILE_DIR:scadtemplates_tests>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GTest::gtest_main> $<TARGET_FILE_DIR:scadtemplates_tests>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:scadtemplates_lib> $<TARGET_FILE_DIR:scadtemplates_tests>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:scadtemplates_tests>
        COMMENT "Copy GTest, Qt, and project DLLs to test binary directory")
    endif()

    # Discover tests for CTest
    # Use PRE_TEST discovery mode to defer test discovery until ctest runs,
    # avoiding DLL loading issues during build when using Ninja generator
    gtest_discover_tests(scadtemplates_tests
        DISCOVERY_MODE PRE_TEST
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# Installation rules
include(GNUInstallDirs)

install(TARGETS scadtemplates_lib
    EXPORT scadtemplates-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY src/include/scadtemplates
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
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
