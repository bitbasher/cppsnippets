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
 * 3. Call appInfo::setTestAppName() before any discovery calls
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
 *       appInfo::setTestAppName(QString::fromUtf8(argv[1]));
 *       // ... rest of test
 *   }
 */

#pragma once

#include <QString>
#include <string>

namespace appInfo {
    // Application name without suffix (test default)
    constexpr const char* baseName = "OpenSCAD";
    
    // Test override for application name (used by test executables)
    // Internal storage for test name override
    inline QString& testAppName() {
        static QString name;
        return name;
    }
    
    // Get effective base name (test override if set, otherwise compile-time constant)
    // Use this instead of accessing baseName directly
    inline QString getBaseName() {
        return testAppName().isEmpty() 
            ? QString::fromUtf8(baseName) 
            : testAppName();
    }
    
    // Set test override (call from test executables before discovery)
    inline void setTestAppName(const QString& name) {
        testAppName() = name;
    }
    
    // Test defaults for other metadata
    constexpr const char* suffix = "";
    constexpr const char* displayName = "Test App";
    
    // Get sibling installation folder name (for discovering alternate versions)
    inline QString getSiblingName() {
        QString base = getBaseName();
        QString suf = QString::fromUtf8(suffix);
        
        // LTS → Nightly sibling candidate
        if (suf.isEmpty()) {
            return base + QStringLiteral(" (Nightly)");
        }
        // Nightly → LTS sibling
        return base;
    }
    
    constexpr const char* author = "Test Author";
    constexpr const char* organization = "Test Org";
    
    // Version information (test defaults)
    constexpr const char* version = "0.0.0-test";
    constexpr int versionMajor = 0;
    constexpr int versionMinor = 0;
    constexpr int versionPatch = 0;
    
    constexpr const char* gitCommitHash = "test";
} // namespace appInfo
