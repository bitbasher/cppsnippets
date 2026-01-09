/**
 * @file export.hpp
 * @brief Export macros for resourceDiscovery library
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef RESOURCEDISCOVERY_EXPORTS
    #define RESOURCEDISCOVERY_API __declspec(dllexport)
  #else
    #define RESOURCEDISCOVERY_API __declspec(dllimport)
  #endif
#else
  #define RESOURCEDISCOVERY_API
#endif
