/**
 * @file export.h
 * @brief Platform-specific export/import macros for the resourceMgmt library
 * 
 * This header defines macros for proper symbol visibility when building
 * or consuming the resourceMgmt library as a shared/dynamic library.
 * 
 * The resourceMgmt library combines:
 * - platformInfo: OS detection, path resolution, defaults
 * - resInventory: Location storage, scanning, UI widgets
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #ifdef RESOURCEMGMT_EXPORTS
        #define PLATFORMINFO_API __declspec(dllexport)
    #else
        #define PLATFORMINFO_API __declspec(dllimport)
    #endif
#else
    #if defined(__GNUC__) && __GNUC__ >= 4
        #define PLATFORMINFO_API __attribute__((visibility("default")))
    #else
        #define PLATFORMINFO_API
    #endif
#endif
