#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "JsonReaderPortable::JsonReaderPortable" for configuration ""
set_property(TARGET JsonReaderPortable::JsonReaderPortable APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(JsonReaderPortable::JsonReaderPortable PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libJsonReaderPortable.a"
  )

list(APPEND _cmake_import_check_targets JsonReaderPortable::JsonReaderPortable )
list(APPEND _cmake_import_check_files_for_JsonReaderPortable::JsonReaderPortable "${_IMPORT_PREFIX}/lib/libJsonReaderPortable.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
