/**
 * @file export.h
 * @brief Platform-specific export/import macros for the scadtemplates library
 * 
 * This header defines macros for proper symbol visibility when building
 * or consuming the scadtemplates library as a shared/dynamic library.
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #ifdef SCADTEMPLATES_EXPORTS
        #define SCADTEMPLATES_API __declspec(dllexport)
    #else
        #define SCADTEMPLATES_API __declspec(dllimport)
    #endif
#else
    #if defined(__GNUC__) && __GNUC__ >= 4
        #define SCADTEMPLATES_API __attribute__((visibility("default")))
    #else
        #define SCADTEMPLATES_API
    #endif
#endif
