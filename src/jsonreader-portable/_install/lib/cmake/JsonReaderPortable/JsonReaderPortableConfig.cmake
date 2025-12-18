# JsonReaderPortableConfig.cmake - package config

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was JsonReaderPortableConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

####################################################################################

include("${CMAKE_CURRENT_LIST_DIR}/JsonReaderPortableTargets.cmake")

# Provide a convenient imported target namespace hint
set(JsonReaderPortable_NAMESPACE "JsonReaderPortable::")

# Basic feature info for consumers
set(JsonReaderPortable_VERSION "0.1.0")
