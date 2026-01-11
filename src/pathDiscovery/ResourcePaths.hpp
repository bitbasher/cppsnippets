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

#include "platformInfo/export.hpp"
#include "resourceMetadata/ResourceTypeInfo.hpp"
#include "resourceMetadata/ResourceTier.hpp"
#include "resourceMetadata/ResourceAccess.hpp"
#include "pathDiscovery/PathElement.hpp"

#include <QDir>
#include <QString>
#include <QStringList>
#include <QMap>

namespace pathDiscovery {

// Import resource metadata types for convenience
using resourceMetadata::ResourceType;
using resourceMetadata::ResourceTypeInfo;
using resourceMetadata::s_topLevel;
using resourceMetadata::s_nonContainer;
using resourceMetadata::s_exampleSub;
using resourceMetadata::s_testSub;
using resourceMetadata::s_attachments;
using resourceMetadata::groupNameCapture;




class PLATFORMINFO_API ResourcePaths {
public:

    // Immutable default search paths by tier (compile-time constants with env var templates)
    static const QStringList &defaultInstallSearchPaths();
    static const QStringList &defaultMachineSearchPaths();
    static const QStringList &defaultUserSearchPaths();
    
    // Resolved search paths with environment variables expanded (for testing/debugging)
    // These expand env vars but do NOT apply folder name suffix rules
    // For actual discovery, use qualifiedSearchPaths() instead
    static QStringList resolvedInstallSearchPaths();
    static QStringList resolvedMachineSearchPaths();
    static QStringList resolvedUserSearchPaths();
    
    // PRIMARY API: Single consolidated output of all qualified search paths
    // Returns QList<PathElement> with tier embedded in each element
    // - Expands environment variables to absolute paths
    // - Applies folder name suffix rules per tier
    // - Includes sibling installations (LTS ↔ Nightly)
    // - Includes user-designated paths from QSettings
    // This is the input list for ResourceScanner discovery
    QList<PathElement> qualifiedSearchPaths() const;
    
    // User-designated paths loaded from QSettings
    static QStringList userDesignatedPaths();

private:

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
};

} // namespace pathDiscovery
