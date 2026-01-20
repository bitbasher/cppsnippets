/**
 * @file ResourceIndexer.hpp
 * @brief Global resource index generator for all resource types
 * 
 * Provides unique sequential indices across templates, examples, fonts, etc.
 * Prevents duplicate keys when same basename appears in different resource types.
 */

#pragma once

#include "../platformInfo/export.hpp"
#include "../platformInfo/ResourceLocation.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"

#include <QString>
#include <QHash>

namespace resourceInventory {

using ResourceType = resourceMetadata::ResourceType;

/**
 * @brief Global resource index generator
 * 
 * Generates unique sequential indices (1000, 1001, 1002, ...) for ALL resources.
 * Simple incrementing counter - no caching or registry needed.
 */
class PLATFORMINFO_API ResourceIndexer {
public:
    // Prevent instantiation - this is a pure static utility class
    ResourceIndexer() = delete;
    ResourceIndexer(const ResourceIndexer& other) = delete;
    ResourceIndexer& operator=(const ResourceIndexer& other) = delete;

    /**
     * @brief Get next unique ID string for a resource
     * @param postfix String to append after the index (e.g., basename)
     * @return Unique ID string in format "1000-postfix"
     */
    static QString getUniqueIDString( const QString& postfix )
    {
        return QString("%1-%2").arg(++s_index, 4, 10, QChar('0')).arg(postfix);
    }
private:
    static inline int s_index = 999;    ///< Global unique ID counter
};

} // namespace resourceInventory
