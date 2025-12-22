cmake_minimum_required(VERSION 3.16)
project(JsonReaderPortable VERSION 0.1.0 LANGUAGES CXX)

# Enable Qt's automoc for targets here
set(CMAKE_AUTOMOC ON)

option(JSONREADER_BUILD_TESTS "Build JsonReader tests" ON)

# Try Qt6 first, then Qt5
set(_need_qttest ${JSONREADER_BUILD_TESTS})
if(_need_qttest)
  find_package(Qt6 QUIET COMPONENTS Core Test)
else()
  find_package(Qt6 QUIET COMPONENTS Core)
endif()
if(NOT Qt6_FOUND)
  if(_need_qttest)
    find_package(Qt5 REQUIRED COMPONENTS Core Test)
  else()
    find_package(Qt5 REQUIRED COMPONENTS Core)
  endif()
endif()

add_library(JsonReaderPortable STATIC
  src/JsonReader.cc
)

target_include_directories(JsonReaderPortable PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(JsonReaderPortable PUBLIC cxx_std_17)

if(Qt6_FOUND)
  target_link_libraries(JsonReaderPortable PUBLIC Qt6::Core)
else()
  target_link_libraries(JsonReaderPortable PUBLIC Qt5::Core)
endif()

if(JSONREADER_BUILD_TESTS)
  enable_testing()
  
  # Find nlohmann_json and json-schema-validator for testing only
  find_package(nlohmann_json CONFIG QUIET)
  find_package(nlohmann_json_schema_validator CONFIG QUIET)
  
  add_executable(jsonreader_qttest tests/jsonreader_qttest.cpp)
  target_link_libraries(jsonreader_qttest PRIVATE JsonReaderPortable)
  if(Qt6_FOUND)
    target_link_libraries(jsonreader_qttest PRIVATE Qt6::Test)
  else()
    target_link_libraries(jsonreader_qttest PRIVATE Qt5::Test)
  endif()
  add_test(NAME jsonreader_qttest COMMAND jsonreader_qttest)
  
  # Copy test data to build directory for runtime access
  add_custom_command(TARGET jsonreader_qttest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/data"
      "$<TARGET_FILE_DIR:jsonreader_qttest>/data"
    COMMENT "Copying JsonReader test data...")

  # Additional file-based tests that read real color schemes and templates
  add_executable(jsonreader_files_qttest tests/test_jsonreader_files.cpp)
  target_link_libraries(jsonreader_files_qttest PRIVATE JsonReaderPortable)
  if(Qt6_FOUND)
    target_link_libraries(jsonreader_files_qttest PRIVATE Qt6::Test)
  else()
    target_link_libraries(jsonreader_files_qttest PRIVATE Qt5::Test)
  endif()
  
  # Add schema validation libraries if available (test-only, not in main library)
  if(nlohmann_json_FOUND AND nlohmann_json_schema_validator_FOUND)
    target_link_libraries(jsonreader_files_qttest PRIVATE 
      nlohmann_json::nlohmann_json
      nlohmann_json_schema_validator::validator)
    target_compile_definitions(jsonreader_files_qttest PRIVATE HAS_SCHEMA_VALIDATOR)
    message(STATUS "JsonReader tests: Schema validation enabled (nlohmann_json + validator)")
  else()
    message(STATUS "JsonReader tests: Schema validation disabled (libraries not found)")
  endif()
  
  target_compile_definitions(jsonreader_files_qttest PRIVATE SRCDIR="${CMAKE_SOURCE_DIR}")
  add_test(NAME jsonreader_files_qttest COMMAND jsonreader_files_qttest)
endif()

# -----------------------------
# Installation (find_package)
# -----------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install library and export targets
install(TARGETS JsonReaderPortable
        EXPORT JsonReaderPortableTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Generate and install package config files
set(JSONREADER_PORTABLE_NAMESPACE JsonReaderPortable::)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/JsonReaderPortableConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/JsonReaderPortableConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/JsonReaderPortableConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/JsonReaderPortable
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

install(EXPORT JsonReaderPortableTargets
        NAMESPACE ${JSONREADER_PORTABLE_NAMESPACE}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/JsonReaderPortable)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/JsonReaderPortableConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/JsonReaderPortableConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/JsonReaderPortable)
