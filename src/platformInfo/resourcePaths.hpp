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

namespace platformInfo {


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
    QStringList qualifiedInstallSearchPaths() const;
    QStringList qualifiedMachineSearchPaths() const;
    QStringList qualifiedUserSearchPaths() const;

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
};

} // namespace platformInfo
