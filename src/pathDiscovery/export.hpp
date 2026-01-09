/**
 * @file export.hpp
 * @brief Export macros for pathDiscovery library
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef PATHDISCOVERY_EXPORTS
    #define PATHDISCOVERY_API __declspec(dllexport)
  #else
    #define PATHDISCOVERY_API __declspec(dllimport)
  #endif
#else
  #define PATHDISCOVERY_API
#endif
