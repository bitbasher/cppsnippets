/**
 * @file export.h
 * @brief Platform-specific export/import macros for the cppsnippets library
 * 
 * This header defines macros for proper symbol visibility when building
 * or consuming the cppsnippets library as a shared/dynamic library.
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #ifdef CPPSNIPPETS_EXPORTS
        #define CPPSNIPPETS_API __declspec(dllexport)
    #else
        #define CPPSNIPPETS_API __declspec(dllimport)
    #endif
#else
    #if defined(__GNUC__) && __GNUC__ >= 4
        #define CPPSNIPPETS_API __attribute__((visibility("default")))
    #else
        #define CPPSNIPPETS_API
    #endif
#endif
