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

/**
 * @brief Resource location tier/level
 */
enum class ResourceTier {
    Installation,   ///< Built-in application resources (read-only, hardcoded)
    Machine,        ///< System-wide resources for all users (admin-managed)
    User            ///< Per-user resources (user-managed)
};


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
    

private:

    mutable QList<ResourceLocation> m_installationLocations;
    mutable QList<ResourceLocation> m_machineLocations;
    mutable QList<ResourceLocation> m_userLocations;

    
};

} // namespace platformInfo
