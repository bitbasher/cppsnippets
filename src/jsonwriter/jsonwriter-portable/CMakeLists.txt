cmake_minimum_required(VERSION 3.16)
project(JsonWriterPortable VERSION 0.1.0 LANGUAGES CXX)

# Enable Qt's automoc for targets here
set(CMAKE_AUTOMOC ON)

option(JSONWRITER_BUILD_TESTS "Build JsonWriter tests" ON)

# Try Qt6 first, then Qt5
set(_need_qttest ${JSONWRITER_BUILD_TESTS})
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

add_library(JsonWriterPortable STATIC
  src/JsonWriter.cc
)

target_include_directories(JsonWriterPortable PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(JsonWriterPortable PUBLIC cxx_std_17)

if(Qt6_FOUND)
  target_link_libraries(JsonWriterPortable PUBLIC Qt6::Core)
else()
  target_link_libraries(JsonWriterPortable PUBLIC Qt5::Core)
endif()

if(JSONWRITER_BUILD_TESTS)
  enable_testing()
  
  # Find nlohmann_json and json-schema-validator for testing only
  find_package(nlohmann_json CONFIG QUIET)
  find_package(nlohmann_json_schema_validator CONFIG QUIET)
  
  add_executable(jsonwriter_qttest tests/jsonwriter_qttest.cpp)
  target_link_libraries(jsonwriter_qttest PRIVATE JsonWriterPortable)
  if(Qt6_FOUND)
    target_link_libraries(jsonwriter_qttest PRIVATE Qt6::Test)
  else()
    target_link_libraries(jsonwriter_qttest PRIVATE Qt5::Test)
  endif()
  add_test(NAME jsonwriter_qttest COMMAND jsonwriter_qttest)
  
  # Create test output directory
  add_custom_command(TARGET jsonwriter_qttest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
      "$<TARGET_FILE_DIR:jsonwriter_qttest>/test_output"
    COMMENT "Creating JsonWriter test output directory...")

  # Additional file-based tests for write/read round-trip verification
  add_executable(jsonwriter_files_qttest tests/test_jsonwriter_files.cpp)
  target_link_libraries(jsonwriter_files_qttest PRIVATE JsonWriterPortable)
  if(Qt6_FOUND)
    target_link_libraries(jsonwriter_files_qttest PRIVATE Qt6::Test)
  else()
    target_link_libraries(jsonwriter_files_qttest PRIVATE Qt5::Test)
  endif()
  
  # Add schema validation libraries if available (test-only, not in main library)
  if(nlohmann_json_FOUND AND nlohmann_json_schema_validator_FOUND)
    target_link_libraries(jsonwriter_files_qttest PRIVATE 
      nlohmann_json::nlohmann_json
      nlohmann_json_schema_validator::validator)
    target_compile_definitions(jsonwriter_files_qttest PRIVATE HAS_SCHEMA_VALIDATOR)
    message(STATUS "JsonWriter tests: Schema validation enabled (nlohmann_json + validator)")
  else()
    message(STATUS "JsonWriter tests: Schema validation disabled (libraries not found)")
  endif()
  
  target_compile_definitions(jsonwriter_files_qttest PRIVATE SRCDIR="${CMAKE_SOURCE_DIR}")
  add_test(NAME jsonwriter_files_qttest COMMAND jsonwriter_files_qttest)
  
  # Validator tests with known good/bad JSON files
  if(nlohmann_json_FOUND AND nlohmann_json_schema_validator_FOUND)
    add_executable(jsonwriter_validator_qttest tests/test_jsonwriter_validator.cpp)
    target_link_libraries(jsonwriter_validator_qttest PRIVATE 
      JsonWriterPortable
      nlohmann_json::nlohmann_json
      nlohmann_json_schema_validator::validator)
    if(Qt6_FOUND)
      target_link_libraries(jsonwriter_validator_qttest PRIVATE Qt6::Test)
    else()
      target_link_libraries(jsonwriter_validator_qttest PRIVATE Qt5::Test)
    endif()
    target_compile_definitions(jsonwriter_validator_qttest PRIVATE SRCDIR="${CMAKE_SOURCE_DIR}")
    add_test(NAME jsonwriter_validator_qttest COMMAND jsonwriter_validator_qttest)
  endif()
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
install(TARGETS JsonWriterPortable
        EXPORT JsonWriterPortableTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Generate and install package config files
set(JSONWRITER_PORTABLE_NAMESPACE JsonWriterPortable::)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/JsonWriterPortableConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/JsonWriterPortableConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/JsonWriterPortableConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/JsonWriterPortable
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

install(EXPORT JsonWriterPortableTargets
        NAMESPACE ${JSONWRITER_PORTABLE_NAMESPACE}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/JsonWriterPortable)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/JsonWriterPortableConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/JsonWriterPortableConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/JsonWriterPortable)
