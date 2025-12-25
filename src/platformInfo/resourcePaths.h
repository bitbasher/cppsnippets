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

class QSettings;

namespace platformInfo {

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
 * @brief Enumeration of OpenSCAD resource types
 *
 * These correspond to the subdirectories under the resource directory
 * that contain different types of resources.
 */
enum class ResourceType {
  Examples, ///< Example .scad files ($RESOURCEDIR/examples)
  Group,    ///< Grouping type - not an actual resource - names to be captured
  Tests, ///< Test .scad files ($RESOURCEDIR/tests) - may have .json/.txt/.dat
         ///< attachments
  Fonts, ///< Font files - .ttf, .otf ($RESOURCEDIR/fonts)
  ColorSchemes, ///< Color scheme folder - no files ($RESOURCEDIR/color-schemes)
  EditorColors, ///< Color scheme files for the editor - .json
                ///< ($RESOURCEDIR/color-schemes)
  RenderColors, ///< Color scheme files for 3D View - .json
                ///< ($RESOURCEDIR/color-schemes)
  Shaders,      ///< Shader files - .frag, .vert ($RESOURCEDIR/shaders)
  Templates,    ///< Template files ($RESOURCEDIR/templates)
  Libraries,    ///< OpenSCAD library .scad scripts ($RESOURCEDIR/libraries) -
                ///< extend OpenSCAD features
  Translations  ///< Translation/locale files ($RESOURCEDIR/locale)
};

static const QString groupNameCapture = QStringLiteral("__capture__");

/**
 * @brief Resource type metadata
 *
 * Contains the subdirectory name, description, and file extensions for a
 * resource type.
 */
struct RESOURCEMGMT_API ResourceTypeInfo {
  ResourceType type;    ///< The resource type enum
  QString subdirectory; ///< Subdirectory name under resource dir
  QString description;  ///< Human-readable description
  QVector<ResourceType>
      subResTypes; ///< Sub-resource types contained within this resource type
  QStringList primaryExtensions; ///< Primary file extensions for this resource
  QStringList attachmentExtensions; ///< Optional attachment file extensions

  ResourceTypeInfo() = default;
  ResourceTypeInfo(ResourceType t, const QString &subdir, const QString &desc,
                   const QVector<ResourceType> &subRes = {},
                   const QStringList &primary = {},
                   const QStringList &attachments = {})
      : type(t), subdirectory(subdir), description(desc), subResTypes(subRes),
        primaryExtensions(primary), attachmentExtensions(attachments) {}
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
 * - `OPENSCAD_FOLDER_NAME` - "OpenSCAD" + OPENSCAD_SUFFIX for user paths
 * - `RESOURCE_FOLDER(path)` - Appends OPENSCAD_SUFFIX to share paths
 *
 * Example with suffix " (Nightly)":
 * - `OPENSCAD_FOLDER_NAME` → "OpenSCAD (Nightly)"
 * - `RESOURCE_FOLDER("../share/openscad")` → "../share/openscad (Nightly)"
 *
 * This class mirrors this behavior via the `suffix` parameter.
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
   * define. It is appended to both share paths ("../share/openscad" + suffix)
   * and user config paths ("OpenSCAD" + suffix).
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
   * This suffix is appended to:
   * - Share paths: "../share/openscad" + suffix (via RESOURCE_FOLDER macro)
   * - User config paths: "OpenSCAD" + suffix (via OPENSCAD_FOLDER_NAME)
   */
  void setSuffix(const QString &suffix);

  /**
   * @brief Get the build suffix
   * @return Current suffix (e.g., "" or " (Nightly)")
   */
  QString suffix() const { return m_suffix; }

  /**
   * @brief Get the OpenSCAD folder name (for user paths)
   * @return "OpenSCAD" + suffix
   *
   * This is equivalent to OpenSCAD's OPENSCAD_FOLDER_NAME macro.
   */
  QString folderName() const;

  // ========== Resource Type Information ==========

  /**
   * @brief Get all resource type definitions
   * @return Vector of all resource type info
   */
  static QVector<ResourceTypeInfo> allResourceTypes();

  /**
   * @brief Get info for a specific resource type
   * @param type The resource type
   * @return Pointer to resource type info, or nullptr if not found
   */
  static const ResourceTypeInfo *resourceTypeInfo(ResourceType type);

  /**
   * @brief Get the subdirectory name for a resource type
   * @param type The resource type
   * @return Subdirectory name (e.g., "examples", "fonts")
   */
  static QString resourceSubdirectory(ResourceType type);

  /**
   * @brief Get primary file extensions for a resource type
   * @param type The resource type
   * @return List of file extensions (e.g., {".scad"}, {".ttf", ".otf"})
   */
  static QStringList resourceExtensions(ResourceType type);

  /**
   * @brief Get all top-level resource types
   * @return Immutable vector of top-level resource types that can be discovered
   *
   * Returns the list of resource types that can appear at the top level of
   * resource directories and inside libraries. Excludes sub-resource types
   * like EditorColors and RenderColors which are nested under ColorSchemes.
   *
   * @note This list is read-only and cannot be modified.
   */
  static QVector<ResourceType> allTopLevelResourceTypes();

  // ========== Default Search Paths (Immutable) ==========
  //
  // These are compile-time constants defining where to look for resources.
  // Used as the "Restore Defaults" source in preferences.
  // User cannot modify these - they modify their selection in ResLocMap.

  /**
   * @brief Get default installation search paths for this platform
   * @return Immutable list of relative paths to search for installation
   * resources
   *
   * These paths are relative to the application executable directory.
   * This is a compile-time constant list that cannot be modified by the user.
   * Used as the default/reset source for the Installation tier in preferences.
   *
   * @note Paths may include the build suffix (e.g., " (Nightly)") for share
   * paths.
   */
  static const QStringList &defaultInstallSearchPaths();

  /**
   * @brief Get default machine-level search paths for this platform
   * @return Immutable list of absolute paths for machine-wide resources
   *
   * These are system-wide locations accessible to all users.
   * This is a compile-time constant list that cannot be modified by the user.
   * Used as the default/reset source for the Machine tier in preferences.
   */
  static const QStringList &defaultMachineSearchPaths();

  /**
   * @brief Get default user-level search paths for this platform
   * @return Immutable list of paths for user-specific resources
   *
   * These are locations in the user's home/config directory.
   * This is a compile-time constant list that cannot be modified by the user.
   * Used as the default/reset source for the User tier in preferences.
   *
   * @note Paths are relative to userConfigBasePath() or userDocumentsPath().
   */
  static const QStringList &defaultUserSearchPaths();
  
  /**
   * @brief Get expanded user search paths with env vars resolved
   * @return List of expanded paths
   * 
   * Expands any ${VAR} or %VAR% templates in defaultUserSearchPaths()
   * using current environment variables and user overrides.
   */
  QStringList expandedUserSearchPaths() const;
  
  /**
   * @brief Get expanded machine search paths with env vars resolved
   * @return List of expanded paths
   * 
   * Expands any ${VAR} or %VAR% templates in defaultMachineSearchPaths()
   * using current environment variables and user overrides.
   */
  QStringList expandedMachineSearchPaths() const;

private:
  // ========== Platform-Specific Search Path Constants ==========
  // These static members are defined inline with #ifdef for each platform

  /// Installation tier: relative to application executable
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
  inline static const QStringList s_defaultInstallSearchPaths = {
      QStringLiteral("."),         // Release location (same as exe)
      QStringLiteral("../share/"), // MSYS2 style Base share folder (append
                                   // openscad + suffix)
      QStringLiteral("..")         // Dev location
  };
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
  inline static const QStringList s_defaultInstallSearchPaths = {
      QStringLiteral("../Resources"), // Resources bundled in .app
      QStringLiteral("../../.."),     // Dev location
      QStringLiteral("../../../.."),  // Test location (cmake)
      QStringLiteral(".."),           // Test location
      QStringLiteral(
          "../share/") // Base share folder (append openscad + suffix)
  };
#else // Linux/BSD/POSIX
  inline static const QStringList s_defaultInstallSearchPaths = {
      QStringLiteral(
          "../share/"), // Standard install (append openscad + suffix)
      QStringLiteral(
          "../../share/"),    // Alternate install (append openscad + suffix)
      QStringLiteral("."),    // Build directory
      QStringLiteral(".."),   // Up one
      QStringLiteral("../..") // Up two
  };
#endif

  /// Machine tier: system-wide locations for all users
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
  inline static const QStringList s_defaultMachineSearchPaths = {
      QStringLiteral("%PROGRAMDATA%/CppSnippets"), // All users app data (template)
      QStringLiteral("C:/ProgramData/")             // Fallback: legacy absolute path
  };
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
  inline static const QStringList s_defaultMachineSearchPaths = {
      QStringLiteral("/Library/Application Support/CppSnippets") // System-wide
  };
#else // Linux/BSD/POSIX
  inline static const QStringList s_defaultMachineSearchPaths = {
      QStringLiteral("/usr/share/"),         // System packages
      QStringLiteral("/usr/local/share/"),   // Local installs
      QStringLiteral("/opt/openscad/share/") // Opt installs
  };
#endif

  /// User tier: base directories where user resources can be located
  /// Resolved using userConfigBasePath(), userDocumentsPath(), etc.
  /// Paths ending with "/" have the app name + suffix appended; others are
  /// scanned directly
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
  inline static const QStringList s_defaultUserSearchPaths = {
      QStringLiteral("%APPDATA%/CppSnippets"),      // User roaming app data (template)
      QStringLiteral("%LOCALAPPDATA%/CppSnippets"), // User local app data (template)
      QStringLiteral("."),                           // Current directory
      QStringLiteral("../")                          // User documents base (legacy)
  };
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
  inline static const QStringList s_defaultUserSearchPaths = {
      QStringLiteral("${HOME}/Library/Application Support/CppSnippets"), // User app support (template)
      QStringLiteral("."),                                                 // Current directory
      QStringLiteral("../../Documents/")                                  // User documents (legacy)
  };
#else // Linux/BSD/POSIX
  inline static const QStringList s_defaultUserSearchPaths = {
      QStringLiteral("${XDG_CONFIG_HOME}/cppsnippets"),    // XDG config (template)
      QStringLiteral("${HOME}/.config/cppsnippets"),       // Fallback config (template)
      QStringLiteral("${HOME}/.local/share/cppsnippets"),  // XDG data (template)
      QStringLiteral("."),                                  // Current directory
      QStringLiteral("../../.local/share/")                // Legacy relative path
  };
#endif

    // ===== Shared constants used by resource type definitions =====
    inline static const QVector<ResourceType> s_allTopLevelResTypes = {
      ResourceType::Examples,
      ResourceType::Tests,
      ResourceType::Fonts,
      ResourceType::ColorSchemes,
      ResourceType::Shaders,
      ResourceType::Templates,
      ResourceType::Libraries,
      ResourceType::Translations
    };

    inline static const QVector<ResourceType> s_exampleSubResTypes = {
      ResourceType::Group, ResourceType::Templates};

    inline static const QVector<ResourceType> s_testSubResTypes = {
      ResourceType::Templates};

    inline static const QStringList s_attachmentsList = {
      QStringLiteral(".json"), QStringLiteral(".txt"), QStringLiteral(".dat"),
      QStringLiteral(".png"),  QStringLiteral(".jpg"), QStringLiteral(".jpeg"),
      QStringLiteral(".svg"),  QStringLiteral(".gif"), QStringLiteral(".csv"),
      QStringLiteral(".stl"),  QStringLiteral(".off"), QStringLiteral(".dxf")};

    // Static resource type definitions with file extensions
    inline static const QVector<ResourceTypeInfo> s_resourceTypes = {
      {ResourceType::Examples, // container, but may contain resources directly
       QStringLiteral("examples"),
       QStringLiteral("Example scripts"),
       s_exampleSubResTypes, // groups are just folder with .scad files in them
       {QStringLiteral(".scad")},
       s_attachmentsList},

      {ResourceType::Group, // container of resources
       groupNameCapture,
       QStringLiteral("Editor Categories"),
       {}, // no sub-resources
       {QStringLiteral(".scad")},
      s_attachmentsList},

      {ResourceType::Tests, // container, but may contain resources directly
       QStringLiteral("tests"),
       QStringLiteral("Test OpenSCAD scripts"),
      s_testSubResTypes, // might have templates
       {QStringLiteral(".scad")},
      s_attachmentsList},

      {ResourceType::Fonts,
       QStringLiteral("fonts"),
       QStringLiteral("Font files (supplements OS fonts)"),
       {}, // no sub-resources
       {QStringLiteral(".ttf"), QStringLiteral(".otf")},
       {}},

      {ResourceType::ColorSchemes, // container only
       QStringLiteral("color-schemes"),
       QStringLiteral("Color scheme definitions"),
       QVector<ResourceType>{
           ResourceType::EditorColors,
           ResourceType::RenderColors}, // contains editor and render colors
       {}, // no primary extensions (container only)
       {}},

      {ResourceType::EditorColors, // scheme definitions
       QStringLiteral("editor"),
       QStringLiteral("Editor color schemes"),
       {}, // no sub-resources
       {QStringLiteral(".json")},
       {}},

      {ResourceType::RenderColors, // scheme definitions
       QStringLiteral("render"),
       QStringLiteral("Render color schemes"),
       {}, // no sub-resources
       {QStringLiteral(".json")},
       {}},

      {ResourceType::Shaders,
       QStringLiteral("shaders"),
       QStringLiteral("OpenGL shader files"),
       {}, // no sub-resources
       {QStringLiteral(".frag"), QStringLiteral(".vert")},
       {}},

      {ResourceType::Templates,
       QStringLiteral("templates"),
       QStringLiteral("Template files"),
       {}, // no sub-resources
       {QStringLiteral(".json")},
       {}},

      {ResourceType::Libraries,
       QStringLiteral("libraries"),
       QStringLiteral("OpenSCAD library scripts that extend features"),
         s_allTopLevelResTypes, // libraries can contain any top-level resource
       {QStringLiteral(".scad")},
       {}},

      {ResourceType::Translations,
       QStringLiteral("locale"),
       QStringLiteral("Translation files"),
       {}, // no sub-resources
       {QStringLiteral(".qm"), QStringLiteral(".ts")},
       {}}};

public:
  /**
   * @brief Get platform-specific search paths for application resources
   * @return List of relative paths to search for resource directory
   *
   * Convenience method that returns defaultInstallSearchPaths().
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
   * @brief Get the user config base path (platform-specific)
   * @return Path to writable config directory (without OpenSCAD folder)
   *
   * Platform behavior (matches OpenSCAD PlatformUtils::userConfigPath base):
   * - Linux: $XDG_CONFIG_HOME, fallback $HOME/.config
   * - Windows: CSIDL_LOCAL_APPDATA
   * - macOS: NSApplicationSupportDirectory (~Library/Application Support)
   *
   * @note This returns only the BASE path. OpenSCAD's userConfigPath()
   * appends OPENSCAD_FOLDER_NAME to this.
   */
  QString userConfigBasePath() const;

  /**
   * @brief Get the user's OpenSCAD config directory
   * @return Path to user's OpenSCAD folder (userConfigBasePath/folderName)
   *
   * @note Equivalent to OpenSCAD's PlatformUtils::userConfigPath()
   */
  QString userOpenSCADPath() const;

  /**
   * @brief Get the full path to a user resource type directory
   * @param type The resource type
   * @return Absolute path to the user resource type directory
   *
   * Returns the path even if it doesn't exist yet (for creation).
   */
  QString userResourcePath(ResourceType type) const;

  /**
   * @brief Check if a user resource directory exists
   * @param type The resource type
   * @return true if the user resource directory exists
   */
  bool hasUserResourceDirectory(ResourceType type) const;

  /**
   * @brief Get user config base path for a specific platform
   * @param osType The OS type
   * @return Platform-specific config base path (without folder name)
   *
   * Platform behavior:
   * - Linux: $XDG_CONFIG_HOME or $HOME/.config
   * - Windows: CSIDL_LOCAL_APPDATA
   * - macOS: NSApplicationSupportDirectory
   */
  static QString userConfigBasePathForPlatform(ExtnOSType osType);

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
   * @brief Get all paths for a resource type (app + user)
   * @param type The resource type
   * @return List of paths to search (app path first, then user path)
   */
  QStringList allResourcePaths(ResourceType type) const;
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
   * @brief Get the detected OS type
   * @return Current OS type used for search path selection
   */
  ExtnOSType osType() const { return m_osType; }

private:
  QString m_applicationPath;
  QString m_suffix; ///< Build suffix (e.g., "" or " (Nightly)")
  ExtnOSType m_osType;
  mutable QString
      m_cachedAppResourceDir; ///< Cached app resource directory path
  mutable QString m_cachedUserConfigPath; ///< Cached user config base path
  mutable bool m_appResourceDirCached = false;
  mutable bool m_userConfigPathCached = false;

  QList<EnvVarEntry> m_envVars; ///< User/config overrides applied before system env

  // Tier-tagged search locations (single list; order preserves priority)
  QList<ResourcePathElement> m_resourcePathElements;

  /**
   * @brief Detect the current OS type
   */
  void detectOSType();

  /**
   * @brief Resolve user config base path (internal, with caching)
   */
  QString resolveUserConfigBasePath() const;
};

} // namespace platformInfo
