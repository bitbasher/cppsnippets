/**
 * @file resourcePaths.cpp
 * @brief Implementation of ResourcePaths class
 */

#include "ResourcePaths.hpp"
#include "platformInfo/platformInfo.hpp"
#include "applicationNameInfo.hpp"

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSettings>

namespace pathDiscovery {

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

  // Unified default search paths structure indexed by tier
  // These are compile-time platform-specific constants
  inline static const QMap<resourceMetadata::ResourceTier, QStringList> s_defaultSearchPaths = {
    // Installation tier: relative to application executable
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    {resourceMetadata::ResourceTier::Installation, {
      QStringLiteral("%PROGRAMFILES%/"), // Standard install (append openscad + suffix)
      QStringLiteral("."),                // Release location (same as exe)
      QStringLiteral("../share/"),        // MSYS2 style Base share folder
      QStringLiteral("..")                // Dev location
    }},
    // Machine tier: system-wide locations for all users
    {resourceMetadata::ResourceTier::Machine, {
      QStringLiteral("C:/ProgramData/")
    }},
    // User tier: user-specific locations
    {resourceMetadata::ResourceTier::User, {
      QStringLiteral("%APPDATA%/"),
      QStringLiteral("%LOCALAPPDATA%/"),
      QStringLiteral("../")
    }}
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
    {resourceMetadata::ResourceTier::Installation, {
      QStringLiteral("../Resources"),
      QStringLiteral("../../.."),
      QStringLiteral("../../../.."),
      QStringLiteral("..")
    }},
    {resourceMetadata::ResourceTier::Machine, {
      QStringLiteral("/Library/Application Support/"),
      QStringLiteral("/usr/share/"),
      QStringLiteral("/usr/local/share/")
    }},
    {resourceMetadata::ResourceTier::User, {
      QStringLiteral("${HOME}/Library/Application Support/"),
      QStringLiteral("${HOME}/Documents/")
    }}
#else // Linux/BSD/POSIX
    {resourceMetadata::ResourceTier::Installation, {
      QStringLiteral("../share/"),
      QStringLiteral("../../share/"),
      QStringLiteral(".."),
      QStringLiteral("../..")
    }},
    {resourceMetadata::ResourceTier::Machine, {
      QStringLiteral("/usr/share/"),
      QStringLiteral("/usr/local/share/"),
      QStringLiteral("/opt/share/"),
      QStringLiteral("/opt/openscad/share/")
    }},
    {resourceMetadata::ResourceTier::User, {
      QStringLiteral("${XDG_CONFIG_HOME}/"),
      QStringLiteral("${HOME}/.local/share/"),
      QStringLiteral("../../.local/share/")
    }}
#endif
  };

const QStringList& ResourcePaths::defaultSearchPaths(resourceMetadata::ResourceTier tier) {
    static const QStringList empty;
    auto it = s_defaultSearchPaths.constFind(tier);
    return (it != s_defaultSearchPaths.constEnd()) ? it.value() : empty;
}

// ============================================================================
// Environment Variable Expansion Helper
// ============================================================================
//
// Expands ${VAR} and %VAR% style environment variable references.
// Uses Qt's QProcessEnvironment to read system environment safely.
//
// Examples:
//   "${HOME}/foo" → "/home/user/foo"
//   "%APPDATA%/bar" → "C:/Users/user/AppData/Roaming/bar"
//   "${UNDEFINED}/foo" → "/foo" (undefined vars expand to empty string)

QString ResourcePaths::expandEnvVars(const QString& path) {
    if (path.isEmpty()) {
        return path;
    }
    
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    // Pattern matches both ${VAR} and %VAR% style environment variable references
    const QRegularExpression pattern(QStringLiteral(R"(\$\{([^}]+)\}|%([^%]+)%)"));
    
    QString result;
    result.reserve(path.size());
    int lastIndex = 0;
    bool hasUndefinedVar = false;
    
    // Iterate through all matches and build the expanded string
    for (auto match = pattern.globalMatch(path); match.hasNext(); ) {
        auto m = match.next();
        
        // Append text before the match
        result.append(path.mid(lastIndex, m.capturedStart() - lastIndex));
        
        // Extract variable name from either ${VAR} or %VAR% capture group
        const QString varName = m.captured(1).isEmpty() ? m.captured(2) : m.captured(1);
        
        // Special handling for HOME on Windows - use Qt's cross-platform home path
        QString varValue;
        if (varName == QStringLiteral("HOME") && !env.contains(varName)) {
            varValue = QDir::homePath();
        } else if (!env.contains(varName)) {
            hasUndefinedVar = true;
            qWarning() << "ResourcePaths: Undefined environment variable" << varName << "in path:" << path;
            // Return empty string to signal invalid path
            return QString();
        } else {
            varValue = env.value(varName);
        }
        
        // Append the expanded value
        result.append(varValue);
        
        lastIndex = m.capturedEnd();
    }
    
    // Append any remaining text after the last match
    result.append(path.mid(lastIndex));
    
    // Normalize path separators to forward slashes for consistency
    // This ensures mixed ${VAR}/../relative paths work correctly
    result.replace(QLatin1Char('\\'), QLatin1Char('/'));
    
    // Note: Do NOT cleanPath here - we need to preserve trailing slashes
    // which indicate that folder name should be appended
    return result;
}

// ============================================================================
// Folder Name Appending Rules
// ============================================================================
//
// Paths ending with "/" are base directories that need the application folder
// name appended. This distinguishes:
//   - Base paths: "%PROGRAMFILES%/" → "%PROGRAMFILES%/ScadTemplates"
//   - Direct paths: "../.." → used as-is without modification
//
// Installation tier gets suffix applied (e.g., "ScadTemplates (Nightly)")
// Machine/User tiers use folder name only (no suffix)

QString ResourcePaths::applyFolderNameRules(const QString& path, bool applyInstallSuffix) const {
    // First expand environment variables (normalized to forward slashes, trailing slashes preserved)
    QString expanded = expandEnvVars(path);
    
    // If expansion failed (undefined variable), skip this path
    if (expanded.isEmpty()) {
        return QString();
    }
    
    // Check if this is a base path that needs folder name appended
    if (expanded.endsWith(QStringLiteral("/"))) {
        // Build folder name with optional suffix
        const QString suffix = QString::fromUtf8(appInfo::suffix);
        const QString folder = applyInstallSuffix 
            ? appInfo::getBaseName() + suffix 
            : appInfo::getBaseName();
        
        expanded += folder;
    }
    
    // Convert to QDir first to resolve relative components correctly
    QDir dir(expanded);
    
    // Get absolute path (resolves . and .. relative to the expanded base)
    QString absolutePath = dir.absolutePath();
    
    // Clean the absolute path to normalize separators
    return QDir::cleanPath(absolutePath);
}

// ============================================================================
// Sibling Installation Discovery
// ============================================================================
//
// Returns candidate sibling installation name (delegated to appInfo)
// Caller should validate if this path actually exists

QString ResourcePaths::getSiblingFolderName() const {
    return appInfo::getSiblingName();
}



// ============================================================================
// User-Designated Paths
// ============================================================================
//
// Loads additional search paths from QSettings that the user has configured.
// These are added to the Installation tier search paths.

QStringList ResourcePaths::userDesignatedPaths() {
    QSettings settings(appInfo::getBaseName(), QStringLiteral("ResourcePaths"));
    return settings.value(QStringLiteral("user_designated_paths"), QStringList()).toStringList();
}

// ============================================================================
// Qualified Search Paths (Environment Expanded + Folder Names Applied)
// ============================================================================
//
// These methods produce fully qualified absolute paths ready for discovery:
// 1. Expand environment variables (${HOME}, %APPDATA%, etc.)
// 2. Apply folder name rules (append "ScadTemplates" + suffix where needed)
// 3. Clean paths (remove redundant separators, resolve . and ..)
// 4. Add executable directory path (QCoreApplication::applicationDirPath())
// 5. Add sibling installations (LTS ↔ Nightly)
// 6. Add user-designated paths from settings (User tier)
// 7. Deduplicate paths to avoid redundant scanning
//
// Installation tier: applies suffix (e.g., "ScadTemplates (Nightly)")
// Machine/User tiers: folder name only (no suffix)
// User-designated paths: added to User tier (read-write, per-user)
//
// Note: "." is resolved relative to CWD, allowing users to test resources
// anywhere by running the app from that directory. The executable location
// is always added separately via applicationDirPath().

QList<PathElement> ResourcePaths::qualifiedSearchPaths() const {
    QList<PathElement> qualified;
    QSet<QString> seenPaths; // Track paths to prevent duplicates
    
    // Helper lambda to add path only if not already present
    auto addIfUnique = [&](resourceMetadata::ResourceTier tier, const QString& path) {
        if (!path.isEmpty() && !seenPaths.contains(path)) {
            seenPaths.insert(path);
            qualified.append(PathElement(tier, path));
        }
    };
    
    // Process Installation tier paths (with suffix)
    for (const QString& path : defaultSearchPaths(resourceMetadata::ResourceTier::Installation)) {
        QString qualified_path = applyFolderNameRules(path, true);
        addIfUnique(resourceMetadata::ResourceTier::Installation, qualified_path);
        
        // Add sibling installation for base directories (paths ending with /)
        if (path.endsWith('/')) {
            QString siblingFolderName = getSiblingFolderName();
            QString sibling_base = expandEnvVars(path) + siblingFolderName;
            QString sibling_path = QDir::cleanPath(QDir(sibling_base).absolutePath());
            addIfUnique(resourceMetadata::ResourceTier::Installation, sibling_path);
        }
    }
    
    // Add executable directory path (always check where the exe actually is)
    // This ensures we find resources even if launched with different CWD
    QString exePath = QDir::cleanPath(QCoreApplication::applicationDirPath());
    addIfUnique(resourceMetadata::ResourceTier::Installation, exePath);
    
    // Process Machine tier paths (no suffix)
    for (const QString& path : defaultSearchPaths(resourceMetadata::ResourceTier::Machine)) {
        QString qualified_path = applyFolderNameRules(path, false);
        addIfUnique(resourceMetadata::ResourceTier::Machine, qualified_path);
    }
    
    // Process User tier paths (no suffix)
    for (const QString& path : defaultSearchPaths(resourceMetadata::ResourceTier::User)) {
        QString qualified_path = applyFolderNameRules(path, false);
        addIfUnique(resourceMetadata::ResourceTier::User, qualified_path);
    }
    
    // Add user's Documents folder (with trailing slash for folder name appending)
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!documentsPath.isEmpty()) {
        QString docPath = applyFolderNameRules(documentsPath + QStringLiteral("/"), false);
        addIfUnique(resourceMetadata::ResourceTier::User, docPath);
    }
    
    // Add user-designated paths with tier classification
    // These can be Installation, Machine, or User tier depending on the path
    QStringList userDesignated = userDesignatedPaths();
    for (const QString& path : userDesignated) {
        QString qualified_path = applyFolderNameRules(path, false);
        
        // Determine tier from path characteristics
        resourceMetadata::ResourceTier tier = resourceMetadata::ResourceTier::User; // default
        
        QString lowerPath = path.toLower();
        if (lowerPath.contains("programfiles") || lowerPath.contains("/inst/") || 
            lowerPath.contains("\\inst\\") || lowerPath.contains("program files")) {
            tier = resourceMetadata::ResourceTier::Installation;
        } else if (lowerPath.contains("programdata") || lowerPath.contains("appdata\\local") || 
                   lowerPath.contains("/local/") || lowerPath.contains("\\local\\")) {
            tier = resourceMetadata::ResourceTier::Machine;
        } else if (lowerPath.contains("documents") || lowerPath.contains("appdata\\roaming") || 
                   lowerPath.contains("/appdata/") || lowerPath.contains("\\appdata\\")) {
            tier = resourceMetadata::ResourceTier::User;
        }
        
        addIfUnique(tier, qualified_path);
    }
    
    return qualified;
}

} // namespace pathDiscovery
