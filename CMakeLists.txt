cmake_minimum_required(VERSION 3.16)

# Job pool configuration to prevent system crashes
# Limit compilation to -j4 and linking to -j2 to reduce memory pressure
# AGENT NOTE: These limits prevent memory exhaustion. Do NOT override with --parallel 8.
# Use: cmake --build . (respects job pool defaults)
# See DEVELOPMENT.md section 1 for details.
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

# Version: MAJOR.MINOR.PATCH
# - MAJOR: Manual (breaking changes)
# - MINOR: Auto-incremented from Git commit count  
# - PATCH: Manual (features/fixes within a release)
set(SCADTEMPLATES_VERSION_MAJOR 2)
set(SCADTEMPLATES_VERSION_PATCH 0)

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
set(SCADTEMPLATES_VERSION_MINOR ${GIT_COMMIT_COUNT})

# Compose full version string
set(SCADTEMPLATES_VERSION "${SCADTEMPLATES_VERSION_MAJOR}.${SCADTEMPLATES_VERSION_MINOR}.${SCADTEMPLATES_VERSION_PATCH}")

project(scadtemplates
  VERSION ${SCADTEMPLATES_VERSION}
  DESCRIPTION "A code template solution for OpenSCAD"
  HOMEPAGE_URL "https://jartisan.com/"
  LANGUAGES CXX
)

message(STATUS "ScadTemplates version: ${SCADTEMPLATES_VERSION} (Git commits: ${GIT_COMMIT_COUNT})")

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
        set(QT6_DEFAULT_PATH "C:/bin/Qt/6.10.1/msvc2022_64")
    elseif(MINGW)
        set(QT6_DEFAULT_PATH "C:/bin/Qt/6.10.1/mingw_64")
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

# JsonReaderPortable integration (static library for JSON file reading with error reporting)
add_subdirectory(src/jsonreader/jsonreader-portable)

# JsonWriterPortable integration (static library for JSON file writing with error reporting)
add_subdirectory(src/jsonwriter/jsonwriter-portable)

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

# Generate version source file from template
configure_file(
    ${CMAKE_SOURCE_DIR}/src/scadtemplates_version.cpp.in
    ${CMAKE_BINARY_DIR}/generated/scadtemplates_version.cpp
    @ONLY
)

# ============================================================================
# resourceMgmt_lib - Platform info and resource inventory management
# This library is designed to be portable and dropped into OpenSCAD
# ============================================================================
set(RESOURCEMGMT_SOURCES
    # platformInfo - OS detection, path resolution, defaults
    src/platformInfo/extnOSVersRef.cpp
    src/platformInfo/platformInfo.cpp
    src/platformInfo/resourcePaths.cpp
    src/platformInfo/resourceLocationManager.cpp
    # resInventory - location storage, scanning, UI widgets
    src/resInventory/ResourceLocation.cpp
    src/resInventory/resLocMap.cpp
    src/resInventory/resLocTree.cpp
    src/resInventory/resourceItem.cpp
    src/resInventory/resourceIterator.cpp
    src/resInventory/resourceScanner.cpp
    src/resInventory/resourceScannerDirListing.cpp
    src/resInventory/resourceStore.cpp
    src/resInventory/resourceTreeWidget.cpp
    src/resInventory/templateTreeModel.cpp
)

set(RESOURCEMGMT_HEADERS
    # platformInfo headers
    src/platformInfo/export.h
    src/platformInfo/extnOSVersRef.h
    src/platformInfo/platformInfo.h
    src/platformInfo/resourcePaths.h
    src/platformInfo/resourceLocationManager.h
    # resInventory headers
    src/resInventory/ResourceLocation.h
    src/resInventory/resLocMap.h
    src/resInventory/resLocTree.h
    src/resInventory/resourceItem.h
    src/resInventory/resourceIterator.h
    src/resInventory/resourceScanner.h
    src/resInventory/resourceScannerDirListing.h
    src/resInventory/resourceStore.h
    src/resInventory/resourceTreeWidget.h
    src/resInventory/templateTreeModel.h
)

add_library(resourceMgmt_lib ${RESOURCEMGMT_SOURCES} ${RESOURCEMGMT_HEADERS})
target_include_directories(resourceMgmt_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(resourceMgmt_lib PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(resourceMgmt_lib PRIVATE RESOURCEMGMT_EXPORTS)
endif()

set_target_properties(resourceMgmt_lib PROPERTIES
    OUTPUT_NAME resourceMgmt
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ============================================================================
# scadtemplates_lib - Template management for the GUI app
# ============================================================================
set(LIB_SOURCES
    ${CMAKE_BINARY_DIR}/generated/scadtemplates_version.cpp
    src/scadtemplates/template.cpp
    src/scadtemplates/template_parser.cpp
    src/scadtemplates/template_manager.cpp
    src/scadtemplates/legacy_template_converter.cpp
    src/scadtemplates/scad_template_session.cpp
    src/scadtemplates/editsubtype.cpp
    src/scadtemplates/edittype.cpp
    src/scadtemplates/filesubtype.cpp
)

set(LIB_HEADERS
    src/scadtemplates/scadtemplates.h
    src/scadtemplates/template.h
    src/scadtemplates/template_parser.h
    src/scadtemplates/template_manager.h
    src/scadtemplates/scad_template_session.h
    src/scadtemplates/export.h
    src/scadtemplates/editsubtype.h
    src/scadtemplates/edittype.h
    src/scadtemplates/filesubtype.h
)

# Build the shared/dynamic library
add_library(scadtemplates_lib ${LIB_SOURCES} ${LIB_HEADERS})
target_include_directories(scadtemplates_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Link to resourceMgmt_lib and other dependencies
# Qt6::Gui moved to PRIVATE - headers don't expose GUI types (only std::string)
target_link_libraries(scadtemplates_lib 
    PUBLIC Qt6::Core JsonReaderPortable resourceMgmt_lib
    PRIVATE Qt6::Gui
)

# Set library properties for Windows DLL export
if(BUILD_SHARED_LIBS)
    target_compile_definitions(scadtemplates_lib PRIVATE SCADTEMPLATES_EXPORTS)
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
        src/gui/preferencesdialog.h
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
        src/gui/envVarsTab.cpp
        src/gui/envVarsTab.h
        src/gui/aboutDialog.cpp
        src/gui/aboutDialog.h
        src/gui/sysInfoDialog.cpp
        src/gui/sysInfoDialog.h
        # resInventory classes are now in resourceMgmt_lib (linked via scadtemplates_lib)
    )
    
    add_executable(scadtemplates_app ${APP_SOURCES})
    target_link_libraries(scadtemplates_app PRIVATE 
        scadtemplates_lib 
        ${QT_LIBRARIES}
        JsonReaderPortable
        JsonWriterPortable
    )
    target_include_directories(scadtemplates_app PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
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
        # resInventory classes are now in resourceMgmt_lib (linked via scadtemplates_lib)
    )
    
    add_executable(inventory_test EXCLUDE_FROM_ALL ${INVENTORY_TEST_SOURCES})
    target_link_libraries(inventory_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(inventory_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    )
    set_target_properties(inventory_test PROPERTIES
        OUTPUT_NAME inventory_test
        WIN32_EXECUTABLE OFF  # Console app, not GUI
    )
    
    # Template converter test app
    add_executable(convert_templates_test EXCLUDE_FROM_ALL src/app/convert_templates_test.cpp)
    target_link_libraries(convert_templates_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(convert_templates_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    )
    set_target_properties(convert_templates_test PROPERTIES
        OUTPUT_NAME convert_templates_test
        WIN32_EXECUTABLE OFF
    )

    # Template discovery test app
    add_executable(template_discovery_test EXCLUDE_FROM_ALL src/app/template_discovery_test.cpp)
    target_link_libraries(template_discovery_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(template_discovery_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    )
    set_target_properties(template_discovery_test PROPERTIES
        OUTPUT_NAME template_discovery_test
        WIN32_EXECUTABLE OFF
    )

    # Template tree demo app (GUI)
    add_executable(template_tree_demo EXCLUDE_FROM_ALL
        src/app/template_tree_demo.cpp
        # resInventory classes are now in resourceMgmt_lib (linked via scadtemplates_lib)
    )
    target_link_libraries(template_tree_demo PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(template_tree_demo PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    )
    set_target_properties(template_tree_demo PROPERTIES
        OUTPUT_NAME template_tree_demo
        WIN32_EXECUTABLE ON
    )

    # Library discovery test app
    add_executable(library_discovery_test EXCLUDE_FROM_ALL
        src/app/library_discovery_test.cpp
        # resInventory classes are now in resourceMgmt_lib (linked via scadtemplates_lib)
    )
    target_link_libraries(library_discovery_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(library_discovery_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
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

        # Also copy plugins for template_tree_demo when built
        add_custom_command(TARGET template_tree_demo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${Qt6_DIR}/../../../plugins/platforms"
                "$<TARGET_FILE_DIR:template_tree_demo>/platforms"
            COMMENT "Copying Qt platform plugins for template_tree_demo..."
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

    # Exclude GoogleTest targets from default ALL build - only build when tests target is built
    foreach(t gmock gmock_main gtest gtest_main)
        if(TARGET ${t})
            set_target_properties(${t} PROPERTIES EXCLUDE_FROM_ALL TRUE)
        endif()
    endforeach()

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
        tests/test_resource_paths.cpp
        tests/test_resource_location_manager.cpp
        tests/qt/test_resource_discovery.cc
        # resInventory classes are now in resourceMgmt_lib (linked via scadtemplates_lib)
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
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:resourceMgmt_lib> $<TARGET_FILE_DIR:scadtemplates_tests>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:scadtemplates_tests>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:scadtemplates_tests>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Widgets> $<TARGET_FILE_DIR:scadtemplates_tests>
        COMMENT "Copy GTest, Qt, and project DLLs to test binary directory")
    endif()

    # Discover tests for CTest
    # Use PRE_TEST discovery mode to defer test discovery until ctest runs,
    # avoiding DLL loading issues during build when using Ninja generator
    gtest_discover_tests(scadtemplates_tests
        DISCOVERY_MODE PRE_TEST
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    
    # =========================================================================
    # Qt Test - New scanner tests using Qt Test framework
    # =========================================================================
    find_package(Qt6 COMPONENTS Test QUIET)
    if(Qt6Test_FOUND)
        message(STATUS "Found Qt6::Test - building Qt Test suite")
        
        # Qt Test for QDirListing scanner
        add_executable(test_resourcescannerdirlisting EXCLUDE_FROM_ALL
            tests/qt/test_resourcescannerdirlisting.cc
            # resInventory classes are now in resourceMgmt_lib (linked via scadtemplates_lib)
        )
        
        target_link_libraries(test_resourcescannerdirlisting PRIVATE
            scadtemplates_lib
            ${QT_LIBRARIES}
            Qt6::Test
        )
        
        target_include_directories(test_resourcescannerdirlisting PRIVATE
            ${CMAKE_SOURCE_DIR}/src
        )
        
        # Define SRCDIR for test to find testFileStructure
        target_compile_definitions(test_resourcescannerdirlisting PRIVATE
            SRCDIR="${CMAKE_SOURCE_DIR}"
        )
        
        # Copy Qt DLLs for test executable
        if(WIN32 AND BUILD_SHARED_LIBS)
            add_custom_command(TARGET test_resourcescannerdirlisting POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:test_resourcescannerdirlisting>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Test> $<TARGET_FILE_DIR:test_resourcescannerdirlisting>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:scadtemplates_lib> $<TARGET_FILE_DIR:test_resourcescannerdirlisting>
                COMMENT "Copy Qt and project DLLs for Qt Test executable")
        endif()
        
        # Register with CTest
        add_test(NAME test_resourcescannerdirlisting 
                 COMMAND test_resourcescannerdirlisting)
        
        set_tests_properties(test_resourcescannerdirlisting PROPERTIES
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        
        # Qt Test for ResourceStore
        add_executable(test_resourcestore EXCLUDE_FROM_ALL
            tests/qt/test_resourcestore.cc
            # resInventory classes are now in resourceMgmt_lib (linked via scadtemplates_lib)
        )
        
        target_link_libraries(test_resourcestore PRIVATE
            scadtemplates_lib
            ${QT_LIBRARIES}
            Qt6::Test
        )
        
        target_include_directories(test_resourcestore PRIVATE
            ${CMAKE_SOURCE_DIR}/src
        )
        
        # Define SRCDIR for test to find testFileStructure
        target_compile_definitions(test_resourcestore PRIVATE
            SRCDIR="${CMAKE_SOURCE_DIR}"
        )
        
        # Copy Qt DLLs for test executable
        if(WIN32 AND BUILD_SHARED_LIBS)
            add_custom_command(TARGET test_resourcestore POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:test_resourcestore>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Test> $<TARGET_FILE_DIR:test_resourcestore>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:scadtemplates_lib> $<TARGET_FILE_DIR:test_resourcestore>
                COMMENT "Copy Qt and project DLLs for test_resourcestore")
        endif()
        
        # Register with CTest
        add_test(NAME test_resourcestore 
                 COMMAND test_resourcestore)
        
        set_tests_properties(test_resourcestore PROPERTIES
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        
        # Qt Test for TemplateTreeModel
        add_executable(test_templatetreemodel EXCLUDE_FROM_ALL
            tests/qt/test_templatetreemodel.cc
            # resInventory classes are now in resourceMgmt_lib (linked via scadtemplates_lib)
        )
        
        target_link_libraries(test_templatetreemodel PRIVATE
            scadtemplates_lib
            ${QT_LIBRARIES}
            Qt6::Test
        )
        
        target_include_directories(test_templatetreemodel PRIVATE
            ${CMAKE_SOURCE_DIR}/src
        )
        
        # Define SRCDIR for test to find testFileStructure
        target_compile_definitions(test_templatetreemodel PRIVATE
            SRCDIR="${CMAKE_SOURCE_DIR}"
        )
        
        # Copy Qt DLLs for test executable
        if(WIN32 AND BUILD_SHARED_LIBS)
            add_custom_command(TARGET test_templatetreemodel POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:test_templatetreemodel>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Test> $<TARGET_FILE_DIR:test_templatetreemodel>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:scadtemplates_lib> $<TARGET_FILE_DIR:test_templatetreemodel>
                COMMENT "Copy Qt and project DLLs for test_templatetreemodel")
        endif()
        
        # Register with CTest
        add_test(NAME test_templatetreemodel 
                 COMMAND test_templatetreemodel)
        
        set_tests_properties(test_templatetreemodel PROPERTIES
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        
        # GUI Test for MainWindow
        add_executable(test_mainwindow_gui EXCLUDE_FROM_ALL
            tests/qt/test_mainwindow_gui.cpp
            tests/qt/test_mainwindow_gui.h
            src/app/mainwindow.cpp
            src/app/mainwindow.h
            src/gui/preferencesdialog.cpp
            src/gui/preferencesdialog.h
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
            src/gui/envVarsTab.cpp
            src/gui/envVarsTab.h
            src/gui/aboutDialog.cpp
            src/gui/aboutDialog.h
            src/gui/sysInfoDialog.cpp
            src/gui/sysInfoDialog.h
        )
        
        target_link_libraries(test_mainwindow_gui PRIVATE
            scadtemplates_lib
            resourceMgmt_lib
            JsonReaderPortable
            ${QT_LIBRARIES}
            Qt6::Test
        )
        
        target_include_directories(test_mainwindow_gui PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/app
            ${CMAKE_SOURCE_DIR}/src/gui
            ${CMAKE_SOURCE_DIR}/src/resInventory
        )
        
        target_compile_definitions(test_mainwindow_gui PRIVATE
            SRCDIR="${CMAKE_SOURCE_DIR}"
        )
        
        if(WIN32 AND BUILD_SHARED_LIBS)
            add_custom_command(TARGET test_mainwindow_gui POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:test_mainwindow_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Test> $<TARGET_FILE_DIR:test_mainwindow_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:test_mainwindow_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Widgets> $<TARGET_FILE_DIR:test_mainwindow_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:scadtemplates_lib> $<TARGET_FILE_DIR:test_mainwindow_gui>
                COMMENT "Copy Qt and project DLLs for test_mainwindow_gui")
        endif()
        
        add_test(NAME test_mainwindow_gui 
                 COMMAND test_mainwindow_gui)
        
        set_tests_properties(test_mainwindow_gui PROPERTIES
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            LABELS "GUI"
        )
        
        # GUI Test for EnvVarsTab
        add_executable(test_envvarstab_gui EXCLUDE_FROM_ALL
            tests/qt/test_envvarstab_gui_simple.cpp
            tests/qt/test_envvarstab_gui_simple.h
            src/gui/envVarsTab.cpp
            src/gui/envVarsTab.h
            src/gui/dialogButtonBar.cpp
            src/gui/dialogButtonBar.hpp
        )
        
        target_link_libraries(test_envvarstab_gui PRIVATE
            scadtemplates_lib
            resourceMgmt_lib
            JsonReaderPortable
            ${QT_LIBRARIES}
            Qt6::Test
        )
        
        target_include_directories(test_envvarstab_gui PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/gui
            ${CMAKE_SOURCE_DIR}/src/resInventory
        )
        
        target_compile_definitions(test_envvarstab_gui PRIVATE
            SRCDIR="${CMAKE_SOURCE_DIR}"
        )
        
        if(WIN32 AND BUILD_SHARED_LIBS)
            add_custom_command(TARGET test_envvarstab_gui POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:test_envvarstab_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Test> $<TARGET_FILE_DIR:test_envvarstab_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:test_envvarstab_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Widgets> $<TARGET_FILE_DIR:test_envvarstab_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:scadtemplates_lib> $<TARGET_FILE_DIR:test_envvarstab_gui>
                COMMENT "Copy Qt and project DLLs for test_envvarstab_gui")
        endif()
        
        add_test(NAME test_envvarstab_gui 
                 COMMAND test_envvarstab_gui)
        
        set_tests_properties(test_envvarstab_gui PROPERTIES
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            LABELS "GUI"
        )
        
        # GUI Test for ResourceLocationWidget
        # Simplified - verifies test infrastructure works
        add_executable(test_resourcelocationwidget_gui EXCLUDE_FROM_ALL
            tests/qt/test_resourcelocationwidget_gui_simple.cpp
            tests/qt/test_resourcelocationwidget_gui_simple.h
        )
        
        target_link_libraries(test_resourcelocationwidget_gui PRIVATE
            scadtemplates_lib
            resourceMgmt_lib
            JsonReaderPortable
            ${QT_LIBRARIES}
            Qt6::Test
        )
        
        target_include_directories(test_resourcelocationwidget_gui PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/gui
            ${CMAKE_SOURCE_DIR}/src/resInventory
        )
        
        target_compile_definitions(test_resourcelocationwidget_gui PRIVATE
            SRCDIR="${CMAKE_SOURCE_DIR}"
        )
        
        if(WIN32 AND BUILD_SHARED_LIBS)
            add_custom_command(TARGET test_resourcelocationwidget_gui POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:test_resourcelocationwidget_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Test> $<TARGET_FILE_DIR:test_resourcelocationwidget_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:test_resourcelocationwidget_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Widgets> $<TARGET_FILE_DIR:test_resourcelocationwidget_gui>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:scadtemplates_lib> $<TARGET_FILE_DIR:test_resourcelocationwidget_gui>
                COMMENT "Copy Qt and project DLLs for test_resourcelocationwidget_gui")
        endif()
        
        add_test(NAME test_resourcelocationwidget_gui 
                 COMMAND test_resourcelocationwidget_gui)
        
        set_tests_properties(test_resourcelocationwidget_gui PROPERTIES
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            LABELS "GUI"
        )

        # JsonReader file tests now live in subproject (jsonreader_files_qttest)
    else()
        message(STATUS "Qt6::Test not found - Qt Test suite will not be built")
    endif()
endif()

# Aggregate custom targets for convenience
if(TARGET template_tree_demo OR TARGET convert_templates_test OR TARGET template_discovery_test)
    add_custom_target(demos ALL
        DEPENDS
            template_tree_demo
            convert_templates_test
            template_discovery_test
    )
    # Note: EXCLUDE_FROM_ALL prevents these from building unless explicitly requested
    set_target_properties(demos PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

if(BUILD_TESTS)
    add_custom_target(tests
        DEPENDS
            scadtemplates_tests
            inventory_test
            library_discovery_test
            convert_templates_test
            template_discovery_test
            test_resourcescannerdirlisting
            test_resourcestore
            test_templatetreemodel
            test_mainwindow_gui
            test_envvarstab_gui
            test_resourcelocationwidget_gui
    )
    set_target_properties(tests PROPERTIES EXCLUDE_FROM_ALL TRUE)

    # Convenience targets to run subsets of tests via CTest
    add_custom_target(non_gui_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} -C $<CONFIG> -LE "^GUI$" --output-on-failure
        USES_TERMINAL
    )
    # Only depend on scadtemplates_tests (GTest suite) - CTest filtering handles Qt Test executables
    add_dependencies(non_gui_tests
        scadtemplates_tests
    )
    set_target_properties(non_gui_tests PROPERTIES EXCLUDE_FROM_ALL TRUE)

    add_custom_target(gui_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} -C $<CONFIG> -L gui --output-on-failure
        USES_TERMINAL
    )
    add_dependencies(gui_tests
        test_mainwindow_gui
        test_envvarstab_gui
        test_resourcelocationwidget_gui
    )
    set_target_properties(gui_tests PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

# Installation rules
include(GNUInstallDirs)

# Install resourceMgmt_lib (the combined platformInfo + resInventory library)
install(TARGETS resourceMgmt_lib
    EXPORT scadtemplates-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY src/platformInfo src/resInventory
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(TARGETS scadtemplates_lib
    EXPORT scadtemplates-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY src/scadtemplates
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
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
