/**
 * @file resourceStore.cpp
 * @brief Implementation of ResourceStore
 */

#include "resInventory/resourceStore.h"

#include <QReadLocker>
#include <QWriteLocker>
#include <QSet>

namespace resInventory {

ResourceStore::ResourceStore(QObject* parent)
    : QObject(parent)
{
}

// ============================================================================
// Adding resources
// ============================================================================

void ResourceStore::addResource(const DiscoveredResource& resource)
{
    QWriteLocker locker(&m_lock);
    
    m_resources[resource.type].append(resource);
    m_pathIndex[resource.path] = resource.type;
    
    locker.unlock();
    emit resourceAdded(resource);
}

void ResourceStore::addResources(const QVector<DiscoveredResource>& resources)
{
    if (resources.isEmpty()) {
        return;
    }
    
    QWriteLocker locker(&m_lock);
    
    for (const auto& resource : resources) {
        m_resources[resource.type].append(resource);
        m_pathIndex[resource.path] = resource.type;
    }
    
    locker.unlock();
    
    // Emit signals after releasing lock
    for (const auto& resource : resources) {
        emit resourceAdded(resource);
    }
}

int ResourceStore::scanAndStore(ResourceScannerDirListing& scanner,
                                 const QString& basePath,
                                 ResourceTier tier,
                                 const QString& locationKey)
{
    int count = 0;
    
    scanner.scanLocation(basePath, tier, locationKey,
                         [this, &count](const DiscoveredResource& res) {
                             addResource(res);
                             ++count;
                         });
    
    return count;
}

int ResourceStore::scanTypeAndStore(ResourceScannerDirListing& scanner,
                                     const QString& basePath,
                                     ResourceType type,
                                     ResourceTier tier,
                                     const QString& locationKey)
{
    int count = 0;
    
    scanner.scanLocationForType(basePath, type, tier, locationKey,
                                [this, &count](const DiscoveredResource& res) {
                                    addResource(res);
                                    ++count;
                                });
    
    return count;
}

// ============================================================================
// Querying resources
// ============================================================================

QVector<DiscoveredResource> ResourceStore::resourcesOfType(ResourceType type) const
{
    QReadLocker locker(&m_lock);
    return m_resources.value(type);
}

QVector<DiscoveredResource> ResourceStore::resourcesOfType(ResourceType type, 
                                                            ResourceTier tier) const
{
    QReadLocker locker(&m_lock);
    
    QVector<DiscoveredResource> result;
    const auto& resources = m_resources.value(type);
    
    for (const auto& res : resources) {
        if (res.tier == tier) {
            result.append(res);
        }
    }
    
    return result;
}

QVector<DiscoveredResource> ResourceStore::resourcesByLocation(ResourceType type,
                                                                const QString& locationKey) const
{
    QReadLocker locker(&m_lock);
    
    QVector<DiscoveredResource> result;
    const auto& resources = m_resources.value(type);
    
    for (const auto& res : resources) {
        if (res.locationKey == locationKey) {
            result.append(res);
        }
    }
    
    return result;
}

QVector<DiscoveredResource> ResourceStore::resourcesByCategory(ResourceType type,
                                                                const QString& category) const
{
    QReadLocker locker(&m_lock);
    
    QVector<DiscoveredResource> result;
    const auto& resources = m_resources.value(type);
    
    for (const auto& res : resources) {
        if (res.category == category) {
            result.append(res);
        }
    }
    
    return result;
}

QVector<DiscoveredResource> ResourceStore::allResources() const
{
    QReadLocker locker(&m_lock);
    
    QVector<DiscoveredResource> result;
    
    for (auto it = m_resources.constBegin(); it != m_resources.constEnd(); ++it) {
        result.append(it.value());
    }
    
    return result;
}

const DiscoveredResource* ResourceStore::findByPath(const QString& path) const
{
    QReadLocker locker(&m_lock);
    
    auto typeIt = m_pathIndex.constFind(path);
    if (typeIt == m_pathIndex.constEnd()) {
        return nullptr;
    }
    
    const auto& resources = m_resources.value(typeIt.value());
    for (const auto& res : resources) {
        if (res.path == path) {
            return &res;
        }
    }
    
    return nullptr;
}

// ============================================================================
// Counts and status
// ============================================================================

int ResourceStore::countByType(ResourceType type) const
{
    QReadLocker locker(&m_lock);
    return m_resources.value(type).size();
}

int ResourceStore::countByTypeAndTier(ResourceType type, ResourceTier tier) const
{
    QReadLocker locker(&m_lock);
    
    int count = 0;
    const auto& resources = m_resources.value(type);
    
    for (const auto& res : resources) {
        if (res.tier == tier) {
            ++count;
        }
    }
    
    return count;
}

int ResourceStore::totalCount() const
{
    QReadLocker locker(&m_lock);
    
    int total = 0;
    for (auto it = m_resources.constBegin(); it != m_resources.constEnd(); ++it) {
        total += it.value().size();
    }
    
    return total;
}

bool ResourceStore::isEmpty() const
{
    QReadLocker locker(&m_lock);
    
    for (auto it = m_resources.constBegin(); it != m_resources.constEnd(); ++it) {
        if (!it.value().isEmpty()) {
            return false;
        }
    }
    
    return true;
}

bool ResourceStore::hasType(ResourceType type) const
{
    QReadLocker locker(&m_lock);
    return !m_resources.value(type).isEmpty();
}

QVector<ResourceType> ResourceStore::availableTypes() const
{
    QReadLocker locker(&m_lock);
    
    QVector<ResourceType> result;
    
    for (auto it = m_resources.constBegin(); it != m_resources.constEnd(); ++it) {
        if (!it.value().isEmpty()) {
            result.append(it.key());
        }
    }
    
    return result;
}

QStringList ResourceStore::categoriesForType(ResourceType type) const
{
    QReadLocker locker(&m_lock);
    
    QSet<QString> categories;
    const auto& resources = m_resources.value(type);
    
    for (const auto& res : resources) {
        if (!res.category.isEmpty()) {
            categories.insert(res.category);
        }
    }
    
    return categories.values();
}

QStringList ResourceStore::locationsForType(ResourceType type) const
{
    QReadLocker locker(&m_lock);
    
    QSet<QString> locations;
    const auto& resources = m_resources.value(type);
    
    for (const auto& res : resources) {
        if (!res.locationKey.isEmpty()) {
            locations.insert(res.locationKey);
        }
    }
    
    return locations.values();
}

// ============================================================================
// Modification
// ============================================================================

void ResourceStore::clear()
{
    QWriteLocker locker(&m_lock);
    
    m_resources.clear();
    m_pathIndex.clear();
    
    locker.unlock();
    emit resourcesCleared(ResourceType::Unknown);
}

void ResourceStore::clearType(ResourceType type)
{
    QWriteLocker locker(&m_lock);
    
    // Remove paths from index
    const auto& resources = m_resources.value(type);
    for (const auto& res : resources) {
        m_pathIndex.remove(res.path);
    }
    
    m_resources.remove(type);
    
    locker.unlock();
    emit resourcesCleared(type);
}

void ResourceStore::clearTier(ResourceTier tier)
{
    QWriteLocker locker(&m_lock);
    
    for (auto it = m_resources.begin(); it != m_resources.end(); ++it) {
        auto& resources = it.value();
        
        // Remove from path index and vector
        for (int i = resources.size() - 1; i >= 0; --i) {
            if (resources[i].tier == tier) {
                m_pathIndex.remove(resources[i].path);
                resources.removeAt(i);
            }
        }
    }
}

bool ResourceStore::removeByPath(const QString& path)
{
    QWriteLocker locker(&m_lock);
    
    auto typeIt = m_pathIndex.constFind(path);
    if (typeIt == m_pathIndex.constEnd()) {
        return false;
    }
    
    ResourceType type = typeIt.value();
    auto& resources = m_resources[type];
    
    for (int i = 0; i < resources.size(); ++i) {
        if (resources[i].path == path) {
            resources.removeAt(i);
            m_pathIndex.remove(path);
            
            locker.unlock();
            emit resourceRemoved(path);
            return true;
        }
    }
    
    return false;
}

} // namespace resInventory
