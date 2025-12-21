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
option(BUILD_TESTS "Build test suite" ON)
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

# JsonReaderPortable integration (static library for JSON file reading with error reporting)
add_subdirectory(src/jsonreader-portable)

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
include_directories(${CMAKE_SOURCE_DIR}/src/include)

# Generate version source file from template
configure_file(
    ${CMAKE_SOURCE_DIR}/src/scadtemplates_version.cpp.in
    ${CMAKE_BINARY_DIR}/generated/scadtemplates_version.cpp
    @ONLY
)

# Library source files
set(LIB_SOURCES
    ${CMAKE_BINARY_DIR}/generated/scadtemplates_version.cpp
    src/template.cpp
    src/template_parser.cpp
    src/template_manager.cpp
    src/legacy_template_converter.cpp
    src/scad_template_session.cpp
    src/editsubtype.cpp
    src/edittype.cpp
    src/filesubtype.cpp
    src/extnOSVersRef.cpp
    src/platformInfo.cpp
    src/resourcePaths.cpp
    src/resourceLocationManager.cpp
)

set(LIB_HEADERS
    src/include/scadtemplates/scadtemplates.h
    src/include/scadtemplates/template.h
    src/include/scadtemplates/template_parser.h
    src/include/scadtemplates/template_manager.h
    src/include/scadtemplates/scad_template_session.h
    src/include/scadtemplates/export.h
    src/include/scadtemplates/editsubtype.h
    src/include/scadtemplates/edittype.h
    src/include/scadtemplates/filesubtype.h
    src/include/platformInfo/export.h
    src/include/platformInfo/extnOSVersRef.h
    src/include/platformInfo/platformInfo.h
    src/include/platformInfo/resourcePaths.h
    src/include/platformInfo/resourceLocationManager.h
)

# Build the shared/dynamic library
add_library(scadtemplates_lib ${LIB_SOURCES} ${LIB_HEADERS})
target_include_directories(scadtemplates_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    $<INSTALL_INTERFACE:include>
)

# Link Qt6::Core and Qt6::Gui for QString, QVector, QSysInfo, QScreen support in library
target_link_libraries(scadtemplates_lib PUBLIC Qt6::Core Qt6::Gui JsonReaderPortable)

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
        src/include/gui/preferencesdialog.h
        src/gui/resourcelocationwidget.cpp
        src/include/gui/resourceLocationWidget.hpp
        src/gui/locationInputWidget.cpp
        src/include/gui/locationInputWidget.hpp
        src/gui/platformInfoWidget.cpp
        src/include/gui/platformInfoWidget.hpp
        src/gui/dialogButtonBar.cpp
        src/include/gui/dialogButtonBar.hpp
        src/gui/installationTab.cpp
        src/include/gui/installationTab.hpp
        src/gui/machineTab.cpp
        src/include/gui/machineTab.hpp
        src/gui/userTab.cpp
        src/include/gui/userTab.hpp
        src/gui/aboutDialog.cpp
        src/include/gui/aboutDialog.h
        src/gui/sysInfoDialog.cpp
        src/include/gui/sysInfoDialog.h
        # Resource inventory classes
        src/resInventory/resourceItem.cpp
        src/include/resInventory/resourceItem.h
        src/resInventory/resourceTreeWidget.cpp
        src/include/resInventory/resourceTreeWidget.h
        src/resInventory/resourceScanner.cpp
        src/include/resInventory/resourceScanner.h
        # New resource management classes
        src/resInventory/resourceStore.cpp
        src/include/resInventory/resourceStore.h
        src/resInventory/resourceScannerDirListing.cpp
        src/include/resInventory/resourceScannerDirListing.h
        src/resInventory/templateTreeModel.cpp
        src/include/resInventory/templateTreeModel.h
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
        src/include/gui/machineTab.hpp
        src/gui/userTab.cpp
        src/include/gui/userTab.hpp
        src/gui/resourcelocationwidget.cpp
        src/include/gui/resourceLocationWidget.hpp
        src/gui/locationInputWidget.cpp
        src/include/gui/locationInputWidget.hpp
        # Resource inventory classes
        src/resInventory/resourceItem.cpp
        src/include/resInventory/resourceItem.h
        src/resInventory/resourceTreeWidget.cpp
        src/include/resInventory/resourceTreeWidget.h
        src/resInventory/resourceScanner.cpp
        src/include/resInventory/resourceScanner.h
    )
    
    add_executable(inventory_test ${INVENTORY_TEST_SOURCES})
    target_link_libraries(inventory_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(inventory_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    )
    set_target_properties(inventory_test PROPERTIES
        OUTPUT_NAME inventory_test
        WIN32_EXECUTABLE OFF  # Console app, not GUI
    )
    
    # Template converter test app
    add_executable(convert_templates_test src/app/convert_templates_test.cpp)
    target_link_libraries(convert_templates_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(convert_templates_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    )
    set_target_properties(convert_templates_test PROPERTIES
        OUTPUT_NAME convert_templates_test
        WIN32_EXECUTABLE OFF
    )

    # Template discovery test app
    add_executable(template_discovery_test src/app/template_discovery_test.cpp)
    target_link_libraries(template_discovery_test PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(template_discovery_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    )
    set_target_properties(template_discovery_test PROPERTIES
        OUTPUT_NAME template_discovery_test
        WIN32_EXECUTABLE OFF
    )

    # Template tree demo app (GUI)
    add_executable(template_tree_demo
        src/app/template_tree_demo.cpp
        # Resource model and store/scanner sources
        src/resInventory/templateTreeModel.cpp
        src/include/resInventory/templateTreeModel.h
        src/resInventory/resourceStore.cpp
        src/include/resInventory/resourceStore.h
        src/resInventory/resourceScannerDirListing.cpp
        src/include/resInventory/resourceScannerDirListing.h
    )
    target_link_libraries(template_tree_demo PRIVATE scadtemplates_lib ${QT_LIBRARIES})
    target_include_directories(template_tree_demo PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    )
    set_target_properties(template_tree_demo PROPERTIES
        OUTPUT_NAME template_tree_demo
        WIN32_EXECUTABLE ON
    )

    # Library discovery test app
    add_executable(library_discovery_test
        src/app/library_discovery_test.cpp
        src/resInventory/resourceScanner.cpp
        src/resInventory/resourceItem.cpp
        src/resInventory/resourceTreeWidget.cpp
        src/include/resInventory/resourceScanner.h
        src/include/resInventory/resourceItem.h
        src/include/resInventory/resourceTreeWidget.h
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

        # Also copy plugins for template_tree_demo
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
    add_executable(scadtemplates_tests
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
        src/include/resInventory/resourceScanner.h
        src/include/resInventory/resourceItem.h
        src/include/resInventory/resourceTreeWidget.h
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
    )
    
    # =========================================================================
    # Qt Test - New scanner tests using Qt Test framework
    # =========================================================================
    find_package(Qt6 COMPONENTS Test QUIET)
    if(Qt6Test_FOUND)
        message(STATUS "Found Qt6::Test - building Qt Test suite")
        
        # Qt Test for QDirListing scanner
        add_executable(test_resourcescannerdirlisting
            tests/qt/test_resourcescannerdirlisting.cc
            src/resInventory/resourceScannerDirListing.cpp
            src/include/resInventory/resourceScannerDirListing.h
        )
        
        target_link_libraries(test_resourcescannerdirlisting PRIVATE
            scadtemplates_lib
            ${QT_LIBRARIES}
            Qt6::Test
        )
        
        target_include_directories(test_resourcescannerdirlisting PRIVATE
            ${CMAKE_SOURCE_DIR}/src/include
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
        add_executable(test_resourcestore
            tests/qt/test_resourcestore.cc
            src/resInventory/resourceStore.cpp
            src/include/resInventory/resourceStore.h
            src/resInventory/resourceScannerDirListing.cpp
            src/include/resInventory/resourceScannerDirListing.h
        )
        
        target_link_libraries(test_resourcestore PRIVATE
            scadtemplates_lib
            ${QT_LIBRARIES}
            Qt6::Test
        )
        
        target_include_directories(test_resourcestore PRIVATE
            ${CMAKE_SOURCE_DIR}/src/include
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
        add_executable(test_templatetreemodel
            tests/qt/test_templatetreemodel.cc
            src/resInventory/templateTreeModel.cpp
            src/include/resInventory/templateTreeModel.h
            src/resInventory/resourceStore.cpp
            src/include/resInventory/resourceStore.h
            src/resInventory/resourceScannerDirListing.cpp
            src/include/resInventory/resourceScannerDirListing.h
        )
        
        target_link_libraries(test_templatetreemodel PRIVATE
            scadtemplates_lib
            ${QT_LIBRARIES}
            Qt6::Test
        )
        
        target_include_directories(test_templatetreemodel PRIVATE
            ${CMAKE_SOURCE_DIR}/src/include
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
    else()
        message(STATUS "Qt6::Test not found - Qt Test suite will not be built")
    endif()
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
