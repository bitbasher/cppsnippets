/**
 * @file ResourceIndexer.cpp
 * @brief Implementation of unified resource indexing
 */

#include "ResourceIndexer.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"

#include <QMutexLocker>

namespace resourceInventory {

// Static member initialization
int ResourceIndexer::s_nextIndex = 1000;
QHash<QString, QString> ResourceIndexer::s_keyToIndex;
QHash<QString, QString> ResourceIndexer::s_indexToKey;
QMutex ResourceIndexer::s_mutex;

QString ResourceIndexer::getOrCreateIndex(
    const platformInfo::ResourceLocation& location,
    resourceMetadata::ResourceType type,
    const QString& baseName
)
{
    QMutexLocker locker(&s_mutex);
    
    // Build composite key: "{locationIndex}|{resourceType}|{baseName}"
    QString key = buildKey(location.getIndexString(), type, baseName);
    
    // Check if already registered
    if (s_keyToIndex.contains(key)) {
        return s_keyToIndex.value(key);
    }
    
    // Generate new index
    QString indexString = QString::number(s_nextIndex).rightJustified(4, '0');
    s_nextIndex++;
    
    // Register bidirectional mapping
    s_keyToIndex.insert(key, indexString);
    s_indexToKey.insert(indexString, key);
    
    return indexString;
}

QString ResourceIndexer::getKey(const QString& index)
{
    QMutexLocker locker(&s_mutex);
    return s_indexToKey.value(index, QString());
}

QString ResourceIndexer::extractBaseName(const QString& key)
{
    // Key format: "{locationIndex}|{resourceType}|{baseName}"
    QStringList parts = key.split('|');
    if (parts.size() == 3) {
        return parts[2];
    }
    return QString();
}

int ResourceIndexer::count()
{
    QMutexLocker locker(&s_mutex);
    return s_keyToIndex.size();
}

void ResourceIndexer::clear()
{
    QMutexLocker locker(&s_mutex);
    s_nextIndex = 1000;
    s_keyToIndex.clear();
    s_indexToKey.clear();
}

QString ResourceIndexer::buildKey(
    const QString& locationIndex,
    resourceMetadata::ResourceType type,
    const QString& baseName
)
{
    QString typeName = resourceMetadata::ResourceTypeInfo::getResTypeString(type);
    return QString("%1|%2|%3").arg(locationIndex, typeName, baseName);
}

} // namespace resourceInventory
