/**
 * @file resourcePaths.h
 * @brief Resource directory and path management for OpenSCAD resources
 *
 * Provides platform-specific search paths and resource type definitions
 * for locating OpenSCAD resources (examples, fonts, color schemes, etc.)
 *
 * Resources are sourced from two locations:
 * 1. Application installation directory (primary)
 * 2. User documents directory (secondary, user-customizable)
 */

#pragma once

#include "export.h"
#include "extnOSVersRef.h"
#include "resourceInfo/resourceTypes.h"
#include "resourceInfo/resourceTier.h"
#include "resInventory/ResourceLocation.h"
#include <QDir>
#include <QString>
#include <QStringList>
#include <QList>

class QSettings;

namespace platformInfo {

using resourceInfo::ResourceTier;
using resourceInfo::ResourceType;
using resourceInfo::ResourceTypeInfo;

// Associates a ResourceLocation (path) with its tier assignment
struct ResourcePathElement {
  platformInfo::ResourceLocation location;   ///< Filesystem location (may later carry env vars)
  resourceInfo::ResourceTier tier;           ///< Tier this location belongs to

  ResourcePathElement() = default;
  ResourcePathElement(const platformInfo::ResourceLocation &loc,
                      resourceInfo::ResourceTier t)
      : location(loc), tier(t) {}
};

// User/configurable environment variable entry
struct EnvVarEntry {
  QString name;
  QString value;
};

/**
 * @brief Manages resource paths for OpenSCAD
 *
 * This class provides platform-specific search paths for locating
 * the OpenSCAD resource directory, and methods for resolving paths
 * to specific resource types.
 *
 * Resources are loaded from two sources:
 * 1. **Application resources** - Built-in read-only resources
 * (resourceBasePath)
 * 2. **User resources** - Writable user config/data folder (userConfigPath)
 *
 * **OpenSCAD Build Suffixes:**
 *
 * OpenSCAD uses preprocessor macros for build variants:
 * - `OPENSCAD_SUFFIX` - Optional suffix like " (Nightly)" for dev builds
 * - `OPENSCAD_FOLDER_NAME` - "OpenSCAD" for user/machine paths (unsuffixed)
 * - `RESOURCE_FOLDER(path)` - Appends OPENSCAD_SUFFIX to share paths
 *
 * Example with suffix " (Nightly)":
 * - User/machine folder name stays "OpenSCAD"
 * - `RESOURCE_FOLDER("../share/openscad")` → "../share/openscad (Nightly)"

 * This class mirrors this behavior via the `suffix` parameter for install-tier lookups only.
 *
 * OpenSCAD defines several platform-specific paths (in PlatformUtils):
 *
 * | Path | Linux | Windows | macOS | Purpose |
 * |------|-------|---------|-------|---------|
 * | documentsPath | $HOME/.local/share | CSIDL_PERSONAL | NSDocumentDirectory |
 * User libraries, backups | | userDocumentsPath | $XDG_DOCUMENTS_DIR |
 * =documentsPath | =documentsPath | Default save location | | userConfigPath |
 * $XDG_CONFIG_HOME | CSIDL_LOCAL_APPDATA | NSAppSupportDir | Writable config |
 * | resourceBasePath | (from app init) | (from app init) | (from app init) |
 * Read-only resources |
 *
 * @note This class is designed to INTEGRATE with OpenSCAD's PlatformUtils.
 * The following are NOT duplicated - they will be provided by PlatformUtils:
 * - applicationpath - path to the application executable
 * - resourcespath / resourceBasePath() - resolved path to built-in resources
 * - OPENSCAD_FOLDER_NAME - "OpenSCAD" (or variant like "OpenSCAD (Nightly)")
 * - documentsPath() - user libraries and backup files location
 * - userDocumentsPath() - default file save location
 * - userConfigPath() - writable configuration folder + OPENSCAD_FOLDER_NAME
 *
 * This class provides the search path logic and resource type definitions
 * that can be used alongside PlatformUtils.
 */
class RESOURCEMGMT_API ResourcePaths {
public:
  /**
   * @brief Default constructor
   *
   * Initializes with the current platform's search paths.
   * Does not set the application path - call setApplicationPath()
   * or use the parameterized constructor.
   */
  ResourcePaths();

  /**
   * @brief Construct with application path
   * @param applicationPath Path to the application executable directory
   * @param suffix Build suffix (e.g., "" for release, " (Nightly)" for nightly
   * builds)
   *
  * @note The suffix corresponds to OpenSCAD's OPENSCAD_SUFFIX preprocessor
  * define. It is appended to installation/share paths; user and machine tiers
  * remain unsuffixed.
   */
  explicit ResourcePaths(const QString &applicationPath,
                         const QString &suffix = QString());

  /**
   * @brief Set the application executable path
   * @param path Path to the application executable directory
   *
   * Search paths are relative to this location.
   */
  void setApplicationPath(const QString &path);

  /**
   * @brief Get the application path
   * @return Current application path
   */
  QString applicationPath() const { return m_applicationPath; }

  /**
   * @brief Set the build suffix
   * @param suffix Build suffix (e.g., "" for release, " (Nightly)" for nightly)
   *
    * This suffix is appended only to installation/share search paths. Machine and
    * user tier paths remain unsuffixed per revised policy.
   */
  void setSuffix(const QString &suffix);

  /**
   * @brief Get the build suffix
   * @return Current suffix (e.g., "" or " (Nightly)")
   */
  QString suffix() const { return m_suffix; }

  /**
   * @brief Get the OpenSCAD folder name (for user paths)
    * @return "OpenSCAD" (unsuffixed)
   *
    * This is equivalent to OpenSCAD's OPENSCAD_FOLDER_NAME macro without build
    * suffix for user/machine tiers.
   */
  QString folderName() const;

  // ========== Resource Type Information ==========

  /**
   * @brief Get all resource type definitions
   * @return Vector of all resource type info
   */
  static QList<ResourceTypeInfo> allResourceTypes();

  // ========== Default Search Paths (Immutable) ==========
  //
  // These are compile-time constants defining where to look for resources.
  // Used by makeLocationsList() to build the available locations.
  // User cannot modify these - they modify their selection in ResLocMap.

  /**
   * @brief Get default search paths for a specific tier
   * @param tier The resource tier (Installation, Machine, or User)
   * @return Immutable list of default paths for the tier
   *
   * Returns platform-specific compile-time search paths:
   * - Installation: relative to application executable, may include build suffix
   * - Machine: system-wide locations for all users
   * - User: locations in user's home/config directory
   *
   * This is a compile-time constant list that cannot be modified by the user.
   * Used as the default/reset source for preferences.
   */
  static const QStringList &defaultSearchPaths(resourceInfo::ResourceTier tier);
  
  /**
   * @brief Get expanded search paths with env vars resolved
   * @param tier The resource tier
   * @return List of expanded paths
   * 
   * Expands any ${VAR} or %VAR% templates in defaultSearchPaths(tier)
   * using current environment variables and user overrides.
   */
  QStringList expandedSearchPaths(resourceInfo::ResourceTier tier) const;

  /**
   * @brief Build location elements list from default search paths
   * @return Ordered list of locations from Installation → Machine → User
   *
   * Constructs concrete absolute paths from compile-time template defaults:
   * - Expands environment variables in paths
   * - Applies install-tier suffix rules
   * - Discovers sibling installations
   * - Resolves relative paths against application path
   *
   * Returns locations for all three tiers in priority order.
   */
  QList<ResourcePathElement> makeLocationsList() const;

  // Unified default search paths structure indexed by tier
  // These are compile-time platform-specific constants
  inline static const QMap<resourceInfo::ResourceTier, QStringList> s_defaultSearchPaths = {
    // Installation tier: relative to application executable
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    {resourceInfo::ResourceTier::Installation, {
      QStringLiteral("%PROGRAMFILES%/"), // Standard install (append openscad + suffix)
      QStringLiteral("."),                // Release location (same as exe)
      QStringLiteral("../share/"),        // MSYS2 style Base share folder
      QStringLiteral("..")                // Dev location
    }},
    // Machine tier: system-wide locations for all users
    {resourceInfo::ResourceTier::Machine, {
      QStringLiteral("%PROGRAMDATA%/ScadTemplates"),
      QStringLiteral("C:/ProgramData/")
    }},
    // User tier: user-specific locations
    {resourceInfo::ResourceTier::User, {
      QStringLiteral("%APPDATA%/ScadTemplates"),
      QStringLiteral("%LOCALAPPDATA%/ScadTemplates"),
      QStringLiteral("."),
      QStringLiteral("../")
    }}
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
    {resourceInfo::ResourceTier::Installation, {
      QStringLiteral("../Resources"),
      QStringLiteral("../../.."),
      QStringLiteral("../../../.."),
      QStringLiteral(".."),
      QStringLiteral("../share/")
    }},
    {resourceInfo::ResourceTier::Machine, {
      QStringLiteral("/Library/Application Support/ScadTemplates")
    }},
    {resourceInfo::ResourceTier::User, {
      QStringLiteral("${HOME}/Library/Application Support/ScadTemplates"),
      QStringLiteral("."),
      QStringLiteral("../../Documents/")
    }}
#else // Linux/BSD/POSIX
    {resourceInfo::ResourceTier::Installation, {
      QStringLiteral("../share/"),
      QStringLiteral("../../share/"),
      QStringLiteral("."),
      QStringLiteral(".."),
      QStringLiteral("../..")
    }},
    {resourceInfo::ResourceTier::Machine, {
      QStringLiteral("/usr/share/"),
      QStringLiteral("/usr/local/share/"),
      QStringLiteral("/opt/openscad/share/")
    }},
    {resourceInfo::ResourceTier::User, {
      QStringLiteral("${XDG_CONFIG_HOME}/cppsnippets"),
      QStringLiteral("${HOME}/.config/cppsnippets"),
      QStringLiteral("${HOME}/.local/share/cppsnippets"),
      QStringLiteral("."),
      QStringLiteral("../../.local/share/")
    }}
#endif
  };

public:
  /**
   * @brief Get platform-specific search paths for application resources
   * @return List of relative paths to search for resource directory
   *
   * Convenience method that returns expanded Installation tier paths.
   * These paths are relative to the application path and are searched
   * in order until a valid resource directory is found.
   */
  QStringList appSearchPaths() const;

  /**
   * @brief Find the application resource directory
   * @return Absolute path to resource directory, or empty string if not found
   *
   * Searches through the platform-specific search paths relative to
   * the application path and returns the first valid directory found.
   */
  QString findAppResourceDirectory() const;

  /**
   * @brief Get the full path to an application resource type directory
   * @param type The resource type
   * @return Absolute path to the resource type directory, or empty if not found
   */
  QString appResourcePath(ResourceType type) const;

  /**
   * @brief Check if an application resource directory exists
   * @param type The resource type
   * @return true if the resource directory exists
   */
  bool hasAppResourceDirectory(ResourceType type) const;

  // ========== User Resource Paths ==========
  //
  // OpenSCAD PlatformUtils defines several user paths:
  //
  // documentsPath() - For user libraries and backup files
  //   Linux: $HOME/.local/share
  //   Windows: CSIDL_PERSONAL (My Documents)
  //   macOS: NSDocumentDirectory (~Documents)
  //
  // userDocumentsPath() - Default save/export location
  //   Linux: $XDG_DOCUMENTS_DIR (or from user-dirs.dirs)
  //   Windows/macOS: same as documentsPath()
  //
  // userConfigPath() - Writable configuration folder
  //   Linux: $XDG_CONFIG_HOME (default ~/.config)
  //   Windows: CSIDL_LOCAL_APPDATA
  //   macOS: NSApplicationSupportDirectory (~Library/Application Support)
  //   Returns: base path + "/" + OPENSCAD_FOLDER_NAME
  //
  // These methods provide equivalent functionality for use before
  // OpenSCAD PlatformUtils is available (e.g., during development/testing).

  /**
   * @brief Get the user config directory path (platform-specific)
   * @return Path to writable user config directory
   *
   * This is THE location where user configuration is stored:
   * - Linux: $XDG_CONFIG_HOME or $HOME/.config
   * - Windows: CSIDL_LOCAL_APPDATA (e.g., %LOCALAPPDATA%)
   * - macOS: NSApplicationSupportDirectory (~Library/Application Support)
   *
   * Returns the absolute path where user config files and settings should be stored.
   */
  QString getUserConfigPath() const;

  /**

   * @brief Get user config path for a specific platform
   * @param osType The OS type
   * @return Platform-specific user config directory path
   *
   * Platform behavior:
   * - Linux: $XDG_CONFIG_HOME or $HOME/.config
   * - Windows: CSIDL_LOCAL_APPDATA
   * - macOS: NSApplicationSupportDirectory
   */
  static QString userConfigPathForPlatform(ExtnOSType osType);

  /**
   * @brief Get user documents path for a specific platform
   * @param osType The OS type
   * @return Platform-specific documents path
   *
   * Platform behavior (matches OpenSCAD PlatformUtils::documentsPath):
   * - Linux: $HOME/.local/share
   * - Windows: CSIDL_PERSONAL (My Documents)
   * - macOS: NSDocumentDirectory (~Documents)
   *
   * @note Used for user libraries and backup files.
   */
  static QString userDocumentsPathForPlatform(ExtnOSType osType);

  // ========== Combined Resource Paths ==========

  /**
   * @brief Get all paths for a resource type (app only)
   * @param type The resource type
   * @return List of paths to search (app path if it exists)
   */
  QStringList appResourcePaths(ResourceType type) const;
  // ========== Environment Variable Helpers ==========
  /**
   * @brief Get configured environment variable overrides
   * @return Const reference to env var entries
   */
  const QList<EnvVarEntry>& envVars() const { return m_envVars; }

  /**
   * @brief Replace environment variable overrides
   * @param envVars New entries
   */
  void setEnvVars(const QList<EnvVarEntry>& envVars) { m_envVars = envVars; }

  /**
   * @brief Add or update a single environment variable override
   * @param name Variable name
   * @param value Variable value
   */
  void addEnvVar(const QString& name, const QString& value);

  /**
   * @brief Remove an environment variable override by name
   * @param name Variable name to remove
   */
  void removeEnvVar(const QString& name);

  /**
   * @brief Get an environment variable value (overrides first, then system)
   * @param name Variable name
   * @return Value if present, otherwise empty string
   */
  QString envVarValue(const QString& name) const;

  /**
   * @brief Expand environment variables in a path string
   * @param path Path with placeholders (supports ${VAR} and %VAR%)
   * @return Expanded path
   */
  QString expandEnvVars(const QString& path) const;

  // ========== Resource Path Elements (tier-tagged locations) ==========

  /**
   * @brief Get all configured resource path elements (ordered)
   * @return Const reference to internal list (order preserves search priority)
   */
  const QList<ResourcePathElement>& resourcePathElements() const { return m_resourcePathElements; }

  /**
   * @brief Replace all resource path elements
   * @param elements New list (order is preserved as provided)
   */
  void setResourcePathElements(const QList<ResourcePathElement>& elements) { m_resourcePathElements = elements; }

  /**
   * @brief Add a resource path element (appended at end)
   * @param element Element to add
   */
  void addResourcePathElement(const ResourcePathElement& element) { m_resourcePathElements.append(element); }

  /**
   * @brief Filter elements by tier (preserves order)
   * @param tier Tier to select
   * @return List of elements matching the tier
   */
  QList<ResourcePathElement> elementsByTier(resourceInfo::ResourceTier tier) const;

  // ========== Settings Persistence ==========

  /**
   * @brief Save environment variables to QSettings
   * @param settings QSettings instance to write to
   * 
   * Writes user-configured env vars to [EnvVars] section as key=value pairs.
   */
  void saveEnvVars(QSettings& settings) const;

  /**
   * @brief Load environment variables from QSettings
   * @param settings QSettings instance to read from
   * 
   * Reads user-configured env vars from [EnvVars] section.
   * Should be called during app startup before initializing ResourceLocationManager.
   */
  void loadEnvVars(QSettings& settings);

  // ========== Validation ==========

  /**
   * @brief Check if the application resource base directory has been found
   * @return true if findAppResourceDirectory() returns a valid path
   */
  bool isValid() const;

  /**
   * @brief Check if a path is a valid OpenSCAD resource directory
   * @param path Directory path to validate
   * @return true if the directory contains expected resource subdirectories
   */
  bool isValidResourceDirectory(const QString& path) const;

  /**
   * @brief Get the detected OS type
   * @return Current OS type used for search path selection
   */
  ExtnOSType osType() const { return m_osType; }

private:
  QString m_applicationPath;
  QString m_suffix; ///< Build suffix (e.g., "" or " (Nightly)")
  ExtnOSType m_osType;

  QList<EnvVarEntry> m_envVars; ///< User/config overrides applied before system env

  // Tier-tagged search locations (single list; order preserves priority)
  QList<ResourcePathElement> m_resourcePathElements;

  /**
   * @brief Detect the current OS type
   */
  void detectOSType();

  /**
   * @brief Resolve user config path
   */
  QString resolveUserConfigPath() const;

  /**
   * @brief Resolve a single raw path template to absolute path
   * @param raw Raw path template (may contain env vars)
   * @param baseDir Base directory for relative path resolution
   * @param applyInstallSuffix If true, append app name + m_suffix to paths ending with /
   * @return Canonical or absolute path; empty if cannot be determined
   *
   * Common logic for all three tiers:
   * 1. Expands environment variables
   * 2. Optionally applies install folder rules (Installation tier only)
   * 3. Resolves relative paths against baseDir
   * 4. Attempts canonical path (follows symlinks), falls back to absolute
   */
  QString resolveSinglePath(const QString& raw, const QDir& baseDir, bool applyInstallSuffix) const;

  /**
   * @brief Find sibling OpenSCAD installations on this system
   * @return Vector of discovered sibling installations
   * 
   * Scans for other OpenSCAD installations:
   * - Windows: Scans Program Files for OpenSCAD* folders
   * - macOS: Scans Applications for OpenSCAD*.app bundles
   * - Linux/BSD: Returns empty (system paths, not siblings)
   */
  QVector<ResourceLocation> findSiblingInstallations() const;

  /**
   * @brief Platform-specific sibling installation discovery
   * @param osType Operating system type
   * @param applicationPath Current application path
   * @param currentFolderName Current installation folder name
   * @return Vector of discovered sibling installations
   */
  static QVector<ResourceLocation> findSiblingInstallationsForPlatform(
      ExtnOSType osType,
      const QString& applicationPath,
      const QString& currentFolderName);
};

} // namespace platformInfo

// Backward-compatible alias to ease future renaming across the codebase.
// Prefer `platformInfo::PlatformLocations` as the semantic name; it currently
// maps directly to `platformInfo::ResourcePaths`.
namespace platformInfo {
using PlatformLocations = ResourcePaths;
}
