/**
 * @file resourcePaths.h
 * @brief Resource type metadata and immutable default search paths
 * 
 * Provides platform-specific search paths and resource type definitions
 * for locating OpenSCAD resources (examples, fonts, color schemes, etc.)
 * 
 * Application naming information is now provided by applicationNameInfo.hpp
 * (generated from applicationNameInfo.hpp.in during CMake configuration)
 */

#pragma once

#include "export.hpp"
#include "ResourceTypeInfo.hpp"

#include <QDir>
#include <QString>
#include <QStringList>
#include <QMap>

namespace resourceInfo {

/**
 * @brief Resource location tier/level
 * 
 * Resources are organized into three tiers based on their scope and accessibility:
 * - Installation: Built-in resources from the application installation (read-only)
 * - Machine: System-wide resources available to all users (read-only)
 * - User: Personal resources in the user's home directory (read-write)
 */
enum class ResourceTier {
    Installation,   ///< Built-in application resources (read-only)
    Machine,        ///< System-wide resources for all users (read-only)
    User            ///< Per-user resources (read-write)
};

/**
 * @brief Access permissions for resources
 */
enum class Access {
    Unknown = 0,  ///< Access level not yet determined
    ReadOnly,     ///< Read-only access
    ReadWrite,    ///< Read and write access
    WriteOnly,    ///< Write-only access
    FullAccess,   ///< All permissions (read, write, execute)
    NoAccess      ///< No access
};

/**
 * @brief Default access level for each resource tier
 * 
 * Maps resource tiers to their default access permissions.
 * Actual discovered resources may have different permissions.
 */
static const QMap<ResourceTier, Access> accessByTier = {
    { ResourceTier::Installation, Access::ReadOnly },
    { ResourceTier::Machine, Access::ReadOnly },
    { ResourceTier::User, Access::ReadWrite }
};

} // namespace resourceInfo

namespace platformInfo {

/**
 * @brief Represents a single search path with its tier and access info
 * 
 * Used to track search paths through expansion, qualification, and discovery.
 * Access starts as Unknown and may be updated during discovery.
 */
class PLATFORMINFO_API PathElement {
public:
    PathElement(resourceInfo::ResourceTier tier, const QString& path)
        : m_tier(tier), m_path(path) {}
    
    resourceInfo::ResourceTier tier() const { return m_tier; }
    QString path() const { return m_path; }
    
private:
    resourceInfo::ResourceTier m_tier;
    QString m_path;
};




class PLATFORMINFO_API ResourcePaths {
public:

    QString folderName() const;
    
    // Application suffix (e.g., " (Nightly)") for Installation tier paths
    void setSuffix(const QString& suffix);
    QString suffix() const;

    static const ResourceTypeInfo *resourceTypeInfo(ResourceType type);
    static QString resourceSubdirectory(ResourceType type);
    static QStringList resourceExtensions(ResourceType type);
    static QList<ResourceType> allTopLevelResourceTypes();

    // Immutable default search paths by tier (compile-time constants with env var templates)
    static const QStringList &defaultInstallSearchPaths();
    static const QStringList &defaultMachineSearchPaths();
    static const QStringList &defaultUserSearchPaths();
    
    // Resolved search paths with environment variables expanded to absolute paths
    // These expand env vars but do NOT apply folder name suffix rules
    static QStringList resolvedInstallSearchPaths();
    static QStringList resolvedMachineSearchPaths();
    static QStringList resolvedUserSearchPaths();
    
    // Fully qualified paths: env vars expanded + folder names appended per tier rules
    // Installation tier: paths ending with "/" get folderName() + suffix appended
    // Machine/User tiers: paths ending with "/" get folderName() appended (no suffix)
    // Includes sibling installations (LTS ↔ Nightly) and user-designated paths from settings
    QList<PathElement> qualifiedSearchPaths() const;
    
    // User-designated paths loaded from QSettings
    static QStringList userDesignatedPaths();

private:

    QString m_suffix;  // Empty for release, " (Nightly)" for nightly builds

    static const QStringList &s_defaultInstallSearchPaths;
    static const QStringList &s_defaultMachineSearchPaths;
    static const QStringList &s_defaultUserSearchPaths;
    
    // Environment variable expansion helper
    // Expands ${VAR} and %VAR% style environment variables to their values
    // Uses system environment from QProcessEnvironment::systemEnvironment()
    static QString expandEnvVars(const QString& path);
    
    // Folder name appending helper
    // Paths ending with "/" get folderName (+ optional suffix) appended
    // Paths without trailing "/" are returned with just env var expansion
    QString applyFolderNameRules(const QString& path, bool applyInstallSuffix) const;
    
    // Sibling installation helper
    // Given "OpenSCAD" returns "OpenSCAD (Nightly)", and vice versa
    QString getSiblingFolderName() const;
    
    // Check if a path template should trigger sibling discovery
    // Returns true for platform-specific program files locations
    static bool isSiblingCandidatePath(const QString& path);
};

} // namespace platformInfo
