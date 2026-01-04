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

    static const ResourceTypeInfo *resourceTypeInfo(ResourceType type);
    static QString resourceSubdirectory(ResourceType type);
    static QStringList resourceExtensions(ResourceType type);
    static QList<ResourceType> allTopLevelResourceTypes();

    // Immutable default search paths by tier (compile-time constants with env var templates)
    static const QStringList &defaultInstallSearchPaths();
    static const QStringList &defaultMachineSearchPaths();
    static const QStringList &defaultUserSearchPaths();
    
    // Resolved search paths with environment variables expanded to absolute paths
    static QStringList resolvedInstallSearchPaths();
    static QStringList resolvedMachineSearchPaths();
    static QStringList resolvedUserSearchPaths();

private:

    static const QStringList &s_defaultInstallSearchPaths;
    static const QStringList &s_defaultMachineSearchPaths;
    static const QStringList &s_defaultUserSearchPaths;
    
    // Environment variable expansion helper
    // Expands ${VAR} and %VAR% style environment variables to their values
    // Uses system environment from QProcessEnvironment::systemEnvironment()
    static QString expandEnvVars(const QString& path);
};

} // namespace platformInfo
