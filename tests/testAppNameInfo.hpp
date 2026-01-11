/**
 * @file testAppNameInfo.hpp
 * @brief Test-specific application metadata for test executables
 * 
 * This header is used by test executables that compile their own copy
 * of ResourcePaths code. It provides runtime-modifiable application name
 * to test resource discovery with different application configurations.
 * 
 * Test apps that use this header must:
 * 1. Define USE_TEST_APP_INFO before including any headers
 * 2. Compile ResourcePaths.cpp and dependencies into the test executable
 * 3. Call appInfo::setBaseName() before any discovery calls
 * 
 * Usage:
 *   // In CMakeLists.txt:
 *   target_compile_definitions(mytest PRIVATE USE_TEST_APP_INFO)
 *   
 *   // In test code:
 *   #ifdef USE_TEST_APP_INFO
 *   #include "testAppNameInfo.hpp"
 *   #else
 *   #include "applicationNameInfo.hpp"
 *   #endif
 *   
 *   int main(int argc, char* argv[]) {
 *       appInfo::setBaseName(QString::fromUtf8(argv[1]));
 *       // ... rest of test
 *   }
 */

#pragma once

#include <QString>
#include <string>

namespace appInfo {
    // Runtime-modifiable application name for testing
    namespace detail {
        static QString g_testBaseName = "OpenSCAD";  // Default test name
    }
    
    // Set the test application name (call from main before any discovery)
    inline void setBaseName(const QString& name) {
        detail::g_testBaseName = name;
    }
    
    // Get effective base name - returns the runtime-set value
    inline QString getBaseName() {
        return detail::g_testBaseName;
    }
    
    // Test defaults for other metadata
    constexpr const char* suffix = "";
    constexpr const char* displayName = "Test App";
    
    constexpr const char* author = "Test Author";
    constexpr const char* organization = "Test Org";
    
    // Version information (test defaults)
    constexpr const char* version = "0.0.0-test";
    constexpr int versionMajor = 0;
    constexpr int versionMinor = 0;
    constexpr int versionPatch = 0;
    
    constexpr const char* gitCommitHash = "test";
} // namespace appInfo
