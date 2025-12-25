#include "resInventory/resLocMap.h"

ResLocMap::ResLocMap() {
}

void ResLocMap::clear() {
    m_LocList.clear();
}

// ============================================================================
// Installation tier
// ============================================================================

void ResLocMap::addLocation(const QString& key, const platformInfo::ResourceLocation& loc) {
    m_LocList.insert(key, loc);
}

void ResLocMap::removeLocation(const QString& key) {
    m_LocList.remove(key);
}

platformInfo::ResourceLocation ResLocMap::Location(const QString& key) const {
    return m_LocList.value(key);
}

QList<platformInfo::ResourceLocation> ResLocMap::allLocations() const {
    return m_LocList.values();
}

bool ResLocMap::hasLocation(const QString& key) const {
    return m_LocList.contains(key);
}

