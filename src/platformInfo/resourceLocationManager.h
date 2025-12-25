/**
 * @file resourceLocationManager.h
 * @brief Three-tier resource location management for OpenSCAD
 * 
 * Manages resource locations at three levels:
 * 1. **Installation** - Hardcoded platform-specific paths (read-only)
 * 2. **Machine** - Admin-configurable paths for all users on the machine
 * 3. **User** - User-configurable paths in their own space
 * 
 * Each level has:
 * - A list of AVAILABLE locations (reference info, from config files)
 * - A list of ENABLED locations (user selections, stored in QSettings)
 * 
 * Resources are searched in order: Installation → Machine → User
 */

#pragma once

#include "export.h"
#include "extnOSVersRef.h"
#include "resourcePaths.h"
#include <QString>
#include <QStringList>
#include <QVector>
#include <QSettings>

#include "resInventory/ResourceLocation.h"

namespace platformInfo {

/**
 * @brief Manages three-tier resource locations with QSettings persistence
 * 
 * ## Architecture
 * 
 * ### Tier 1: Installation Locations
 * - Hardcoded in platform-specific code
 * - Read-only, cannot be modified by users or admins
 * - Searched first (highest priority for bundled resources)
 * - Examples: app bundle Resources folder, ../share/openscad
 * 
 * ### Tier 2: Machine Locations  
 * - Available locations defined in a config file in admin-protected location
 * - Enabled locations stored in machine-level QSettings (HKLM on Windows)
 * - Modifiable only by system administrators
 * - Typical paths:
 *   - Windows: C:/ProgramData/OpenSCAD
 *   - macOS: /Library/Application Support/OpenSCAD
 *   - Linux: /usr/local/share/openscad, /opt/openscad
 * 
 * ### Tier 3: User Locations
 * - Available locations defined in user's config file
 * - Enabled locations stored in user-level QSettings
 * - Fully user-controllable
 * - Typical paths:
 *   - Windows: ~/AppData/Local/OpenSCAD, ~/AppData/Roaming/OpenSCAD, ~/Documents/OpenSCAD
 *   - macOS: ~/Library/Application Support/OpenSCAD, ~/Documents/OpenSCAD  
 *   - Linux: ~/.config/openscad, ~/.local/share/openscad
 * 
 * ## Config File Format (JSON)
 * ```json
 * {
 *   "locations": [
 *     {
 *       "path": "/Library/Application Support/OpenSCAD",
 *       "displayName": "System OpenSCAD Resources",
 *       "description": "Shared resources for all users"
 *     }
 *   ]
 * }
 * ```
 * 
 * ## QSettings Keys
 * - Machine: "Resources/MachinePaths" (QStringList of enabled paths)
 * - User: "Resources/UserPaths" (QStringList of enabled paths)
 */
class RESOURCEMGMT_API ResourceLocationManager {
public:
    /**
     * @brief Construct with optional settings
     * @param settings QSettings instance to use (nullptr = create default)
     * @param suffix OpenSCAD build suffix (e.g., "", " (Nightly)")
     * 
     * If settings is nullptr, creates a default QSettings with:
     * - Organization: "OpenSCAD"
     * - Application: "OpenSCAD" + suffix
     */
    explicit ResourceLocationManager(QSettings* settings = nullptr, 
                                      const QString& suffix = QString());
    
    ~ResourceLocationManager();
    
    // ========== Initialization ==========
    
    /**
     * @brief Initialize with application path
     * @param applicationPath Path to application executable directory
     * 
     * Must be called before using installation-relative paths.
     */
    void setApplicationPath(const QString& applicationPath);
    
    /**
     * @brief Get the application path
     */
    QString applicationPath() const { return m_applicationPath; }
    
    /**
     * @brief Set the build suffix
     * @param suffix Build suffix (e.g., "" or " (Nightly)")
     */
    void setSuffix(const QString& suffix);
    
    /**
     * @brief Get the build suffix
     */
    QString suffix() const { return m_suffix; }
    
    /**
     * @brief Get the folder name (OpenSCAD + suffix)
     */
    QString folderName() const;
    
    // ========== Installation Validation ==========
    
    /**
     * @brief Check if a directory is a valid OpenSCAD installation
     * @param path Path to check (e.g., C:/Program Files/OpenSCAD)
     * @return true if this looks like an OpenSCAD installation directory
     * 
     * Validates by checking for:
     * - openscad.com (Windows) or openscad executable
     * - At least one resource subdirectory (examples, fonts, libraries, etc.)
     */
    static bool isValidInstallation(const QString& path);
    
    /**
     * @brief Check if this app is running from a valid OpenSCAD installation
     * @return true if current application path is a valid installation
     */
    bool isRunningFromValidInstallation() const;
    
    /**
     * @brief Get the default installation search path for file dialogs
     * @return Platform-specific default (e.g., "C:/Program Files/OpenSCAD")
     */
    QString defaultInstallationSearchPath() const;
    
    /**
     * @brief Get/set user-specified installation path (for non-installed runs)
     * 
     * When the app is not running from a valid OpenSCAD installation,
     * the user can specify an installation path manually. This is saved
     * to QSettings and restored on next run.
     */
    QString userSpecifiedInstallationPath() const;
    void setUserSpecifiedInstallationPath(const QString& path);
    
    /**
     * @brief Get the effective installation path
     * @return User-specified path if set and valid, else application path if valid, else empty
     */
    QString effectiveInstallationPath() const;
    
    // ========== Installation Locations (Tier 1) ==========
    
    /**
     * @brief Get installation search paths (hardcoded, read-only)
     * @return List of relative paths to search from application directory
     * 
     * These are the same paths used by OpenSCAD's PlatformUtils to find
     * the built-in resource directory. They cannot be modified.
     */
    QStringList installationSearchPaths() const;
    
    /**
     * @brief Find the active installation resource directory
     * @return Absolute path to found resource directory, or empty if not found
     */
    QString findInstallationResourceDir() const;
    
    /**
     * @brief Find sibling OpenSCAD installations
     * @return List of sibling installation locations (not including current)
     * 
     * On Windows: Scans parent of application path (e.g., C:\Program Files\)
     *             for other OpenSCAD* folders
     * On macOS: Scans /Applications/ for other OpenSCAD*.app bundles
     * On Linux: Returns empty (installations are in system paths, not siblings)
     * 
     * Examples:
     * - Running "OpenSCAD (Nightly)" finds sibling "OpenSCAD"
     * - Running "OpenSCAD" finds sibling "OpenSCAD (Nightly)"
     */
    QVector<ResourceLocation> findSiblingInstallations() const;
    
    /**
     * @brief Find sibling installations for a specific platform
     * @param osType Target platform
     * @param applicationPath Path to current application
     * @param currentFolderName Current app's folder name (to exclude)
     * @return List of sibling installation locations
     */
    static QVector<ResourceLocation> findSiblingInstallationsForPlatform(
        ExtnOSType osType,
        const QString& applicationPath,
        const QString& currentFolderName);
    
    /**
     * @brief Get enabled sibling installation paths from QSettings
     * @return List of paths that have been enabled by the user
     */
    QStringList enabledSiblingPaths() const;
    
    /**
     * @brief Set which sibling installations are enabled
     * @param paths List of paths to enable
     */
    void setEnabledSiblingPaths(const QStringList& paths);
    
    // ========== Machine Locations (Tier 2) ==========
    
    /**
     * @brief Get default machine-level locations for the current platform
     * @return List of default machine resource locations
     * 
     * Platform defaults:
     * - Windows: C:/ProgramData/OpenSCAD[suffix]
     * - macOS: /Library/Application Support/OpenSCAD[suffix]
     * - Linux: /usr/local/share/openscad[suffix], /opt/openscad[suffix]
     */
    QVector<ResourceLocation> defaultMachineLocations() const;
    
    /**
     * @brief Get default machine locations for a specific platform
     */
    static QVector<ResourceLocation> defaultMachineLocationsForPlatform(
        ExtnOSType osType, const QString& suffix = QString());
    
    /**
     * @brief Get path to machine-level config file
     * @return Path where machine locations config should be stored
     * 
     * Platform paths:
     * - Windows: C:/ProgramData/OpenSCAD/locations.json
     * - macOS: /Library/Application Support/OpenSCAD/locations.json
     * - Linux: /etc/openscad/locations.json or /usr/local/etc/openscad/locations.json
     */
    QString machineConfigFilePath() const;
    
    /**
     * @brief Get available machine locations (from config file)
     * @return All locations defined in machine config, with exists/writable status
     */
    QVector<ResourceLocation> availableMachineLocations() const;
    
    /**
     * @brief Get enabled machine locations (from QSettings)
     * @return Only the locations that are currently enabled
     */
    QVector<ResourceLocation> enabledMachineLocations() const;
    
    /**
     * @brief Set which machine locations are enabled
     * @param paths List of paths to enable (must exist in available locations)
     * 
     * Saves to system-level QSettings (requires admin on some platforms).
     */
    void setEnabledMachineLocations(const QStringList& paths);
    
    /**
     * @brief Load machine locations from config file
     * @return true if config file was found and loaded
     */
    bool loadMachineLocationsConfig();
    
    /**
     * @brief Save machine locations to config file
     * @param locations Locations to save
     * @return true if saved successfully (requires admin permissions)
     */
    bool saveMachineLocationsConfig(const QVector<ResourceLocation>& locations);
    
    // ========== User Locations (Tier 3) ==========
    
    /**
     * @brief Get default user-level locations for the current platform
     * @return List of default user resource locations
     * 
     * Platform defaults:
     * - Windows: ~/AppData/Roaming/OpenSCAD, ~/Documents/OpenSCAD
     * - macOS: ~/Library/Application Support/OpenSCAD, ~/Documents/OpenSCAD
     * - Linux: ~/.config/openscad, ~/.local/share/openscad, ~/Documents/OpenSCAD
     */
    QVector<ResourceLocation> defaultUserLocations() const;
    
    /**
     * @brief Get default user locations for a specific platform
     */
    static QVector<ResourceLocation> defaultUserLocationsForPlatform(
        ExtnOSType osType, const QString& suffix = QString());
    
    /**
     * @brief Get path to user-level config file
     * @return Path where user locations config is stored
     * 
     * Stored in user's config directory:
     * - Windows: ~/AppData/Roaming/OpenSCAD/locations.json
     * - macOS: ~/Library/Application Support/OpenSCAD/locations.json
     * - Linux: ~/.config/openscad/locations.json
     */
    QString userConfigFilePath() const;
    
    /**
     * @brief Get available user locations (from config file)
     * @return All locations defined in user config, with exists/writable status
     */
    QVector<ResourceLocation> availableUserLocations() const;
    
    /**
     * @brief Get enabled user locations (from QSettings)
     * @return Only the locations that are currently enabled
     */
    QVector<ResourceLocation> enabledUserLocations() const;
    
    /**
     * @brief Set which user locations are enabled
     * @param paths List of paths to enable
     * 
     * Saves to user-level QSettings.
     */
    void setEnabledUserLocations(const QStringList& paths);
    
    /**
     * @brief Load user locations from config file
     * @return true if config file was found and loaded
     */
    bool loadUserLocationsConfig();
    
    /**
     * @brief Save user locations to config file
     * @param locations Locations to save
     * @return true if saved successfully
     */
    bool saveUserLocationsConfig(const QVector<ResourceLocation>& locations);
    
    /**
     * @brief Add a new user location
     * @param location Location to add
     * @param enable Whether to enable it immediately
     * @return true if added successfully
     */
    bool addUserLocation(const ResourceLocation& location, bool enable = true);
    
    /**
     * @brief Remove a user location
     * @param path Path of location to remove
     * @return true if removed successfully
     */
    bool removeUserLocation(const QString& path);
    
    // ========== Combined Resource Resolution ==========
    
    /**
     * @brief Get all enabled locations in search order
     * @return Combined list: Installation → Machine → User
     * 
     * Only includes locations that exist on disk.
     */
    QVector<ResourceLocation> allEnabledLocations() const;
    
    /**
     * @brief Get all paths for a specific resource type
     * @param type The resource type
     * @return List of absolute paths to resource type directories
     * 
     * Returns paths in search order (installation first, user last).
     * Only includes paths that exist on disk.
     */
    QStringList resourcePathsForType(ResourceType type) const;
    
    /**
     * @brief Find the first existing path for a resource type
     * @param type The resource type
     * @return First found path, or empty if none exist
     */
    QString findResourcePath(ResourceType type) const;
    
    /**
     * @brief Get the writable user path for a resource type
     * @param type The resource type
     * @return Path where user can save resources of this type
     * 
     * Returns the first enabled user location + resource subdirectory.
     * Creates the directory if it doesn't exist.
     */
    QString writableUserPath(ResourceType type) const;
    
    // ========== Refresh and Validation ==========
    
    /**
     * @brief Refresh exists/writable status of all locations
     * 
     * Call after file system changes or when displaying in UI.
     */
    void refreshLocationStatus();
    
    /**
     * @brief Reload all configuration
     * 
     * Reloads both machine and user config files and QSettings.
     */
    void reloadConfiguration();
    
    /**
     * @brief Get the detected OS type
     */
    ExtnOSType osType() const { return m_osType; }

private:
    QString m_applicationPath;
    QString m_suffix;
    ExtnOSType m_osType;
    QSettings* m_settings;
    bool m_ownsSettings;
    
    // Cached locations
    mutable QVector<ResourceLocation> m_machineLocations;
    mutable QVector<ResourceLocation> m_userLocations;
    mutable bool m_machineLocationsLoaded = false;
    mutable bool m_userLocationsLoaded = false;
    
    // Cached installation resource dir
    mutable QString m_cachedInstallDir;
    mutable bool m_installDirCached = false;
    
    void detectOSType();
    void initializeSettings();
    
    // Config file I/O
    static QVector<ResourceLocation> loadLocationsFromJson(const QString& filePath);
    static bool saveLocationsToJson(const QString& filePath, 
                                    const QVector<ResourceLocation>& locations);
    
    // Update exists/writable status
    static void updateLocationStatus(ResourceLocation& loc);
    static void updateLocationStatuses(QVector<ResourceLocation>& locations);
    
    // QSettings keys
    static constexpr const char* KEY_MACHINE_PATHS = "Resources/MachinePaths";
    static constexpr const char* KEY_USER_PATHS = "Resources/UserPaths";
    static constexpr const char* KEY_SIBLING_PATHS = "Resources/SiblingInstallPaths";
    static constexpr const char* KEY_USER_INSTALL_PATH = "Resources/UserSpecifiedInstallPath";
    static constexpr const char* CONFIG_FILENAME = "locations.json";
};

} // namespace platformInfo
