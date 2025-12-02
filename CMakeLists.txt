cmake_minimum_required(VERSION 3.16)

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

# project(cppsnippets VERSION 1.0.0 LANGUAGES CXX)
project(cppsnippets
  VERSION 1.0.0
  DESCRIPTION "A code snippet solution in C++"
  HOMEPAGE_URL "https://jartisan.com/"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(INFO "Display build configuration info at end of cmake config" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_APP "Build Qt application" ON)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Find Qt6 (no fallback to Qt5)
if(BUILD_APP)
    # User hint: set CMAKE_PREFIX_PATH to your Qt install, e.g.:
    # cmake -B build -DCMAKE_PREFIX_PATH="C:/bin/Qt/6.10.0/msvc2019_64"
    if(NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH "C:/bin/Qt/6.10.0/msvc2019_64" CACHE PATH "Path to Qt6 installation")
    endif()
    find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
    if(NOT Qt6_FOUND)
        message(FATAL_ERROR "Qt 6 not found! Please set CMAKE_PREFIX_PATH to your Qt installation.")
    else()
        set(DEPLOYMENT_TARGET "11.0")
        set(QT_VERSION 6)
        set(QT_LIBRARIES Qt6::Core Qt6::Widgets)
    endif()
    message(STATUS "Found Qt${QT_VERSION} at ${CMAKE_PREFIX_PATH}")
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
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

# Library source files
set(LIB_SOURCES
    src/snippets.cpp
    src/snippet.cpp
    src/snippet_parser.cpp
    src/snippet_manager.cpp
)

set(LIB_HEADERS
    src/include/cppsnippets/cppsnippets.h
    src/include/cppsnippets/snippet.h
    src/include/cppsnippets/snippet_parser.h
    src/include/cppsnippets/snippet_manager.h
    src/include/cppsnippets/export.h
)

# Build the shared/dynamic library
add_library(cppsnippets_lib ${LIB_SOURCES} ${LIB_HEADERS})
target_include_directories(cppsnippets_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    $<INSTALL_INTERFACE:include>
)

# Set library properties for Windows DLL export
if(BUILD_SHARED_LIBS)
    target_compile_definitions(cppsnippets_lib PRIVATE CPPSNIPPETS_EXPORTS)
endif()

# Set output name and versioning
set_target_properties(cppsnippets_lib PROPERTIES
    OUTPUT_NAME cppsnippets
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Build Qt application
if(BUILD_APP)
    set(APP_SOURCES
        src/app/main.cpp
        src/app/mainwindow.cpp
        src/app/mainwindow.h
    )
    
    add_executable(cppsnippets_app ${APP_SOURCES})
    target_link_libraries(cppsnippets_app PRIVATE cppsnippets_lib ${QT_LIBRARIES})
    set_target_properties(cppsnippets_app PROPERTIES
        OUTPUT_NAME cppsnippets
        WIN32_EXECUTABLE ON
    )
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
    
    include(GoogleTest)
    
    # Test executable
    add_executable(cppsnippets_tests
        tests/test_main.cpp
        tests/test_snippet.cpp
        tests/test_snippet_parser.cpp
        tests/test_snippet_manager.cpp
        tests/test_snippet_session.cpp
    )
    
    target_link_libraries(cppsnippets_tests PRIVATE
        cppsnippets_lib
        GTest::gtest
        GTest::gtest_main
    )

    # Ensure GTest built shared when desired (already in your PR maybe).
    # After target_link_libraries(cppsnippets_tests PRIVATE GTest::gtest_main ...)

    if (WIN32 AND BUILD_SHARED_LIBS)
      # Copy GTest and project library DLLs next to the test executable so the Windows loader finds them
      add_custom_command(TARGET cppsnippets_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime DLLs to test output dir..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GTest::gtest> $<TARGET_FILE_DIR:cppsnippets_tests>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:GTest::gtest_main> $<TARGET_FILE_DIR:cppsnippets_tests>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:cppsnippets_lib> $<TARGET_FILE_DIR:cppsnippets_tests>
        COMMENT "Copy GTest and project DLLs to test binary directory")
    endif()

    # Discover tests for CTest
    gtest_discover_tests(cppsnippets_tests)
endif()

# Installation rules
include(GNUInstallDirs)

install(TARGETS cppsnippets_lib
    EXPORT cppsnippets-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY src/include/cppsnippets
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(BUILD_APP)
    install(TARGETS cppsnippets_app
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Export targets for find_package support
install(EXPORT cppsnippets-targets
    FILE cppsnippets-targets.cmake
    NAMESPACE cppsnippets::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppsnippets
)

# Generate package config
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/cppsnippets-config.cmake.in
    ${CMAKE_BINARY_DIR}/cppsnippets-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppsnippets
)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/cppsnippets-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_BINARY_DIR}/cppsnippets-config.cmake
    ${CMAKE_BINARY_DIR}/cppsnippets-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppsnippets
)
