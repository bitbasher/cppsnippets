/**
 * @file resourcePaths.cpp
 * @brief Implementation of ResourcePaths class
 */

#include "platformInfo/resourcePaths.h"
#include "platformInfo/platformInfo.h"
#include "resourceInfo/resourceTypes.h"
#include "resourceInfo/resourceTier.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSettings>

#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
#include <shlobj.h>
#include <windows.h>
#endif

namespace platformInfo {

// Constants moved to header as inline static members

// Static resource type definitions with file extensions
// s_resourceTypes moved to header as inline static member

ResourcePaths::ResourcePaths()
    : m_suffix()  // Empty suffix = release build, otherwise "(Nightly)"
    , m_osType(ExtnOSType::Unknown)
{
    detectOSType();
}

ResourcePaths::ResourcePaths(const QString& applicationPath, const QString& suffix)
    : m_applicationPath(applicationPath)
    , m_suffix(suffix)
    , m_osType(ExtnOSType::Unknown)
{
    detectOSType();
}

void ResourcePaths::setApplicationPath(const QString& path) {
    m_applicationPath = path;
}

void ResourcePaths::setSuffix(const QString& suffix) {
    m_suffix = suffix;
}

QString ResourcePaths::folderName() const {
    // User/machine tiers are unsuffixed per updated spec
    return QStringLiteral("ScadTemplates");
}

void ResourcePaths::detectOSType() {
    PlatformInfo info;
    m_osType = info.currentOSType();
}

QList<ResourceTypeInfo> ResourcePaths::allResourceTypes() {
    return resourceInfo::ResourceTypeRegistry::allTypes();
}

// ============================================================================
// Default Search Paths (Immutable, Compile-Time Constants)
// ============================================================================
//
// These static const lists define the default locations to search for resources.
// They are platform-specific and determined at compile time using #ifdef.
// Users cannot modify these - they are the "Restore Defaults" source.
//
// Note: The suffix (e.g., " (Nightly)") is handled separately when resolving paths.
// These are the base paths without suffix applied.

// Constants moved to header as inline static member (unified map)
const QStringList& ResourcePaths::defaultSearchPaths(resourceInfo::ResourceTier tier) {
    static const QStringList empty;
    auto it = s_defaultSearchPaths.find(tier);
    return (it != s_defaultSearchPaths.end()) ? it.value() : empty;
}

QStringList ResourcePaths::expandedSearchPaths(resourceInfo::ResourceTier tier) const {
    const QStringList& defaults = defaultSearchPaths(tier);
    QStringList expanded;
    expanded.reserve(defaults.size());
    
    for (const QString& path : defaults) {
        expanded.append(expandEnvVars(path));
    }
    
    return expanded;
}

QList<ResourcePathElement> ResourcePaths::defaultElements() const {
    QList<ResourcePathElement> result;

    // Installation tier (relative to application path, with suffix rules)
    if (!m_applicationPath.isEmpty()) {
        QDir appDir(m_applicationPath);
        for (const QString& raw : defaultSearchPaths(resourceInfo::ResourceTier::Installation)) {
            QString base = expandEnvVars(raw);
            QString rel = base.endsWith(QStringLiteral("/"))
                ? QDir::cleanPath(base + QStringLiteral("openscad") + m_suffix)
                : QDir::cleanPath(base);
            QString abs = appDir.absoluteFilePath(rel);
            QFileInfo fi(abs);
            QString finalPath = fi.canonicalFilePath();
            if (finalPath.isEmpty()) finalPath = fi.absoluteFilePath();
            ResourceLocation loc(finalPath, QStringLiteral("Installation"), QString(), resourceInfo::ResourceTier::Installation);
            result.append(ResourcePathElement(loc, resourceInfo::ResourceTier::Installation));
        }
        
        // Add sibling installations discovered on this platform
        const QVector<ResourceLocation> siblings = findSiblingInstallations();
        for (const ResourceLocation& sib : siblings) {
            result.append(ResourcePathElement(sib, resourceInfo::ResourceTier::Installation));
        }
    }

    // Machine tier
    for (const QString& raw : defaultSearchPaths(resourceInfo::ResourceTier::Machine)) {
        QString expanded = QDir::cleanPath(expandEnvVars(raw));
        QFileInfo fi(expanded);
        QString finalPath = fi.canonicalFilePath();
        if (finalPath.isEmpty()) finalPath = fi.absoluteFilePath();
        ResourceLocation loc(finalPath, QStringLiteral("Machine"), QString(), resourceInfo::ResourceTier::Machine);
        result.append(ResourcePathElement(loc, resourceInfo::ResourceTier::Machine));
    }

    // User tier
    for (const QString& raw : defaultSearchPaths(resourceInfo::ResourceTier::User)) {
        QString expanded = QDir::cleanPath(expandEnvVars(raw));
        QFileInfo fi(expanded);
        QString finalPath = fi.canonicalFilePath();
        if (finalPath.isEmpty()) finalPath = fi.absoluteFilePath();
        ResourceLocation loc(finalPath, QStringLiteral("User"), QString(), resourceInfo::ResourceTier::User);
        result.append(ResourcePathElement(loc, resourceInfo::ResourceTier::User));
    }

    return result;
}

QStringList ResourcePaths::appSearchPaths() const {
    // Build concrete search paths from defaults with env var expansion and suffix rules:
    // - Expand ${VAR} and %VAR% using ResourcePaths env registry + system env
    // - Paths ending with "/" have app name + suffix appended
    //   Example: "%PROGRAMFILES%/" -> "C:/Program Files/openscad (Nightly)"
    // - Paths without trailing "/" are used as-is (after expansion)
    const QStringList& installPaths = defaultSearchPaths(resourceInfo::ResourceTier::Installation);
    QStringList paths;
    paths.reserve(installPaths.size());
    for (const QString& raw : installPaths) {
        QString base = expandEnvVars(raw);
        if (base.endsWith(QStringLiteral("/"))) {
            // Append app folder name + suffix for share-style bases
            QString withSuffix = QDir::cleanPath(base + QStringLiteral("openscad") + m_suffix);
            paths << withSuffix;
        } else {
            // Use expanded path directly
            paths << QDir::cleanPath(base);
        }
    }
    return paths;
}

QString ResourcePaths::findAppResourceDirectory() const {
    if (m_applicationPath.isEmpty()) {
        return QString();
    }

    QDir appDir(m_applicationPath);
    const QStringList paths = appSearchPaths();

    for (const QString& relativePath : paths) {
        QString candidatePath = appDir.absoluteFilePath(relativePath);
        QDir candidateDir(candidatePath);

        // Check if this looks like a valid resource directory
        // by checking for at least one known subdirectory
        if (candidateDir.exists()) {
            //FIXME need a better way to verify
            // Check for examples, fonts, or color-schemes subdirectory as validation
            if (candidateDir.exists(QStringLiteral("examples")) ||
                candidateDir.exists(QStringLiteral("fonts")) ||
                candidateDir.exists(QStringLiteral("color-schemes"))) {
                return candidateDir.absolutePath();
            }
        }
    }

    return QString();
}

QString ResourcePaths::appResourcePath(ResourceType type) const {
    QString resourceDir = findAppResourceDirectory();
    if (resourceDir.isEmpty()) {
        return QString();
    }
    
    QString subdir = resourceSubdirectory(type);
    if (subdir.isEmpty()) {
        return QString();
    }
    
    QDir dir(resourceDir);
    QString path = dir.absoluteFilePath(subdir);
    
    // Return the path only if it exists
    if (QDir(path).exists()) {
        return path;
    }
    
    return QString();
}

bool ResourcePaths::hasAppResourceDirectory(ResourceType type) const {
    return !appResourcePath(type).isEmpty();
}

// ========== User Resource Paths ==========

QString ResourcePaths::resolveUserConfigPath() const {
    return userConfigPathForPlatform(m_osType);
}

QString ResourcePaths::userConfigPathForPlatform(ExtnOSType osType) {
    QString path;
    
    // These paths match OpenSCAD's PlatformUtils::userConfigPath() BASE directory
    // OpenSCAD's userConfigPath() returns this + "/" + OPENSCAD_FOLDER_NAME
    //
    // userConfigPath is for WRITABLE configuration:
    //   Linux: $XDG_CONFIG_HOME (default ~/.config)
    //   Windows: CSIDL_LOCAL_APPDATA
    //   macOS: NSApplicationSupportDirectory (~Library/Application Support)
    
    switch (osType) {
        case ExtnOSType::Windows: {
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
            // Windows: CSIDL_LOCAL_APPDATA for writable config
            // (This is different from documentsPath which uses CSIDL_PERSONAL)
            wchar_t appDataPath[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath))) {
                path = QString::fromWCharArray(appDataPath);
            }
#else
            // Cross-compile fallback using Qt
            path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
            // Remove app name that Qt adds
            QDir dir(path);
            dir.cdUp();
            path = dir.absolutePath();
#endif
            break;
        }
        
        case ExtnOSType::MacOS: {
            // macOS: NSApplicationSupportDirectory -> ~/Library/Application Support
            // OpenSCAD uses:
            //   [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory ...]
            //   then appends OPENSCAD_FOLDER_NAME
            path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
            break;
        }
        
        case ExtnOSType::Linux:
        case ExtnOSType::BSD:
        case ExtnOSType::Solaris:
        default: {
            // Linux/POSIX: $XDG_CONFIG_HOME or $HOME/.config
            // OpenSCAD uses:
            //   getenv("XDG_CONFIG_HOME") if set
            //   else getenv("HOME") + "/.config"
            QString xdgConfig = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_HOME"));
            if (!xdgConfig.isEmpty() && QDir(xdgConfig).exists()) {
                path = xdgConfig;
            } else {
                // Fallback to $HOME/.config
                QString home = QString::fromLocal8Bit(qgetenv("HOME"));
                if (!home.isEmpty()) {
                    path = home + QStringLiteral("/.config");
                }
            }
            break;
        }
    }
    
    return path;
}

QString ResourcePaths::userDocumentsPathForPlatform(ExtnOSType osType) {
    QString path;
    
    // These paths match OpenSCAD's PlatformUtils::documentsPath()
    // Used for user libraries and backup files
    //
    // documentsPath:
    //   Linux: $HOME/.local/share
    //   Windows: CSIDL_PERSONAL (My Documents)
    //   macOS: NSDocumentDirectory (~Documents)
    
    switch (osType) {
        case ExtnOSType::Windows: {
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
            // Windows: CSIDL_PERSONAL (My Documents)
            wchar_t docPath[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, docPath))) {
                path = QString::fromWCharArray(docPath);
            }
#else
            // Cross-compile fallback using Qt
            path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif
            break;
        }
        
        case ExtnOSType::MacOS: {
            // macOS: NSDocumentDirectory -> ~/Documents
            path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            break;
        }
        
        case ExtnOSType::Linux:
        case ExtnOSType::BSD:
        case ExtnOSType::Solaris:
        default: {
            // Linux: $HOME/.local/share
            QString home = QString::fromLocal8Bit(qgetenv("HOME"));
            if (!home.isEmpty()) {
                path = home + QStringLiteral("/.local/share");
            }
            break;
        }
    }
    
    return path;
}

QString ResourcePaths::getUserConfigPath() const {
    return resolveUserConfigPath();
}

QString ResourcePaths::userResourcePath(ResourceType type) const {
    QString configPath = getUserConfigPath();
    if (configPath.isEmpty()) {
        return QString();
    }
    
    QString subdir = resourceSubdirectory(type);
    if (subdir.isEmpty()) {
        return QString();
    }
    
    QDir dir(configPath);
    return dir.absoluteFilePath(folderName() + QLatin1Char('/') + subdir);
}

bool ResourcePaths::hasUserResourceDirectory(ResourceType type) const {
    QString path = userResourcePath(type);
    return !path.isEmpty() && QDir(path).exists();
}

// ========== Combined Resource Paths ==========

QStringList ResourcePaths::allResourcePaths(ResourceType type) const {
    QStringList paths;
    
    // App resources first (primary)
    QString appPath = appResourcePath(type);
    if (!appPath.isEmpty()) {
        paths << appPath;
    }
    
    // User resources second (can override/extend)
    QString userPath = userResourcePath(type);
    if (!userPath.isEmpty() && QDir(userPath).exists()) {
        paths << userPath;
    }
    
    return paths;
}

// ========== Environment Variable Helpers ==========

void ResourcePaths::addEnvVar(const QString& name, const QString& value) {
    if (name.isEmpty()) {
        return;
    }

    for (auto& entry : m_envVars) {
        if (entry.name == name) {
            entry.value = value;
            return;
        }
    }

    m_envVars.append({name, value});
}

void ResourcePaths::removeEnvVar(const QString& name) {
    if (name.isEmpty()) {
        return;
    }

    for (int i = m_envVars.size() - 1; i >= 0; --i) {
        if (m_envVars.at(i).name == name) {
            m_envVars.removeAt(i);
        }
    }
}

QString ResourcePaths::envVarValue(const QString& name) const {
    if (name.isEmpty()) {
        return QString();
    }

    for (const auto& entry : m_envVars) {
        if (entry.name == name) {
            return entry.value;
        }
    }

    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    return env.value(name, QString());
}

QString ResourcePaths::expandEnvVars(const QString& path) const {
    if (path.isEmpty()) {
        return path;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    for (const auto& entry : m_envVars) {
        if (!entry.name.isEmpty()) {
            env.insert(entry.name, entry.value);
        }
    }

    const QRegularExpression pattern(
        QStringLiteral(R"(\$\{([^}]+)\}|%([^%]+)%)"));

    QString expanded;
    expanded.reserve(path.size());
    int lastIndex = 0;
    auto matchIt = pattern.globalMatch(path);
    while (matchIt.hasNext()) {
        const auto match = matchIt.next();
        const int start = match.capturedStart();
        const int end = match.capturedEnd();
        expanded.append(path.mid(lastIndex, start - lastIndex));

        const QString key = !match.captured(1).isEmpty() ? match.captured(1)
                                                         : match.captured(2);
        expanded.append(env.value(key, QString()));
        lastIndex = end;
    }

    expanded.append(path.mid(lastIndex));
    return expanded;
}

QList<ResourcePathElement> ResourcePaths::elementsByTier(resourceInfo::ResourceTier tier) const {
    QList<ResourcePathElement> result;
    result.reserve(m_resourcePathElements.size());
    for (const auto& elem : m_resourcePathElements) {
        if (elem.tier == tier) {
            result.append(elem);
        }
    }
    result.squeeze();
    return result;
}

// ========== Settings Persistence ==========

void ResourcePaths::saveEnvVars(QSettings& settings) const {
    settings.beginGroup(QStringLiteral("EnvVars"));
    settings.remove(QString());  // Clear existing env vars
    
    for (const EnvVarEntry& entry : m_envVars) {
        settings.setValue(entry.name, entry.value);
    }
    
    settings.endGroup();
}

void ResourcePaths::loadEnvVars(QSettings& settings) {
    settings.beginGroup(QStringLiteral("EnvVars"));
    
    const QStringList keys = settings.childKeys();
    for (const QString& key : keys) {
        const QString value = settings.value(key).toString();
        addEnvVar(key, value);
    }
    
    settings.endGroup();
}

// ========== Validation ==========

bool ResourcePaths::isValidResourceDirectory(const QString& path) const {
    QDir dir(path);
    if (!dir.exists()) {
        return false;
    }
    
    // Validate by checking for known OpenSCAD subdirectories
    return dir.exists(QStringLiteral("examples")) ||
           dir.exists(QStringLiteral("fonts")) ||
           dir.exists(QStringLiteral("locale")) ||
           dir.exists(QStringLiteral("libraries"));
}

// ========== Sibling Installation Discovery ==========

QVector<ResourceLocation> ResourcePaths::findSiblingInstallations() const {
    QDir appDir(m_applicationPath);
    QString currentFolderName = appDir.dirName();
    if (currentFolderName.compare(QStringLiteral("bin"), Qt::CaseInsensitive) == 0) {
        appDir.cdUp();
        currentFolderName = appDir.dirName();
    }

    return findSiblingInstallationsForPlatform(m_osType, m_applicationPath, currentFolderName);
}

QVector<ResourceLocation> ResourcePaths::findSiblingInstallationsForPlatform(
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
            
            // Fallback: if not running from a standard installation, scan common Windows paths
            if (parentPath.isEmpty()) {
                QString progFiles = QStringLiteral("C:/Program Files");
                if (QDir(progFiles).exists()) {
                    parentPath = progFiles;
                } else {
                    // Try alternate path
                    progFiles = QStringLiteral("C:/Program Files (x86)");
                    if (QDir(progFiles).exists()) {
                        parentPath = progFiles;
                    }
                }
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
            // Linux/POSIX: Installations are in system paths, not siblings
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
            loc.tier = resourceInfo::ResourceTier::Installation;
            loc.isEnabled = true; // Auto-enable discovered siblings
            
            // Update exists/writable status
            QFileInfo info(resourceDir);
            loc.exists = info.exists() && info.isDir();
            loc.isWritable = false; // Installation dirs are read-only
            
            siblings.append(loc);
        }
    }
    
    return siblings;
}

bool ResourcePaths::isValid() const {
    return !findAppResourceDirectory().isEmpty();
}

} // namespace platformInfo
