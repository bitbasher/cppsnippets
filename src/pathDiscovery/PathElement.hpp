/**
 * @file PathElement.hpp
 * @brief Represents a single search path with its tier classification
 */

#pragma once

#include "export.hpp"
#include "resourceMetadata/ResourceTier.hpp"

#include <QString>

namespace pathDiscovery {

/**
 * @brief Represents a single search path with its tier
 * 
 * Used to track search paths through expansion, qualification, and discovery.
 * Access permissions are determined by tier via resourceMetadata::accessByTier map.
 */
class PathElement {
public:
    PathElement(resourceMetadata::ResourceTier tier, const QString& path)
        : m_tier(tier), m_path(path) {}
    
    resourceMetadata::ResourceTier tier() const { return m_tier; }
    QString path() const { return m_path; }
    
private:
    resourceMetadata::ResourceTier m_tier;
    QString m_path;
};

} // namespace pathDiscovery
