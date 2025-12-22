#ifndef RESLOCMAP_H
#define RESLOCMAP_H

#include <QMap>
#include <QString>
#include "platformInfo/export.h"
#include "resInventory/ResourceLocation.h"

/**
 * @brief Storage class for resource locations organized by tier
 * 
 * Provides three QMap<QString, ResourceLocation> members for 
 * installation, machine, and user tiers.
 */
class RESOURCEMGMT_API ResLocMap {
public:
    ResLocMap();
    
    void clear();
    
    // Installation tier locations
    void addInstallLocation(const QString& key, const platformInfo::ResourceLocation& loc);
    void removeInstallLocation(const QString& key);
    platformInfo::ResourceLocation installLocation(const QString& key) const;
    QList<platformInfo::ResourceLocation> allInstallLocations() const;
    bool hasInstallLocation(const QString& key) const;
    
    // Machine tier locations
    void addMachineLocation(const QString& key, const platformInfo::ResourceLocation& loc);
    void removeMachineLocation(const QString& key);
    platformInfo::ResourceLocation machineLocation(const QString& key) const;
    QList<platformInfo::ResourceLocation> allMachineLocations() const;
    bool hasMachineLocation(const QString& key) const;
    
    // User tier locations
    void addUserLocation(const QString& key, const platformInfo::ResourceLocation& loc);
    void removeUserLocation(const QString& key);
    platformInfo::ResourceLocation userLocation(const QString& key) const;
    QList<platformInfo::ResourceLocation> allUserLocations() const;
    bool hasUserLocation(const QString& key) const;

private:
    QMap<QString, platformInfo::ResourceLocation> m_installLocList;
    QMap<QString, platformInfo::ResourceLocation> m_machineLocList;
    QMap<QString, platformInfo::ResourceLocation> m_userLocList;
};

#endif // RESLOCMAP_H
