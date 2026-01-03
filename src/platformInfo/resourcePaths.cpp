/**
 * @file resourcePaths.cpp
 * @brief Implementation of ResourcePaths class
 */

#include "resourcePaths.hpp"
#include "platformInfo.hpp"
#include "resourceTier.h"

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStandardPaths>

namespace platformInfo {

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
  inline static const QMap<resourceInfo::ResourceTier, QStringList> s_defaultSearchPaths = {
    // Installation tier: relative to application executable
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    {resourceInfo::ResourceTier::Installation, {
      QStringLiteral("%PROGRAMFILES%/"), // Standard install (append openscad + suffix)
      QStringLiteral("."),                // Release location (same as exe)
      QStringLiteral("../share/"),        // MSYS2 style Base share folder
      QStringLiteral("..")                // Dev location
    }},
    // Machine tier: system-wide locations for all users
    {resourceInfo::ResourceTier::Machine, {
      QStringLiteral("%PROGRAMDATA%/ScadTemplates"),
      QStringLiteral("C:/ProgramData/")
    }},
    // User tier: user-specific locations
    {resourceInfo::ResourceTier::User, {
      QStringLiteral("%APPDATA%/ScadTemplates"),
      QStringLiteral("%LOCALAPPDATA%/ScadTemplates"),
      QStringLiteral("."),
      QStringLiteral("../")
    }}
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
    {resourceInfo::ResourceTier::Installation, {
      QStringLiteral("../Resources"),
      QStringLiteral("../../.."),
      QStringLiteral("../../../.."),
      QStringLiteral(".."),
      QStringLiteral("../share/")
    }},
    {resourceInfo::ResourceTier::Machine, {
      QStringLiteral("/Library/Application Support/ScadTemplates")
    }},
    {resourceInfo::ResourceTier::User, {
      QStringLiteral("${HOME}/Library/Application Support/ScadTemplates"),
      QStringLiteral("."),
      QStringLiteral("../../Documents/")
    }}
#else // Linux/BSD/POSIX
    {resourceInfo::ResourceTier::Installation, {
      QStringLiteral("../share/"),
      QStringLiteral("../../share/"),
      QStringLiteral("."),
      QStringLiteral(".."),
      QStringLiteral("../..")
    }},
    {resourceInfo::ResourceTier::Machine, {
      QStringLiteral("/usr/share/"),
      QStringLiteral("/usr/local/share/"),
      QStringLiteral("/opt/openscad/share/")
    }},
    {resourceInfo::ResourceTier::User, {
      QStringLiteral("${XDG_CONFIG_HOME}/cppsnippets"),
      QStringLiteral("${HOME}/.config/cppsnippets"),
      QStringLiteral("${HOME}/.local/share/cppsnippets"),
      QStringLiteral("."),
      QStringLiteral("../../.local/share/")
    }}
#endif
  };
const QStringList& ResourcePaths::s_defaultInstallSearchPaths = s_defaultSearchPaths.value(resourceInfo::ResourceTier::Installation);
const QStringList& ResourcePaths::s_defaultMachineSearchPaths = s_defaultSearchPaths.value(resourceInfo::ResourceTier::Machine);
const QStringList& ResourcePaths::s_defaultUserSearchPaths    = s_defaultSearchPaths.value(resourceInfo::ResourceTier::User);

QString ResourcePaths::folderName() const {
    // Folder name is derived from application base name in applicationNameInfo.hpp
    // For now returning the hardcoded value; this can be made dynamic later if needed
    return QStringLiteral("ScadTemplates");
}

const QStringList& ResourcePaths::defaultInstallSearchPaths() {
    return s_defaultInstallSearchPaths;
}

const QStringList& ResourcePaths::defaultMachineSearchPaths() {
    return s_defaultMachineSearchPaths;
}

const QStringList& ResourcePaths::defaultUserSearchPaths() {
    return s_defaultUserSearchPaths;
}



const ResourceTypeInfo *ResourcePaths::resourceTypeInfo(ResourceType type) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(type);
    return it == ResourceTypeInfo::s_resourceTypes.constEnd() ? nullptr : &(*it);
}

QString ResourcePaths::resourceSubdirectory(ResourceType type) {
    const ResourceTypeInfo *info = resourceTypeInfo(type);
    return info ? info->getSubDir() : QString();
}

QStringList ResourcePaths::resourceExtensions(ResourceType type) {
    const ResourceTypeInfo *info = resourceTypeInfo(type);
    return info ? info->getPrimaryExtensions() : QStringList{};
}

QList<ResourceType> ResourcePaths::allTopLevelResourceTypes() {
    return s_topLevel;
}





} // namespace platformInfo
