/**
 * @file resourceLocationManager.cpp
 * @brief Implementation of ResourceLocationManager
 */

#include "platformInfo/resourceLocationManager.h"
#include "resInventory/ResourceLocation.h"
#include "platformInfo/platformInfo.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QCoreApplication>

#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
#include <shlobj.h>
#include <windows.h>
#endif

namespace platformInfo {

// ============================================================================
// Construction / Destruction
// ============================================================================

ResourceLocationManager::ResourceLocationManager(QSettings* settings, const QString& suffix)
    : m_suffix(suffix)
    , m_osType(ExtnOSType::Unknown)
    , m_settings(settings)
    , m_ownsSettings(settings == nullptr)
{
    detectOSType();
    initializeSettings();
}

ResourceLocationManager::~ResourceLocationManager() {
    if (m_ownsSettings && m_settings) {
        delete m_settings;
    }
}

void ResourceLocationManager::detectOSType() {
    PlatformInfo info;
    m_osType = info.currentOSType();
}

void ResourceLocationManager::initializeSettings() {
    if (!m_settings && m_ownsSettings) {
        // Create default QSettings for OpenSCAD
        QString appName = QStringLiteral("OpenSCAD") + m_suffix;
        m_settings = new QSettings(QStringLiteral("OpenSCAD"), appName);
    }
}

// ============================================================================
// Basic Properties
// ============================================================================

void ResourceLocationManager::setApplicationPath(const QString& applicationPath) {
    if (m_applicationPath != applicationPath) {
        m_applicationPath = applicationPath;
        m_installDirCached = false;
        m_cachedInstallDir.clear();
    }
}

void ResourceLocationManager::setSuffix(const QString& suffix) {
    if (m_suffix != suffix) {
        m_suffix = suffix;
        m_installDirCached = false;
        m_cachedInstallDir.clear();
        m_machineLocationsLoaded = false;
        m_userLocationsLoaded = false;
    }
}

QString ResourceLocationManager::folderName() const {
    return QStringLiteral("OpenSCAD") + m_suffix;
}

// ============================================================================
// Installation Validation
// ============================================================================

bool ResourceLocationManager::isValidInstallation(const QString& path) {
    if (path.isEmpty()) return false;
    
    QDir dir(path);
    if (!dir.exists()) return false;
    
    // Check for OpenSCAD executable
    // Windows: openscad.com or openscad.exe
    // macOS: Inside .app bundle, check for openscad in Contents/MacOS
    // Linux: openscad executable
    bool hasExecutable = false;
    
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    hasExecutable = QFile::exists(dir.absoluteFilePath(QStringLiteral("openscad.com"))) ||
                    QFile::exists(dir.absoluteFilePath(QStringLiteral("openscad.exe")));
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
    // For .app bundles, we might be pointing to the .app or inside it
    if (path.endsWith(QStringLiteral(".app"), Qt::CaseInsensitive)) {
        hasExecutable = QFile::exists(dir.absoluteFilePath(QStringLiteral("Contents/MacOS/OpenSCAD")));
    } else {
        hasExecutable = QFile::exists(dir.absoluteFilePath(QStringLiteral("openscad")));
    }
#else
    hasExecutable = QFile::exists(dir.absoluteFilePath(QStringLiteral("openscad")));
#endif

    if (!hasExecutable) return false;
    
    // Check for at least one resource subdirectory
    // Use the canonical list of top-level resource types from ResourcePaths
    const QVector<ResourceType> topLevelTypes = ResourcePaths::allTopLevelResourceTypes();
    
    // Check in the directory itself
    for (const ResourceType& type : topLevelTypes) {
        QString subdir = ResourcePaths::resourceSubdirectory(type);
        if (!subdir.isEmpty() && dir.exists(subdir)) {
            return true;
        }
    }
    
    // Check in common resource locations
    QStringList resourcePaths = {
        QStringLiteral("share/openscad"),
        QStringLiteral("Resources"),          // macOS .app bundle
        QStringLiteral("Contents/Resources")  // macOS .app bundle from outside
    };
    
    for (const QString& resPath : resourcePaths) {
        QDir resDir(dir.absoluteFilePath(resPath));
        if (resDir.exists()) {
            for (const ResourceType& type : topLevelTypes) {
                QString subdir = ResourcePaths::resourceSubdirectory(type);
                if (!subdir.isEmpty() && resDir.exists(subdir)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool ResourceLocationManager::isRunningFromValidInstallation() const {
    if (m_applicationPath.isEmpty()) return false;
    
    // Navigate up from application path to find the installation root
    QDir appDir(m_applicationPath);
    
    // Check current directory and parent directories
    QString current = appDir.absolutePath();
    for (int i = 0; i < 3; ++i) { // Check up to 3 levels up
        if (isValidInstallation(current)) {
            return true;
        }
        QDir dir(current);
        if (!dir.cdUp()) break;
        QString parent = dir.absolutePath();
        if (parent == current) break;
        current = parent;
    }
    
    return false;
}

QString ResourceLocationManager::defaultInstallationSearchPath() const {
    switch (m_osType) {
        case ExtnOSType::Windows:
            return QStringLiteral("C:/Program Files/OpenSCAD");
        case ExtnOSType::MacOS:
            return QStringLiteral("/Applications/OpenSCAD.app");
        case ExtnOSType::Linux:
        case ExtnOSType::BSD:
        case ExtnOSType::Solaris:
        default:
            return QStringLiteral("/usr/share/openscad");
    }
}

QString ResourceLocationManager::userSpecifiedInstallationPath() const {
    if (!m_settings) return QString();
    return m_settings->value(QLatin1String(KEY_USER_INSTALL_PATH)).toString();
}

void ResourceLocationManager::setUserSpecifiedInstallationPath(const QString& path) {
    if (m_settings) {
        if (path.isEmpty()) {
            m_settings->remove(QLatin1String(KEY_USER_INSTALL_PATH));
        } else {
            m_settings->setValue(QLatin1String(KEY_USER_INSTALL_PATH), path);
        }
        m_settings->sync();
        
        // Invalidate caches since installation path changed
        m_installDirCached = false;
        m_cachedInstallDir.clear();
    }
}

QString ResourceLocationManager::effectiveInstallationPath() const {
    // First try user-specified path
    QString userPath = userSpecifiedInstallationPath();
    if (!userPath.isEmpty() && isValidInstallation(userPath)) {
        return userPath;
    }
    
    // Then try application path
    if (isRunningFromValidInstallation()) {
        return m_applicationPath;
    }
    
    return QString();
}

// ============================================================================
// Installation Locations (Tier 1)
// ============================================================================

QStringList ResourceLocationManager::installationSearchPaths() const {
    // Use the immutable default paths, applying suffix to share paths
    QStringList paths;
    for (const QString& path : ResourcePaths::defaultInstallSearchPaths()) {
        if (path.contains(QStringLiteral("share/openscad"))) {
            paths << (path + m_suffix);
        } else {
            paths << path;
        }
    }
    return paths;
}

QString ResourceLocationManager::findInstallationResourceDir() const {
    if (m_installDirCached) {
        return m_cachedInstallDir;
    }
    
    // Try effective installation path first (user-specified or app path)
    QString effectivePath = effectiveInstallationPath();
    QString searchBase = effectivePath.isEmpty() ? m_applicationPath : effectivePath;
    
    if (searchBase.isEmpty()) {
        return QString();
    }
    
    QDir appDir(searchBase);
    const QStringList paths = installationSearchPaths();
    
    for (const QString& relPath : paths) {
        QString absPath = appDir.absoluteFilePath(relPath);
        QDir testDir(absPath);
        
        // Check if this looks like a valid resource directory
        // (contains at least one expected subdirectory)
        if (testDir.exists()) {
            if (testDir.exists(QStringLiteral("examples")) ||
                testDir.exists(QStringLiteral("fonts")) ||
                testDir.exists(QStringLiteral("locale")) ||
                testDir.exists(QStringLiteral("libraries"))) {
                m_cachedInstallDir = testDir.absolutePath();
                m_installDirCached = true;
                return m_cachedInstallDir;
            }
        }
    }
    
    m_installDirCached = true;
    return QString();
}

// ============================================================================
// Sibling Installation Detection
// ============================================================================

QVector<ResourceLocation> ResourceLocationManager::findSiblingInstallations() const {
    return findSiblingInstallationsForPlatform(m_osType, m_applicationPath, folderName());
}

QVector<ResourceLocation> ResourceLocationManager::findSiblingInstallationsForPlatform(
    ExtnOSType osType,
    const QString& applicationPath,
    const QString& currentFolderName)
{
    QVector<ResourceLocation> siblings;
    
    if (applicationPath.isEmpty()) {
        return siblings;
    }
    
    // Determine the parent directory to scan and the pattern to match
    QString parentPath;
    QString pattern;
    bool isMacOSApp = false;
    
    switch (osType) {
        case ExtnOSType::Windows: {
            // Windows: application is in e.g., C:\Program Files\OpenSCAD (Nightly)\
            // We want to scan C:\Program Files\ for OpenSCAD* folders
            QDir appDir(applicationPath);
            
            // Go up to find Program Files level
            // App path might be: C:\Program Files\OpenSCAD (Nightly)\bin
            // or just: C:\Program Files\OpenSCAD (Nightly)
            // We need to find the OpenSCAD* folder and then go up one level
            
            QString current = appDir.absolutePath();
            while (!current.isEmpty() && !appDir.isRoot()) {
                QString dirName = appDir.dirName();
                if (dirName.startsWith(QStringLiteral("OpenSCAD"), Qt::CaseInsensitive)) {
                    // Found the OpenSCAD folder, parent is what we want to scan
                    appDir.cdUp();
                    parentPath = appDir.absolutePath();
                    break;
                }
                appDir.cdUp();
                if (appDir.absolutePath() == current) break; // Avoid infinite loop
                current = appDir.absolutePath();
            }
            
            pattern = QStringLiteral("OpenSCAD*");
            break;
        }
        
        case ExtnOSType::MacOS: {
            // macOS: application is in e.g., /Applications/OpenSCAD (Nightly).app/Contents/MacOS/
            // We want to scan /Applications/ for OpenSCAD*.app bundles
            QDir appDir(applicationPath);
            
            // Navigate up to find the .app bundle and then its parent
            QString current = appDir.absolutePath();
            while (!current.isEmpty() && !appDir.isRoot()) {
                QString dirName = appDir.dirName();
                if (dirName.endsWith(QStringLiteral(".app"), Qt::CaseInsensitive) &&
                    dirName.startsWith(QStringLiteral("OpenSCAD"), Qt::CaseInsensitive)) {
                    // Found the .app bundle, parent is what we want to scan
                    appDir.cdUp();
                    parentPath = appDir.absolutePath();
                    isMacOSApp = true;
                    break;
                }
                appDir.cdUp();
                if (appDir.absolutePath() == current) break;
                current = appDir.absolutePath();
            }
            
            pattern = QStringLiteral("OpenSCAD*.app");
            break;
        }
        
        case ExtnOSType::Linux:
        case ExtnOSType::BSD:
        case ExtnOSType::Solaris:
        default:
            // Linux/POSIX: Installations are in system paths (/usr/share, /opt, etc.)
            // Not siblings in the same directory, so return empty
            return siblings;
    }
    
    if (parentPath.isEmpty()) {
        return siblings;
    }
    
    // Scan for sibling installations
    QDir parentDir(parentPath);
    QStringList filters;
    filters << pattern;
    parentDir.setNameFilters(filters);
    parentDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    
    const QStringList entries = parentDir.entryList();
    for (const QString& entry : entries) {
        // Skip current installation
        QString entryName = entry;
        if (isMacOSApp && entryName.endsWith(QStringLiteral(".app"), Qt::CaseInsensitive)) {
            entryName = entryName.left(entryName.length() - 4); // Remove .app suffix
        }
        
        if (entryName.compare(currentFolderName, Qt::CaseInsensitive) == 0) {
            continue; // Skip ourselves
        }
        
        QString siblingPath = parentDir.absoluteFilePath(entry);
        
        // Find the resource directory within this sibling
        // Use the same search logic as for our own installation
        QString resourceDir;
        QStringList searchPaths;
        
        if (isMacOSApp) {
            // macOS .app bundle: check Contents/Resources
            searchPaths << QStringLiteral("Contents/Resources");
        } else {
            // Windows: check . and share/openscad variants
            QString siblingFolderName = entryName; // e.g., "OpenSCAD" or "OpenSCAD (Nightly)"
            // Extract suffix if present
            QString sibSuffix;
            if (siblingFolderName.startsWith(QStringLiteral("OpenSCAD"))) {
                sibSuffix = siblingFolderName.mid(8); // Everything after "OpenSCAD"
            }
            
            searchPaths << QStringLiteral(".");
            searchPaths << (QStringLiteral("share/openscad") + sibSuffix);
        }
        
        QDir sibDir(siblingPath);
        for (const QString& relPath : searchPaths) {
            QString testPath = sibDir.absoluteFilePath(relPath);
            QDir testDir(testPath);
            if (testDir.exists()) {
                // Check if it looks like a valid resource directory
                if (testDir.exists(QStringLiteral("examples")) ||
                    testDir.exists(QStringLiteral("fonts")) ||
                    testDir.exists(QStringLiteral("locale")) ||
                    testDir.exists(QStringLiteral("libraries"))) {
                    resourceDir = testDir.absolutePath();
                    break;
                }
            }
        }
        
        if (!resourceDir.isEmpty()) {
            ResourceLocation loc;
            loc.path = resourceDir;
            loc.displayName = entryName + QStringLiteral(" (Sibling Installation)");
            loc.description = QStringLiteral("Resources from sibling OpenSCAD installation");
            loc.isEnabled = false; // Disabled by default - user must opt-in
            
            // Update exists/writable status
            QFileInfo info(resourceDir);
            loc.exists = info.exists() && info.isDir();
            loc.isWritable = false; // Installation dirs are read-only
            
            siblings.append(loc);
        }
    }
    
    return siblings;
}

QStringList ResourceLocationManager::enabledSiblingPaths() const {
    if (!m_settings) return QStringList();
    return m_settings->value(QLatin1String(KEY_SIBLING_PATHS)).toStringList();
}

void ResourceLocationManager::setEnabledSiblingPaths(const QStringList& paths) {
    if (m_settings) {
        m_settings->setValue(QLatin1String(KEY_SIBLING_PATHS), paths);
        m_settings->sync();
    }
}

// ============================================================================
// Machine Locations (Tier 2)
// ============================================================================

QVector<ResourceLocation> ResourceLocationManager::defaultMachineLocations() const {
    return defaultMachineLocationsForPlatform(m_osType, m_suffix);
}

QVector<ResourceLocation> ResourceLocationManager::defaultMachineLocationsForPlatform(
    ExtnOSType osType, const QString& suffix) 
{
    QVector<ResourceLocation> locations;
    QString folder = QStringLiteral("OpenSCAD") + suffix;
    QString folderLower = QStringLiteral("openscad") + suffix.toLower();
    
    switch (osType) {
        case ExtnOSType::Windows: {
            // Windows: C:/ProgramData/OpenSCAD
            // Uses CSIDL_COMMON_APPDATA
            QString programData;
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
            wchar_t path[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, 0, path))) {
                programData = QString::fromWCharArray(path);
            }
#endif
            if (programData.isEmpty()) {
                programData = QStringLiteral("C:/ProgramData");
            }
            
            locations.append(ResourceLocation(
                QDir(programData).absoluteFilePath(folder),
                QStringLiteral("System Resources"),
                QStringLiteral("Shared resources for all users on this machine")
            ));
            break;
        }
        
        case ExtnOSType::MacOS:
            // macOS: /Library/Application Support/OpenSCAD
            locations.append(ResourceLocation(
                QStringLiteral("/Library/Application Support/") + folder,
                QStringLiteral("System Resources"),
                QStringLiteral("Shared resources for all users on this Mac")
            ));
            break;
            
        case ExtnOSType::Linux:
        case ExtnOSType::BSD:
        case ExtnOSType::Solaris:
        default:
            // Linux/POSIX: /usr/local/share/openscad, /opt/openscad
            // Note: lowercase for Unix convention
            locations.append(ResourceLocation(
                QStringLiteral("/usr/local/share/") + folderLower,
                QStringLiteral("Local System Resources"),
                QStringLiteral("Site-local resources in /usr/local")
            ));
            locations.append(ResourceLocation(
                QStringLiteral("/opt/") + folderLower,
                QStringLiteral("Optional Package Resources"),
                QStringLiteral("Resources in /opt directory")
            ));
            // XDG system data dirs
            QString xdgDataDirs = qEnvironmentVariable("XDG_DATA_DIRS", 
                QStringLiteral("/usr/local/share:/usr/share"));
            const QStringList dataDirs = xdgDataDirs.split(QLatin1Char(':'));
            for (const QString& dir : dataDirs) {
                QString path = QDir(dir).absoluteFilePath(folderLower);
                // Avoid duplicates
                bool found = false;
                for (const auto& loc : locations) {
                    if (loc.path == path) {
                        found = true;
                        break;
                    }
                }
                if (!found && !dir.isEmpty()) {
                    locations.append(ResourceLocation(
                        path,
                        QStringLiteral("XDG Data: ") + dir,
                        QStringLiteral("System data directory")
                    ));
                }
            }
            break;
    }
    
    // Update status for all locations
    updateLocationStatuses(locations);
    return locations;
}

QString ResourceLocationManager::machineConfigFilePath() const {
    QString basePath;
    QString folder = folderName();
    
    switch (m_osType) {
        case ExtnOSType::Windows: {
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
            wchar_t path[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, 0, path))) {
                basePath = QString::fromWCharArray(path);
            }
#endif
            if (basePath.isEmpty()) {
                basePath = QStringLiteral("C:/ProgramData");
            }
            break;
        }
        
        case ExtnOSType::MacOS:
            basePath = QStringLiteral("/Library/Application Support");
            break;
            
        case ExtnOSType::Linux:
        case ExtnOSType::BSD:
        case ExtnOSType::Solaris:
        default:
            // Check /usr/local/etc first, fall back to /etc
            if (QDir(QStringLiteral("/usr/local/etc")).exists()) {
                basePath = QStringLiteral("/usr/local/etc");
            } else {
                basePath = QStringLiteral("/etc");
            }
            folder = folder.toLower(); // Unix convention
            break;
    }
    
    return QDir(basePath).absoluteFilePath(folder + QLatin1Char('/') + QLatin1String(CONFIG_FILENAME));
}

QVector<ResourceLocation> ResourceLocationManager::availableMachineLocations() const {
    if (!m_machineLocationsLoaded) {
        const_cast<ResourceLocationManager*>(this)->loadMachineLocationsConfig();
    }
    return m_machineLocations;
}

QVector<ResourceLocation> ResourceLocationManager::enabledMachineLocations() const {
    QVector<ResourceLocation> enabled;
    if (!m_settings) return enabled;
    
    QStringList enabledPaths = m_settings->value(QLatin1String(KEY_MACHINE_PATHS)).toStringList();
    
    // If no settings yet, enable all available by default
    if (enabledPaths.isEmpty()) {
        return availableMachineLocations();
    }
    
    const QVector<ResourceLocation>& available = availableMachineLocations();
    for (const ResourceLocation& loc : available) {
        if (enabledPaths.contains(loc.path)) {
            ResourceLocation copy = loc;
            copy.isEnabled = true;
            enabled.append(copy);
        }
    }
    
    return enabled;
}

void ResourceLocationManager::setEnabledMachineLocations(const QStringList& paths) {
    if (m_settings) {
        m_settings->setValue(QLatin1String(KEY_MACHINE_PATHS), paths);
        m_settings->sync();
    }
}

bool ResourceLocationManager::loadMachineLocationsConfig() {
    QString configPath = machineConfigFilePath();
    
    if (QFile::exists(configPath)) {
        m_machineLocations = loadLocationsFromJson(configPath);
    } else {
        // Use defaults if no config file
        m_machineLocations = defaultMachineLocations();
    }
    
    updateLocationStatuses(m_machineLocations);
    m_machineLocationsLoaded = true;
    return !m_machineLocations.isEmpty();
}

bool ResourceLocationManager::saveMachineLocationsConfig(const QVector<ResourceLocation>& locations) {
    QString configPath = machineConfigFilePath();
    
    // Ensure directory exists
    QDir dir = QFileInfo(configPath).absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(QStringLiteral("."))) {
            return false;
        }
    }
    
    if (saveLocationsToJson(configPath, locations)) {
        m_machineLocations = locations;
        updateLocationStatuses(m_machineLocations);
        return true;
    }
    return false;
}

// ============================================================================
// User Locations (Tier 3)
// ============================================================================

QVector<ResourceLocation> ResourceLocationManager::defaultUserLocations() const {
    return defaultUserLocationsForPlatform(m_osType, m_suffix);
}

QVector<ResourceLocation> ResourceLocationManager::defaultUserLocationsForPlatform(
    ExtnOSType osType, const QString& suffix) 
{
    QVector<ResourceLocation> locations;
    QString folder = QStringLiteral("OpenSCAD") + suffix;
    QString folderLower = QStringLiteral("openscad") + suffix.toLower();
    
    switch (osType) {
        case ExtnOSType::Windows: {
            // Windows user paths
            
            // 1. GenericDataLocation - %LOCALAPPDATA% (e.g., C:/Users/X/AppData/Local/OpenSCAD)
            // This is where OpenSCAD actually stores user resources
            QString genericData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
            if (!genericData.isEmpty()) {
                locations.append(ResourceLocation(
                    QDir(genericData).absoluteFilePath(folder),
                    QStringLiteral("Local Application Data"),
                    QStringLiteral("Primary user resource location (%LOCALAPPDATA%)")
                ));
            }
            
            // 2. AppData/Roaming (AppDataLocation) - %APPDATA%/OpenSCAD
            QString roaming = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            // AppDataLocation includes app name, go up one level and add our folder
            QDir roamingDir(roaming);
            roamingDir.cdUp();
            locations.append(ResourceLocation(
                roamingDir.absoluteFilePath(folder),
                QStringLiteral("Roaming Application Data"),
                QStringLiteral("Roaming profile - syncs across machines in a domain")
            ));
            
            // 3. Documents/OpenSCAD
            QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            locations.append(ResourceLocation(
                QDir(docs).absoluteFilePath(folder),
                QStringLiteral("My Documents"),
                QStringLiteral("User documents folder - easy to find and backup")
            ));
            break;
        }
        
        case ExtnOSType::MacOS:
            // macOS user paths
            // 1. ~/Library/Application Support/OpenSCAD
            locations.append(ResourceLocation(
                QDir::homePath() + QStringLiteral("/Library/Application Support/") + folder,
                QStringLiteral("Application Support"),
                QStringLiteral("Standard macOS application data location")
            ));
            
            // 2. ~/Documents/OpenSCAD
            locations.append(ResourceLocation(
                QDir::homePath() + QStringLiteral("/Documents/") + folder,
                QStringLiteral("Documents"),
                QStringLiteral("User documents folder - visible in Finder")
            ));
            break;
            
        case ExtnOSType::Linux:
        case ExtnOSType::BSD:
        case ExtnOSType::Solaris:
        default: {
            // Linux/POSIX user paths (XDG compliant)
            // 1. XDG_CONFIG_HOME (~/.config/openscad)
            QString configHome = qEnvironmentVariable("XDG_CONFIG_HOME");
            if (configHome.isEmpty()) {
                configHome = QDir::homePath() + QStringLiteral("/.config");
            }
            locations.append(ResourceLocation(
                QDir(configHome).absoluteFilePath(folderLower),
                QStringLiteral("User Config"),
                QStringLiteral("XDG config directory - for settings and small data")
            ));
            
            // 2. XDG_DATA_HOME (~/.local/share/openscad)
            QString dataHome = qEnvironmentVariable("XDG_DATA_HOME");
            if (dataHome.isEmpty()) {
                dataHome = QDir::homePath() + QStringLiteral("/.local/share");
            }
            locations.append(ResourceLocation(
                QDir(dataHome).absoluteFilePath(folderLower),
                QStringLiteral("User Data"),
                QStringLiteral("XDG data directory - for user resources")
            ));
            
            // 3. ~/Documents/OpenSCAD (if Documents exists)
            QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            if (!docs.isEmpty()) {
                locations.append(ResourceLocation(
                    QDir(docs).absoluteFilePath(folder),
                    QStringLiteral("Documents"),
                    QStringLiteral("User documents folder - easy to find")
                ));
            }
            break;
        }
    }
    
    // Always show OPENSCADPATH entry - disabled if env var not set
    // This lets users know it's an option even if not currently configured
    QString openscadPath = qEnvironmentVariable("OPENSCADPATH");
    if (!openscadPath.isEmpty()) {
        // Environment variable is set - add actual paths
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
        QChar separator = QLatin1Char(';');
#else
        QChar separator = QLatin1Char(':');
#endif
        const QStringList envPaths = openscadPath.split(separator, Qt::SkipEmptyParts);
        for (const QString& envPath : envPaths) {
            QString cleanPath = QDir::cleanPath(envPath.trimmed());
            if (!cleanPath.isEmpty()) {
                // Check for duplicates
                bool isDuplicate = false;
                for (const auto& loc : locations) {
                    if (loc.path == cleanPath) {
                        isDuplicate = true;
                        break;
                    }
                }
                if (!isDuplicate) {
                    locations.append(ResourceLocation(
                        cleanPath,
                        QStringLiteral("OPENSCADPATH"),
                        QStringLiteral("From OPENSCADPATH environment variable")
                    ));
                }
            }
        }
    } else {
        // Environment variable not set - show as informational placeholder
        ResourceLocation envPlaceholder(
            QStringLiteral("(OPENSCADPATH not set)"),
            QStringLiteral("OPENSCADPATH"),
            QStringLiteral("Set OPENSCADPATH environment variable to add custom resource paths")
        );
        envPlaceholder.exists = false;
        envPlaceholder.isEnabled = false;
        locations.append(envPlaceholder);
    }
    
    updateLocationStatuses(locations);
    return locations;
}

QString ResourceLocationManager::userConfigFilePath() const {
    QString basePath;
    QString folder = folderName();
    
    switch (m_osType) {
        case ExtnOSType::Windows: {
            // Use AppData/Roaming for user config
            QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir dir(appData);
            dir.cdUp();
            basePath = dir.absoluteFilePath(folder);
            break;
        }
        
        case ExtnOSType::MacOS:
            basePath = QDir::homePath() + QStringLiteral("/Library/Application Support/") + folder;
            break;
            
        case ExtnOSType::Linux:
        case ExtnOSType::BSD:
        case ExtnOSType::Solaris:
        default: {
            QString configHome = qEnvironmentVariable("XDG_CONFIG_HOME");
            if (configHome.isEmpty()) {
                configHome = QDir::homePath() + QStringLiteral("/.config");
            }
            basePath = QDir(configHome).absoluteFilePath(folder.toLower());
            break;
        }
    }
    
    return QDir(basePath).absoluteFilePath(QLatin1String(CONFIG_FILENAME));
}

QVector<ResourceLocation> ResourceLocationManager::availableUserLocations() const {
    if (!m_userLocationsLoaded) {
        const_cast<ResourceLocationManager*>(this)->loadUserLocationsConfig();
    }
    return m_userLocations;
}

QVector<ResourceLocation> ResourceLocationManager::enabledUserLocations() const {
    QVector<ResourceLocation> enabled;
    if (!m_settings) return enabled;
    
    QStringList enabledPaths = m_settings->value(QLatin1String(KEY_USER_PATHS)).toStringList();
    
    // If no settings yet, enable all available by default
    if (enabledPaths.isEmpty()) {
        return availableUserLocations();
    }
    
    const QVector<ResourceLocation>& available = availableUserLocations();
    for (const ResourceLocation& loc : available) {
        if (enabledPaths.contains(loc.path)) {
            ResourceLocation copy = loc;
            copy.isEnabled = true;
            enabled.append(copy);
        }
    }
    
    return enabled;
}

void ResourceLocationManager::setEnabledUserLocations(const QStringList& paths) {
    if (m_settings) {
        m_settings->setValue(QLatin1String(KEY_USER_PATHS), paths);
        m_settings->sync();
    }
}

bool ResourceLocationManager::loadUserLocationsConfig() {
    QString configPath = userConfigFilePath();
    
    if (QFile::exists(configPath)) {
        m_userLocations = loadLocationsFromJson(configPath);
    } else {
        // Use defaults if no config file
        m_userLocations = defaultUserLocations();
    }
    
    updateLocationStatuses(m_userLocations);
    m_userLocationsLoaded = true;
    return !m_userLocations.isEmpty();
}

bool ResourceLocationManager::saveUserLocationsConfig(const QVector<ResourceLocation>& locations) {
    QString configPath = userConfigFilePath();
    
    // Ensure directory exists
    QDir dir = QFileInfo(configPath).absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(QStringLiteral("."))) {
            return false;
        }
    }
    
    if (saveLocationsToJson(configPath, locations)) {
        m_userLocations = locations;
        updateLocationStatuses(m_userLocations);
        return true;
    }
    return false;
}

bool ResourceLocationManager::addUserLocation(const ResourceLocation& location, bool enable) {
    loadUserLocationsConfig();
    
    // Check for duplicates
    for (const auto& loc : m_userLocations) {
        if (loc.path == location.path) {
            return false; // Already exists
        }
    }
    
    ResourceLocation newLoc = location;
    newLoc.isEnabled = enable;
    updateLocationStatus(newLoc);
    m_userLocations.append(newLoc);
    
    if (!saveUserLocationsConfig(m_userLocations)) {
        m_userLocations.removeLast();
        return false;
    }
    
    if (enable) {
        QStringList enabled = m_settings->value(QLatin1String(KEY_USER_PATHS)).toStringList();
        enabled.append(location.path);
        setEnabledUserLocations(enabled);
    }
    
    return true;
}

bool ResourceLocationManager::removeUserLocation(const QString& path) {
    loadUserLocationsConfig();
    
    int index = -1;
    for (int i = 0; i < m_userLocations.size(); ++i) {
        if (m_userLocations[i].path == path) {
            index = i;
            break;
        }
    }
    
    if (index < 0) {
        return false;
    }
    
    ResourceLocation removed = m_userLocations.takeAt(index);
    
    if (!saveUserLocationsConfig(m_userLocations)) {
        m_userLocations.insert(index, removed);
        return false;
    }
    
    // Also remove from enabled list
    QStringList enabled = m_settings->value(QLatin1String(KEY_USER_PATHS)).toStringList();
    enabled.removeAll(path);
    setEnabledUserLocations(enabled);
    
    return true;
}

// ============================================================================
// Combined Resource Resolution
// ============================================================================

QVector<ResourceLocation> ResourceLocationManager::allEnabledLocations() const {
    QVector<ResourceLocation> all;
    
    // 1. Installation location (if found)
    QString installDir = findInstallationResourceDir();
    if (!installDir.isEmpty()) {
        ResourceLocation install(installDir, 
            QStringLiteral("Application Resources"), 
            QStringLiteral("Built-in resources from installation"));
        install.isEnabled = true;
        install.exists = true;
        install.isWritable = false;
        all.append(install);
    }
    
    // 1b. Sibling installations (if enabled by user)
    QStringList enabledSiblings = enabledSiblingPaths();
    if (!enabledSiblings.isEmpty()) {
        QVector<ResourceLocation> siblings = findSiblingInstallations();
        for (const ResourceLocation& sib : siblings) {
            if (sib.exists && enabledSiblings.contains(sib.path)) {
                ResourceLocation enabled = sib;
                enabled.isEnabled = true;
                all.append(enabled);
            }
        }
    }
    
    // 2. Machine locations (only existing ones)
    const QVector<ResourceLocation> machine = enabledMachineLocations();
    for (const ResourceLocation& loc : machine) {
        if (loc.exists) {
            all.append(loc);
        }
    }
    
    // 3. User locations (only existing ones)
    const QVector<ResourceLocation> user = enabledUserLocations();
    for (const ResourceLocation& loc : user) {
        if (loc.exists) {
            all.append(loc);
        }
    }
    
    return all;
}

QStringList ResourceLocationManager::resourcePathsForType(ResourceType type) const {
    QStringList paths;
    QString subdir = ResourcePaths::resourceSubdirectory(type);
    
    if (subdir.isEmpty()) {
        return paths;
    }
    
    const QVector<ResourceLocation> locations = allEnabledLocations();
    for (const ResourceLocation& loc : locations) {
        QString typePath = QDir(loc.path).absoluteFilePath(subdir);
        if (QDir(typePath).exists()) {
            paths.append(typePath);
        }
    }
    
    return paths;
}

QString ResourceLocationManager::findResourcePath(ResourceType type) const {
    QStringList paths = resourcePathsForType(type);
    return paths.isEmpty() ? QString() : paths.first();
}

QString ResourceLocationManager::writableUserPath(ResourceType type) const {
    QString subdir = ResourcePaths::resourceSubdirectory(type);
    if (subdir.isEmpty()) {
        return QString();
    }
    
    // Find first writable user location
    const QVector<ResourceLocation> user = enabledUserLocations();
    for (const ResourceLocation& loc : user) {
        if (loc.isWritable || !loc.exists) {
            // If it doesn't exist, we might be able to create it
            QDir baseDir(loc.path);
            QString typePath = baseDir.absoluteFilePath(subdir);
            
            // Try to create if needed
            if (!QDir(typePath).exists()) {
                if (!QDir().mkpath(typePath)) {
                    continue; // Can't create, try next
                }
            }
            
            // Verify writable
            QFileInfo info(typePath);
            if (info.isWritable()) {
                return typePath;
            }
        }
    }
    
    return QString();
}

// ============================================================================
// Refresh and Validation
// ============================================================================

void ResourceLocationManager::refreshLocationStatus() {
    updateLocationStatuses(m_machineLocations);
    updateLocationStatuses(m_userLocations);
    m_installDirCached = false;
}

void ResourceLocationManager::reloadConfiguration() {
    m_machineLocationsLoaded = false;
    m_userLocationsLoaded = false;
    m_installDirCached = false;
    m_machineLocations.clear();
    m_userLocations.clear();
    m_cachedInstallDir.clear();
    
    loadMachineLocationsConfig();
    loadUserLocationsConfig();
}

// ============================================================================
// Config File I/O (JSON)
// ============================================================================

QVector<ResourceLocation> ResourceLocationManager::loadLocationsFromJson(const QString& filePath) {
    QVector<ResourceLocation> locations;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return locations;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();
    
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return locations;
    }
    
    QJsonObject root = doc.object();
    QJsonArray locArray = root[QStringLiteral("locations")].toArray();
    
    for (const QJsonValue& val : locArray) {
        if (!val.isObject()) continue;
        
        QJsonObject obj = val.toObject();
        QString path = obj[QStringLiteral("path")].toString();
        if (path.isEmpty()) continue;
        
        ResourceLocation loc;
        loc.path = path;
        loc.displayName = obj[QStringLiteral("displayName")].toString(path);
        loc.description = obj[QStringLiteral("description")].toString();
        loc.isEnabled = obj[QStringLiteral("enabled")].toBool(true);
        
        locations.append(loc);
    }
    
    return locations;
}

bool ResourceLocationManager::saveLocationsToJson(const QString& filePath, 
                                                    const QVector<ResourceLocation>& locations) 
{
    QJsonArray locArray;
    for (const ResourceLocation& loc : locations) {
        QJsonObject obj;
        obj[QStringLiteral("path")] = loc.path;
        obj[QStringLiteral("displayName")] = loc.displayName;
        if (!loc.description.isEmpty()) {
            obj[QStringLiteral("description")] = loc.description;
        }
        // Note: enabled state is stored in QSettings, not the config file
        locArray.append(obj);
    }
    
    QJsonObject root;
    root[QStringLiteral("locations")] = locArray;
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

// ============================================================================
// Location Status Updates
// ============================================================================

void ResourceLocationManager::updateLocationStatus(ResourceLocation& loc) {
    QFileInfo info(loc.path);
    loc.exists = info.exists() && info.isDir();
    loc.isWritable = loc.exists && info.isWritable();
    
    // Check for resource folders (examples, fonts, libraries, locale)
    loc.hasResourceFolders = false;
    if (loc.exists) {
        QDir dir(loc.path);
        static const QStringList resourceFolders = {
            QStringLiteral("examples"),
            QStringLiteral("fonts"),
            QStringLiteral("libraries"),
            QStringLiteral("locale"),
            QStringLiteral("templates"),
            QStringLiteral("color-schemes")
        };
        for (const QString& folder : resourceFolders) {
            if (dir.exists(folder)) {
                loc.hasResourceFolders = true;
                break;
            }
        }
    }
}

void ResourceLocationManager::updateLocationStatuses(QVector<ResourceLocation>& locations) {
    for (ResourceLocation& loc : locations) {
        updateLocationStatus(loc);
    }
}

} // namespace platformInfo
