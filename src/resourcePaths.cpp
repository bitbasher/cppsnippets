/**
 * @file resourcePaths.cpp
 * @brief Implementation of ResourcePaths class
 */

#include "platformInfo/resourcePaths.h"
#include "platformInfo/platformInfo.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <shlobj.h>
#include <windows.h>
#endif

namespace platformInfo {

// Static resource type definitions with file extensions
static const QVector<ResourceTypeInfo> s_resourceTypes = {
    { ResourceType::Examples,     
      QStringLiteral("examples"),       
      QStringLiteral("Example OpenSCAD scripts"),
      { QStringLiteral(".scad") },
      { QStringLiteral(".json"), QStringLiteral(".txt"), QStringLiteral(".dat") } },
      
    { ResourceType::Tests,        
      QStringLiteral("tests"),          
      QStringLiteral("Test OpenSCAD scripts"),
      { QStringLiteral(".scad") },
      { QStringLiteral(".json"), QStringLiteral(".txt"), QStringLiteral(".dat") } },
      
    { ResourceType::Fonts,        
      QStringLiteral("fonts"),          
      QStringLiteral("Font files (supplements OS fonts)"),
      { QStringLiteral(".ttf"), QStringLiteral(".otf") },
      {} },
      
    { ResourceType::ColorSchemes, 
      QStringLiteral("color-schemes"),  
      QStringLiteral("Editor color schemes"),
      { QStringLiteral(".json") },
      {} },
      
    { ResourceType::Shaders,      
      QStringLiteral("shaders"),        
      QStringLiteral("OpenGL shader files"),
      { QStringLiteral(".frag"), QStringLiteral(".vert") },
      {} },
      
    { ResourceType::Templates,    
      QStringLiteral("templates"),      
      QStringLiteral("Template files"),
      { QStringLiteral(".scad"), QStringLiteral(".json") },
      {} },
      
    { ResourceType::Libraries,    
      QStringLiteral("libraries"),      
      QStringLiteral("OpenSCAD library scripts that extend features"),
      { QStringLiteral(".scad") },
      {} },
      
    { ResourceType::Translations, 
      QStringLiteral("locale"),         
      QStringLiteral("Translation files"),
      { QStringLiteral(".qm"), QStringLiteral(".ts") },
      {} }
};

ResourcePaths::ResourcePaths()
    : m_suffix()  // Empty suffix = release build
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
    if (m_applicationPath != path) {
        m_applicationPath = path;
        m_appResourceDirCached = false;
        m_cachedAppResourceDir.clear();
    }
}

void ResourcePaths::setSuffix(const QString& suffix) {
    if (m_suffix != suffix) {
        m_suffix = suffix;
        m_appResourceDirCached = false;
        m_cachedAppResourceDir.clear();
        m_userConfigPathCached = false;
        m_cachedUserConfigPath.clear();
    }
}

QString ResourcePaths::folderName() const {
    // OPENSCAD_FOLDER_NAME = "OpenSCAD" + OPENSCAD_SUFFIX
    return QStringLiteral("OpenSCAD") + m_suffix;
}

void ResourcePaths::detectOSType() {
    PlatformInfo info;
    m_osType = info.currentOSType();
}

QVector<ResourceTypeInfo> ResourcePaths::allResourceTypes() {
    return s_resourceTypes;
}

const ResourceTypeInfo* ResourcePaths::resourceTypeInfo(ResourceType type) {
    for (const auto& info : s_resourceTypes) {
        if (info.type == type) {
            return &info;
        }
    }
    return nullptr;
}

QString ResourcePaths::resourceSubdirectory(ResourceType type) {
    const auto* info = resourceTypeInfo(type);
    return info ? info->subdirectory : QString();
}

QStringList ResourcePaths::resourceExtensions(ResourceType type) {
    const auto* info = resourceTypeInfo(type);
    return info ? info->primaryExtensions : QStringList();
}

QString ResourcePaths::buildSharePath(const QString& basePath) const {
    // Equivalent to RESOURCE_FOLDER("../share/openscad"):
    // basePath already ends with "openscad", we just append the suffix
    return basePath + m_suffix;
}

QStringList ResourcePaths::appSearchPaths() const {
    return searchPathsForPlatform(m_osType, m_suffix);
}

QStringList ResourcePaths::searchPathsForPlatform(ExtnOSType osType, const QString& suffix) {
    QStringList paths;
    
    // RESOURCE_FOLDER macro in OpenSCAD:
    //   #ifdef OPENSCAD_SUFFIX
    //     #define RESOURCE_FOLDER(path) path OPENSCAD_SUFFIX
    //   #else
    //     #define RESOURCE_FOLDER(path) path
    //   #endif
    //
    // Usage: RESOURCE_FOLDER("../share/openscad") expands to:
    //   "../share/openscad" (release) or "../share/openscad (Nightly)" (nightly)
    //
    // Helper lambda mimics this behavior:
    auto resourceFolder = [&suffix](const QString& path) -> QString {
        return path + suffix;
    };
    
    // These search paths match OpenSCAD's PlatformUtils.cc exactly:
    // #ifdef __APPLE__ / #ifdef _WIN32 / #else (POSIX)
    
    switch (osType) {
        case ExtnOSType::MacOS:
            // macOS search paths (from OpenSCAD #ifdef __APPLE__)
            paths << QStringLiteral("../Resources");                                    // Resources bundled in .app
            paths << QStringLiteral("../../..");                                        // Dev location
            paths << QStringLiteral("../../../..");                                     // Test location (cmake)
            paths << QStringLiteral("..");                                              // Test location
            paths << resourceFolder(QStringLiteral("../share/openscad"));               // Unix mode
            break;
            
        case ExtnOSType::Windows:
            // Windows search paths (from OpenSCAD #ifdef _WIN32)
            paths << QStringLiteral(".");                                               // Release location
            paths << resourceFolder(QStringLiteral("../share/openscad"));               // MSYS2
            paths << QStringLiteral("..");                                              // Dev location
            break;
            
        case ExtnOSType::Linux:
        case ExtnOSType::BSD:
        case ExtnOSType::Solaris:
        default:
            // Linux/POSIX search paths (from OpenSCAD #else)
            paths << resourceFolder(QStringLiteral("../share/openscad"));               // Standard install
            paths << resourceFolder(QStringLiteral("../../share/openscad"));            // Alternate install
            paths << QStringLiteral(".");                                               // Build directory
            paths << QStringLiteral("..");                                              // Up one
            paths << QStringLiteral("../..");                                           // Up two
            break;
    }
    
    return paths;
}

QString ResourcePaths::findAppResourceDirectory() const {
    // Return cached value if available
    if (m_appResourceDirCached) {
        return m_cachedAppResourceDir;
    }
    
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
            // Check for examples, fonts, or color-schemes subdirectory as validation
            if (candidateDir.exists(QStringLiteral("examples")) ||
                candidateDir.exists(QStringLiteral("fonts")) ||
                candidateDir.exists(QStringLiteral("color-schemes"))) {
                m_cachedAppResourceDir = candidateDir.absolutePath();
                m_appResourceDirCached = true;
                return m_cachedAppResourceDir;
            }
        }
    }
    
    m_appResourceDirCached = true;
    m_cachedAppResourceDir.clear();
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
    if (m_userConfigPathCached) {
        return m_cachedUserConfigPath;
    }
    
    m_cachedUserConfigPath = userConfigBasePathForPlatform(m_osType);
    m_userConfigPathCached = true;
    return m_cachedUserConfigPath;
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
#ifdef Q_OS_WIN
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
#ifdef Q_OS_WIN
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

// ========== Validation ==========

bool ResourcePaths::isValid() const {
    return !findAppResourceDirectory().isEmpty();
}

} // namespace platformInfo
