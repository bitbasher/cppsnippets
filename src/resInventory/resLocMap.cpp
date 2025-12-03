#include "resInventory/resLocMap.h"

ResLocMap::ResLocMap() {
}

void ResLocMap::clear() {
    m_installLocList.clear();
    m_machineLocList.clear();
    m_userLocList.clear();
}

// ============================================================================
// Installation tier
// ============================================================================

void ResLocMap::addInstallLocation(const QString& key, const platformInfo::ResourceLocation& loc) {
    m_installLocList.insert(key, loc);
}

void ResLocMap::removeInstallLocation(const QString& key) {
    m_installLocList.remove(key);
}

platformInfo::ResourceLocation ResLocMap::installLocation(const QString& key) const {
    return m_installLocList.value(key);
}

QList<platformInfo::ResourceLocation> ResLocMap::allInstallLocations() const {
    return m_installLocList.values();
}

bool ResLocMap::hasInstallLocation(const QString& key) const {
    return m_installLocList.contains(key);
}

// ============================================================================
// Machine tier
// ============================================================================

void ResLocMap::addMachineLocation(const QString& key, const platformInfo::ResourceLocation& loc) {
    m_machineLocList.insert(key, loc);
}

void ResLocMap::removeMachineLocation(const QString& key) {
    m_machineLocList.remove(key);
}

platformInfo::ResourceLocation ResLocMap::machineLocation(const QString& key) const {
    return m_machineLocList.value(key);
}

QList<platformInfo::ResourceLocation> ResLocMap::allMachineLocations() const {
    return m_machineLocList.values();
}

bool ResLocMap::hasMachineLocation(const QString& key) const {
    return m_machineLocList.contains(key);
}

// ============================================================================
// User tier
// ============================================================================

void ResLocMap::addUserLocation(const QString& key, const platformInfo::ResourceLocation& loc) {
    m_userLocList.insert(key, loc);
}

void ResLocMap::removeUserLocation(const QString& key) {
    m_userLocList.remove(key);
}

platformInfo::ResourceLocation ResLocMap::userLocation(const QString& key) const {
    return m_userLocList.value(key);
}

QList<platformInfo::ResourceLocation> ResLocMap::allUserLocations() const {
    return m_userLocList.values();
}

bool ResLocMap::hasUserLocation(const QString& key) const {
    return m_userLocList.contains(key);
}
