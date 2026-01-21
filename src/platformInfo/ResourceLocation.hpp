#pragma once
#include "export.hpp"
#include "../resourceMetadata/ResourceTier.hpp"
#include "../pathDiscovery/PathElement.hpp"
#include "../pathDiscovery/ResourcePaths.hpp"
#include <QString>
#include <QHash>
#include <QDirListing>  

namespace platformInfo {

// Use Gold Standard enum from resourceMetadata
using ResourceTier = resourceMetadata::ResourceTier;

/**
 * @brief Resource location with smart display name generation
 * 
 * Represents a filesystem location where resources may be found.
 * Display names are automatically generated from paths following these rules:
 * - Short paths (< 24 chars): Display full path
 * - Home directory paths: Replace with "~"  
 * - Drive roots (C:, /): Already minimal
 * - Long paths: Truncate to configured max (default 60 chars)
 * 
 * Tool tips always show the full path.
 */
class PLATFORMINFO_API ResourceLocation {
public:
    // Constructors
    ResourceLocation();
    ResourceLocation(const QString& resolvedPath, ResourceTier tier, const QString& rawPath = QString(), const QString& name = QString(), const QString& desc = QString());
    ResourceLocation(const pathDiscovery::PathElement discoveryPath );
    ResourceLocation(const ResourceLocation& other);

    bool operator==(const ResourceLocation& other) const { return m_path == other.m_path; }

    // Getters
    QString path() const { return m_path; }
    QString rawPath() const { return m_rawPath; }
    QString getDisplayName() const;
    QString description() const { return m_description; }
    ResourceTier tier() const { return m_tier; }

    // Setters
    void setPath(const QString& p) { m_path = p; }
    void setRawPath(const QString& raw) { m_rawPath = raw; }
    void setDescription(const QString& desc) { m_description = desc; }
    void setTier(ResourceTier tier) { m_tier = tier; }
    
    // validators
    static bool locationHasResource(const pathDiscovery::PathElement& possibleLoc) {
        using ItFlag = QDirListing::IteratorFlag;
        QDirListing location(possibleLoc.path(), resourceMetadata::s_allResourceFolders, ItFlag::DirsOnly);
        return (location.cbegin() != location.cend());
    }

private:
    QString m_path;                     ///< Absolute resolved path to the resource location
    QString m_rawPath;                  ///< Original path with env vars (e.g., "$OPENSCAD_LIBRARIES")
    QString m_description;              ///< User-friendly description
    ResourceTier m_tier;                ///< Tier: Installation, Machine, or User

    /**
     * @brief Get configured maximum display name length
     * @return Max length from settings (default 60 characters)
     */
    static int getMaxDisplayLength();
};

} // namespace platformInfo
