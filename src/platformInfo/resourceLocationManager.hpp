/**
 * @file resourceLocationManager.h
 * @brief Three-tier resource location management for OpenSCAD
 * 
 * Manages resource locations at three levels:
 * 1. **Installation** - Hardcoded platform-specific paths (read-only)
 * 2. **Machine** - Admin-configurable paths for all users on the machine
 * 3. **User** - User-configurable paths in their own space
 * 
 * Each level has:
 * - A list of AVAILABLE locations (reference info, from config files)
 * - A list of ENABLED locations (user selections, stored in QSettings)
 * 
 * Resources are searched in order: Installation → Machine → User
 */

#pragma once

#include "export.hpp"
#include "resourcePaths.hpp"
#include <QString>
#include <QStringList>
#include <QList>

#include "platformInfo/platformInfo.hpp"
#include "platformInfo/ResourceLocation.hpp"

namespace platformInfo {
using resourceMetadata::ResourceTier;

/**
 * @brief Manages three-tier resource locations with QSettings persistence
 * 
 */
class PLATFORMINFO_API ResourceLocationManager {
public:
    /**
     * @brief Construct with optional settings
     *
     */
    explicit ResourceLocationManager() {}
    
    /**
     * @brief Get the default installation search path for file dialogs
     * @return Platform-specific default (e.g., "C:/Program Files/OpenSCAD")
     */
    QList<ResourceLocation> defaultInstallationSearchPaths() const;
    
    /**
     * @brief Get default machine-level locations for the current platform
     * @return List of default machine resource locations
     */
    QList<ResourceLocation> defaultMachineLocations() const {
    return QList<ResourceLocation>();
}

    /**
     * @brief Get default user-level locations for the current platform
     * @return List of default user resource locations
     */
    QList<ResourceLocation> defaultUserLocations() const {
    return QList<ResourceLocation>();
}
    
    // Stub methods for legacy template converter (low priority)
    QVector<ResourceLocation> findSiblingInstallations() const { return QVector<ResourceLocation>(); }
    QVector<ResourceLocation> enabledMachineLocations() const { return QVector<ResourceLocation>(); }
    QVector<ResourceLocation> enabledUserLocations() const { return QVector<ResourceLocation>(); }
    
    // Methods needed by main app (stubbed for now)
    void setApplicationPath(const QString &path) { /* Stub */ }
    QString findInstallationResourceDir() const { return QString(); }
    QString folderName() const { return QStringLiteral("ScadTemplates"); }
    bool isRunningFromValidInstallation() const { return false; }
    QString userSpecifiedInstallationPath() const { return QString(); }
    
    // Additional methods needed by preferences dialog
    QStringList enabledSiblingPaths() const { return QStringList(); }
    QString defaultInstallationSearchPath() const { return QString(); }
    QList<ResourceLocation> availableMachineLocations() const { return QList<ResourceLocation>(); }
    QList<ResourceLocation> availableUserLocations() const { return QList<ResourceLocation>(); }
    void setUserSpecifiedInstallationPath(const QString &path) { Q_UNUSED(path); /* Stub */ }
    void setEnabledSiblingPaths(const QStringList &paths) { Q_UNUSED(paths); /* Stub */ }
    void reloadConfiguration() { /* Stub */ }

private:

    mutable QList<ResourceLocation> m_installationLocations;
    mutable QList<ResourceLocation> m_machineLocations;
    mutable QList<ResourceLocation> m_userLocations;

    
};

} // namespace platformInfo
