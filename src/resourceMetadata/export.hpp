/**
 * @file export.hpp
 * @brief Export macros for resourceMetadata library
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef RESOURCEMETADATA_EXPORTS
    #define RESOURCEMETADATA_API __declspec(dllexport)
  #else
    #define RESOURCEMETADATA_API __declspec(dllimport)
  #endif
#else
  #define RESOURCEMETADATA_API
#endif
