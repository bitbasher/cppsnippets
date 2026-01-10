/**
 * @file export.hpp
 * @brief Export macros for resourceMetadata library
 * 
 * NOTE: resourceMetadata is compiled into scadtemplates_lib, so we use
 * SCADTEMPLATES_EXPORTS define, not RESOURCEMETADATA_EXPORTS
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef SCADTEMPLATES_EXPORTS
    #define RESOURCEMETADATA_API __declspec(dllexport)
  #else
    #define RESOURCEMETADATA_API __declspec(dllimport)
  #endif
#else
  #define RESOURCEMETADATA_API
#endif
