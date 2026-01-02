/**
 * @file platformInfo.h
 * @brief Platform information utilities for cross-platform support
 */

#pragma once

#include "export.hpp"
#include "extnOSVersRef.hpp"
#include <QString>
#include <QList>

// Forward declarations
class QScreen;
class QSysInfo;

namespace platformInfo {

/**
 * @brief Provides platform and system information
 * 
 * Wraps Qt's QSysInfo to provide convenient access to system
 * information including OS details, CPU architecture, and screen info.
 */
class PLATFORMINFO_API PlatformInfo {
public:
    /**
     * @brief Default constructor
     */
    PlatformInfo();
    
    /**
     * @brief Copy constructor
     */
    PlatformInfo(const PlatformInfo& other);
    
    /**
     * @brief Assignment operator
     */
    PlatformInfo& operator=(const PlatformInfo& other);
    
    /**
     * @brief Get the current OS type
     * @return The detected OS type enumeration
     */
    ExtnOSType currentOSType() const;
    
    /**
     * @brief Get the OS version reference for the current platform
     * @return Pointer to the OS version reference, or nullptr
     */
    const ExtnOSVersRef* currentOSVersionRef() const;
    
    /**
     * @brief Get the product type (e.g., "windows", "macos", "linux")
     * @return Product type string
     */
    static QString productType();
    
    /**
     * @brief Get the product version (e.g., "10.0", "14.0")
     * @return Product version string
     */
    static QString productVersion();
    
    /**
     * @brief Get the kernel type (e.g., "winnt", "darwin", "linux")
     * @return Kernel type string
     */
    static QString kernelType();
    
    /**
     * @brief Get the kernel version
     * @return Kernel version string
     */
    static QString kernelVersion();
    
    /**
     * @brief Get the CPU architecture (e.g., "x86_64", "arm64")
     * @return CPU architecture string
     */
    static QString cpuArchitecture();
    
    /**
     * @brief Get the build CPU architecture
     * @return Build architecture string
     */
    static QString buildCpuArchitecture();
    
    /**
     * @brief Get the pretty product name (e.g., "Windows 11", "macOS Sonoma")
     * @return Human-readable product name
     */
    static QString prettyProductName();
    
    /**
     * @brief Get the machine host name
     * @return Host name string
     */
    static QString machineHostName();
    
    /**
     * @brief Get all available screens
     * @return List of QScreen pointers
     */
    static QList<QScreen*> screens();
    
    /**
     * @brief Print platform information to debug output
     */
    void debugPrint() const;

private:
    ExtnOSVersRef m_osVersionRef;
};

} // namespace platformInfo
