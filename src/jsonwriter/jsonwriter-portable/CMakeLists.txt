cmake_minimum_required(VERSION 3.16)
project(JsonWriterPortable VERSION 0.1.0 LANGUAGES CXX)

# Enable Qt's automoc for targets here
set(CMAKE_AUTOMOC ON)

option(JSONWRITER_BUILD_TESTS "Build JsonWriter tests" OFF)

# Qt is provided by parent project
# Assume Qt6::Core is already available

add_library(JsonWriterPortable STATIC
  src/JsonWriter.cc
)

target_include_directories(JsonWriterPortable PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(JsonWriterPortable PUBLIC cxx_std_17)

# Link against Qt6::Core (parent project provides this)
target_link_libraries(JsonWriterPortable PUBLIC Qt6::Core)
