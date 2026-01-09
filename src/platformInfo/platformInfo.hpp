/**
 * @file platformInfo.h
 * @brief Platform information utilities for cross-platform support
 */

#pragma once

#include "export.hpp"
#include "resourceTypeInfo.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QList>
#include <QScreen>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QSysInfo>
#include <QDataStream>

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
   * @brief Get the current OS type
   * @return The detected OS type enumeration
   */
  static QString currentOSType() { return QSysInfo::productType(); }

  /**
   * @brief Get the product type (e.g., "windows", "macos", "linux")
   * @return Product type string
   */
  static QString productType() { return QSysInfo::productType(); }

  /**
   * @brief Get the product version (e.g., "10.0", "14.0")
   * @return Product version string
   */
  static QString productVersion() { return QSysInfo::productVersion(); }

  /**
   * @brief Get the kernel type (e.g., "winnt", "darwin", "linux")
   * @return Kernel type string
   */
  static QString kernelType() { return QSysInfo::kernelType(); }

  /**
   * @brief Get the kernel version
   * @return Kernel version string
   */
  static QString kernelVersion() { return QSysInfo::kernelVersion(); }

  /**
   * @brief Get the CPU architecture (e.g., "x86_64", "arm64")
   * @return CPU architecture string
   */
  static QString cpuArchitecture() { return QSysInfo::currentCpuArchitecture(); }

  /**
   * @brief Get the build CPU architecture
   * @return Build architecture string
   */
  static QString buildCpuArchitecture() { return QSysInfo::buildCpuArchitecture(); }

  /**
   * @brief Get the pretty product name (e.g., "Windows 11", "macOS Sonoma")
   * @return Human-readable product name
   */
  static QString prettyProductName() { return QSysInfo::prettyProductName(); }

  /**
   * @brief Get the machine host name
   * @return Host name string
   */
  static QString machineHostName() { return QSysInfo::machineHostName(); }


  /* ========================================
     Installation Tier (Executable Location)
     ======================================== */

  /**
   * @brief Get the full path to the currently running executable
   * @return Absolute path to the executable file
   *
   * Uses QCoreApplication::applicationFilePath() to get the full path
   * to the executable that is currently running. This is reliable across
   * all platforms and works even for renamed or moved executables.
   */
  static QString getCurrentExecutablePath() {
    return QCoreApplication::applicationFilePath();
  }

  /**
   * @brief Get the directory containing the currently running executable
   * @return Absolute path to the executable's directory
   *
   * Wraps QCoreApplication::applicationDirPath(), which already returns
   * an absolute directory path for the running application.
   */
  static QString getCurrentExecutableDirPath() {
    return QCoreApplication::applicationDirPath();
  }


 /* ========================================
     User Tier (Personal Resources)
    ======================================== */

  /**
   * @brief Get user's home directory
   * @return Absolute path to user's home directory
   *
   * Uses QDir::homePath() to get the user's home directory.
   * On Windows: C:/Users/<USER>
   * On Linux: ~
   * On macOS: ~
   */
  static QString getHomeDirectory() {
    return QDir::homePath();
  }

  // ========================================
  // Generic User Locations (Non-App-Specific)
  // ========================================

  /**
   * @brief Get user's documents directory
   * @return Path to user's documents (guaranteed non-empty)
   *
   * Returns path from QStandardPaths::DocumentsLocation.
   * On Windows: C:/Users/<USER>/Documents
   * On Linux/macOS: ~/Documents
   */
  static QString getUserDocumentsLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  }

  /**
   * @brief Get user's templates directory
   * @return Path to user's templates (may be empty if no concept of templates)
   *
   * Returns path from QStandardPaths::TemplatesLocation.
   * On Windows: C:/Users/<USER>/AppData/Roaming/Microsoft/Windows/Templates
   * On Linux/macOS: ~/Templates
   */
  static QString getUserTemplatesLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::TemplatesLocation);
  }

  /**
   * @brief Get all available screens
   * @return List of QScreen pointers
   */
  static QList<QScreen *> screens() { return QGuiApplication::screens(); }

  /**
   * @brief Print platform information to debug output
   */
  void debugPrint() const;
};

} // namespace platformInfo
