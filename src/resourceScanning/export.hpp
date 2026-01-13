/**
 * @file export.hpp
 * @brief Export macros for resourceScanning library
 */

#pragma once

#if defined(RESOURCESCANNING_STATIC_DEFINE)
  // Static linking - no export needed
  #define RESOURCESCANNING_API
#elif defined(_WIN32) || defined(_WIN64)
  #ifdef RESOURCESCANNING_EXPORTS
    #define RESOURCESCANNING_API __declspec(dllexport)
  #else
    #define RESOURCESCANNING_API __declspec(dllimport)
  #endif
#else
  #define RESOURCESCANNING_API
#endif
