/**
 * @file ResourceIndexer.hpp
 * @brief Unified resource indexing across all resource types
 * 
 * Provides single source of truth for resource indices across templates,
 * examples, fonts, tests, libraries, shaders, translations, etc.
 * 
 * Handles name collisions: "cube.json" template vs "cube.scad" example
 * get different indices even if they're in the same location.
 */

#pragma once

#include "../platformInfo/export.hpp"
#include "../platformInfo/ResourceLocation.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"

#include <QString>
#include <QHash>
#include <QMutex>

namespace resourceInventory {

// Use ResourceType from resourceMetadata namespace
using ResourceType = resourceMetadata::ResourceType;

/**
 * @brief Unified resource index generator for all resource types
 * 
 * Generates unique sequential indices (1000, 1001, 1002, ...) for resources
 * across ALL resource types. Ensures that:
 * - Same basename in different types gets different indices
 * - Same basename in different locations gets different indices
 * - Thread-safe for concurrent scanning
 * 
 * Example:
 * - Installation/templates/cube.json → "1000"
 * - Installation/examples/cube.scad → "1001"
 * - User/templates/cube.json → "1002"
 * 
 * Composite key format: "{locationIndex}|{resourceType}|{baseName}"
 */
class PLATFORMINFO_API ResourceIndexer {
public:
    /**
     * @brief Get or create unique index for a resource
     * 
     * Generates compound key from location index, resource type, and basename.
     * Returns existing index if already registered, otherwise creates new one.
     * 
     * Thread-safe for concurrent access during scanning.
     * 
     * @param location Resource location containing tier and path
     * @param type Resource type (Templates, Examples, Fonts, etc.)
     * @param baseName Base name without extension (e.g., "cube" not "cube.json")
     * @return Unique index string (e.g., "1000", "1001")
     */
    static QString getOrCreateIndex(
        const platformInfo::ResourceLocation& location,
        resourceMetadata::ResourceType type,
        const QString& baseName
    );
    
    /**
     * @brief Get composite key for an existing index
     * @param index Index string (e.g., "1000")
     * @return Composite key or empty if not found
     */
    static QString getKey(const QString& index);
    
    /**
     * @brief Extract basename from composite key
     * @param key Composite key "{locationIndex}|{type}|{baseName}"
     * @return Base name or empty if malformed
     */
    static QString extractBaseName(const QString& key);
    
    /**
     * @brief Get total count of registered resources across all types
     * @return Number of unique resource indices issued
     */
    static int count();
    
    /**
     * @brief Clear all registered indices (for testing)
     * 
     * Resets counter to 1000 and clears all mappings.
     * Use with caution - only for test teardown.
     */
    static void clear();
    
private:
    // Static storage (shared across all ResourceIndexer uses)
    static int s_nextIndex;                          ///< Next index to issue (starts at 1000)
    static QHash<QString, QString> s_keyToIndex;     ///< Composite key → index string
    static QHash<QString, QString> s_indexToKey;     ///< Index string → composite key
    static QMutex s_mutex;                           ///< Thread safety for concurrent scanning
    
    /**
     * @brief Build composite key from components
     * @param locationIndex Location index from ResourceLocation::getIndexString()
     * @param type Resource type enum
     * @param baseName Base name without extension
     * @return Composite key "{locationIndex}|{resourceType}|{baseName}"
     */
    static QString buildKey(
        const QString& locationIndex,
        resourceMetadata::ResourceType type,
        const QString& baseName
    );
};

} // namespace resourceInventory
