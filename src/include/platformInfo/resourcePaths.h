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
#include <QString>
#include <QStringList>
#include <QDir>

namespace platformInfo {

/**
 * @brief Enumeration of OpenSCAD resource types
 * 
 * These correspond to the subdirectories under the resource directory
 * that contain different types of resources.
 */
enum class ResourceType {
    Examples,       ///< Example .scad files ($RESOURCEDIR/examples)
    Tests,          ///< Test .scad files ($RESOURCEDIR/tests) - may have .json/.txt/.dat attachments
    Fonts,          ///< Font files - .ttf, .otf ($RESOURCEDIR/fonts)
    ColorSchemes,   ///< Color scheme files - .json ($RESOURCEDIR/color-schemes)
    Shaders,        ///< Shader files - .frag, .vert ($RESOURCEDIR/shaders)
    Templates,      ///< Template files ($RESOURCEDIR/templates)
    Libraries,      ///< OpenSCAD library .scad scripts ($RESOURCEDIR/libraries) - extend OpenSCAD features
    Translations    ///< Translation/locale files ($RESOURCEDIR/locale)
};

/**
 * @brief Resource type metadata
 * 
 * Contains the subdirectory name, description, and file extensions for a resource type.
 */
struct PLATFORMINFO_API ResourceTypeInfo {
    ResourceType type;              ///< The resource type enum
    QString subdirectory;           ///< Subdirectory name under resource dir
    QString description;            ///< Human-readable description
    QStringList primaryExtensions;  ///< Primary file extensions for this resource
    QStringList attachmentExtensions; ///< Optional attachment file extensions
    
    ResourceTypeInfo() = default;
    ResourceTypeInfo(ResourceType t, const QString& subdir, const QString& desc,
                     const QStringList& primary, const QStringList& attachments = {})
        : type(t), subdirectory(subdir), description(desc),
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
 * 1. **Application resources** - Built-in read-only resources (resourceBasePath)
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
 * | documentsPath | $HOME/.local/share | CSIDL_PERSONAL | NSDocumentDirectory | User libraries, backups |
 * | userDocumentsPath | $XDG_DOCUMENTS_DIR | =documentsPath | =documentsPath | Default save location |
 * | userConfigPath | $XDG_CONFIG_HOME | CSIDL_LOCAL_APPDATA | NSAppSupportDir | Writable config |
 * | resourceBasePath | (from app init) | (from app init) | (from app init) | Read-only resources |
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
class PLATFORMINFO_API ResourcePaths {
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
     * @param suffix Build suffix (e.g., "" for release, " (Nightly)" for nightly builds)
     * 
     * @note The suffix corresponds to OpenSCAD's OPENSCAD_SUFFIX preprocessor define.
     * It is appended to both share paths ("../share/openscad" + suffix) and 
     * user config paths ("OpenSCAD" + suffix).
     */
    explicit ResourcePaths(const QString& applicationPath, const QString& suffix = QString());
    
    /**
     * @brief Set the application executable path
     * @param path Path to the application executable directory
     * 
     * Search paths are relative to this location.
     */
    void setApplicationPath(const QString& path);
    
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
    void setSuffix(const QString& suffix);
    
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
    static const ResourceTypeInfo* resourceTypeInfo(ResourceType type);
    
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
    
    // ========== Application Resource Paths ==========
    
    /**
     * @brief Get platform-specific search paths for application resources
     * @return List of relative paths to search for resource directory
     * 
     * These paths are relative to the application path and are searched
     * in order until a valid resource directory is found.
     */
    QStringList appSearchPaths() const;
    
    /**
     * @brief Get search paths for the current platform
     * @param osType The OS type to get paths for
     * @param suffix Build suffix for share paths (e.g., "" or " (Nightly)")
     * @return List of relative search paths
     * 
     * @note For share paths, this mimics OpenSCAD's RESOURCE_FOLDER macro:
     * RESOURCE_FOLDER("../share/openscad") expands to "../share/openscad" + suffix
     */
    static QStringList searchPathsForPlatform(ExtnOSType osType, const QString& suffix = QString());
    
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
    QString m_suffix;                         ///< Build suffix (e.g., "" or " (Nightly)")
    ExtnOSType m_osType;
    mutable QString m_cachedAppResourceDir;   ///< Cached app resource directory path
    mutable QString m_cachedUserConfigPath;   ///< Cached user config base path
    mutable bool m_appResourceDirCached = false;
    mutable bool m_userConfigPathCached = false;
    
    /**
     * @brief Detect the current OS type
     */
    void detectOSType();
    
    /**
     * @brief Build the share path with suffix (RESOURCE_FOLDER equivalent)
     * @param basePath Base path ending with "/" (e.g., "../share/")
     * @return Path with "openscad" + suffix appended (e.g., "../share/openscad (Nightly)")
     * 
     * @note Equivalent to OpenSCAD's RESOURCE_FOLDER("../share/openscad") macro
     */
    QString buildSharePath(const QString& basePath) const;
    
    /**
     * @brief Resolve user config base path (internal, with caching)
     */
    QString resolveUserConfigBasePath() const;
};

} // namespace platformInfo
