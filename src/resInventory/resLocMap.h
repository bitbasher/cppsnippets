#ifndef RESLOCMAP_H
#define RESLOCMAP_H

#include <QMap>
#include <QString>
#include "platformInfo/export.h"
#include "resInventory/ResourceLocation.h"

/**
 * @brief Storage class for resource locations organized by tier
 * 
 * Provides three QMap<QString, ResourceLocation> members.
 */
class RESOURCEMGMT_API ResLocMap {
public:
    ResLocMap();
    
    void clear();
    
    // Installation tier locations
    void addLocation(const QString& key, const platformInfo::ResourceLocation& loc);
    void removeLocation(const QString& key);
    platformInfo::ResourceLocation Location(const QString& key) const;
    QList<platformInfo::ResourceLocation> allLocations() const;
    bool hasLocation(const QString& key) const;

private:
    QMap<QString, platformInfo::ResourceLocation> m_LocList;
};

#endif // RESLOCMAP_H
