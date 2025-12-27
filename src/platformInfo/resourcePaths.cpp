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

const ResourceTypeInfo* ResourcePaths::resourceTypeInfo(ResourceType type) {
    return resourceInfo::ResourceTypeRegistry::info(type);
}

QString ResourcePaths::resourceSubdirectory(ResourceType type) {
    return resourceInfo::ResourceTypeRegistry::subdir(type);
}

QStringList ResourcePaths::resourceExtensions(ResourceType type) {
    return resourceInfo::ResourceTypeRegistry::extensions(type);
}

QList<ResourceType> ResourcePaths::allTopLevelResourceTypes() {
    return resourceInfo::ResourceTypeRegistry::topLevelTypes();
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

// Constants moved to header as inline static members
const QStringList& ResourcePaths::defaultInstallSearchPaths() {
    return s_defaultInstallSearchPaths;
}

const QStringList& ResourcePaths::defaultMachineSearchPaths() {
    return s_defaultMachineSearchPaths;
}

const QStringList& ResourcePaths::defaultUserSearchPaths() {
    return s_defaultUserSearchPaths;
}

QStringList ResourcePaths::expandedUserSearchPaths() const {
    QStringList expanded;
    expanded.reserve(s_defaultUserSearchPaths.size());
    
    for (const QString& path : s_defaultUserSearchPaths) {
        expanded.append(expandEnvVars(path));
    }
    
    return expanded;
}

QStringList ResourcePaths::expandedMachineSearchPaths() const {
    QStringList expanded;
    expanded.reserve(s_defaultMachineSearchPaths.size());
    
    for (const QString& path : s_defaultMachineSearchPaths) {
        expanded.append(expandEnvVars(path));
    }
    
    return expanded;
}

QList<ResourcePathElement> ResourcePaths::defaultElements() const {
    QList<ResourcePathElement> result;

    // Installation tier (relative to application path, with suffix rules)
    if (!m_applicationPath.isEmpty()) {
        QDir appDir(m_applicationPath);
        for (const QString& raw : s_defaultInstallSearchPaths) {
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
    }

    // Machine tier
    for (const QString& raw : s_defaultMachineSearchPaths) {
        QString expanded = QDir::cleanPath(expandEnvVars(raw));
        QFileInfo fi(expanded);
        QString finalPath = fi.canonicalFilePath();
        if (finalPath.isEmpty()) finalPath = fi.absoluteFilePath();
        ResourceLocation loc(finalPath, QStringLiteral("Machine"), QString(), resourceInfo::ResourceTier::Machine);
        result.append(ResourcePathElement(loc, resourceInfo::ResourceTier::Machine));
    }

    // User tier
    for (const QString& raw : s_defaultUserSearchPaths) {
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
    QStringList paths;
    paths.reserve(s_defaultInstallSearchPaths.size());
    for (const QString& raw : s_defaultInstallSearchPaths) {
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

QString ResourcePaths::resolveUserConfigBasePath() const {
    return userConfigBasePathForPlatform(m_osType);
}

QString ResourcePaths::userConfigBasePathForPlatform(ExtnOSType osType) {
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

QString ResourcePaths::userConfigBasePath() const {
    return resolveUserConfigBasePath();
}

QString ResourcePaths::userOpenSCADPath() const {
    QString configPath = userConfigBasePath();
    if (configPath.isEmpty()) {
        return QString();
    }
    
    // This is equivalent to OpenSCAD's PlatformUtils::userConfigPath()
    // which returns the config base path + "/" + OPENSCAD_FOLDER_NAME
    // folderName() returns "OpenSCAD" + m_suffix
    QDir dir(configPath);
    return dir.absoluteFilePath(folderName());
}

QString ResourcePaths::userResourcePath(ResourceType type) const {
    QString openscadPath = userOpenSCADPath();
    if (openscadPath.isEmpty()) {
        return QString();
    }
    
    QString subdir = resourceSubdirectory(type);
    if (subdir.isEmpty()) {
        return QString();
    }
    
    QDir dir(openscadPath);
    return dir.absoluteFilePath(subdir);
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

bool ResourcePaths::isValid() const {
    return !findAppResourceDirectory().isEmpty();
}

} // namespace platformInfo
