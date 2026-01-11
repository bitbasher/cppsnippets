#pragma once
#include "export.hpp"
#include <QString>

// Forward declare ResourceTier from resourceInventory namespace
namespace resourceInventory {
    enum class ResourceTier;
}

namespace platformInfo {

// Import ResourceTier for use in this namespace
using resourceInventory::ResourceTier;

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
    ResourceLocation(const ResourceLocation& other);

    bool operator==(const ResourceLocation& other) const { return m_path == other.m_path; }

    // Getters
    QString path() const { return m_path; }
    QString rawPath() const { return m_rawPath; }
    QString displayName() const;
    QString description() const { return m_description; }
    ResourceTier tier() const { return m_tier; }
    bool isEnabled() const { return m_isEnabled; }
    bool exists() const { return m_exists; }
    bool isWritable() const { return m_isWritable; }
    bool hasResourceFolders() const { return m_hasResourceFolders; }

    // Setters
    void setPath(const QString& p);
    void setRawPath(const QString& raw) { m_rawPath = raw; }
    void setDisplayName(const QString& name);
    void setDescription(const QString& desc) { m_description = desc; }
    void setTier(ResourceTier tier) { m_tier = tier; }
    void setEnabled(bool enabled) { m_isEnabled = enabled; }
    void setExists(bool exist) { m_exists = exist; }
    void setWritable(bool writable) { m_isWritable = writable; }
    void setHasResourceFolders(bool has) { m_hasResourceFolders = has; }
    
    /**
     * @brief Generate display name from absolute path
     * 
     * Rules:
     * - Validates path is absolute and doesn't contain env var placeholders
     * - Short paths (< 24 chars): returned as-is
     * - Home directory: replaced with "~"
     * - Drive root: returned as-is (already minimal)
     * - Long paths: truncated to max length with ellipsis
     * 
     * @param absolutePath Absolute filesystem path (must not contain $, %, &&)
     * @return Display-friendly name
     */
    static QString generateDisplayName(const QString& absolutePath);
    
private:
    QString m_path;                     ///< Absolute resolved path to the resource location
    QString m_rawPath;                  ///< Original path with env vars (e.g., "$OPENSCAD_LIBRARIES")
    QString m_displayName;              ///< Display-friendly name
    QString m_description;              ///< User-friendly description
    ResourceTier m_tier;                ///< Tier: Installation, Machine, or User
    bool m_isEnabled = true;            ///< Whether this location is active
    bool m_exists = false;              ///< Whether the directory exists
    bool m_isWritable = false;          ///< Whether the directory is writable
    bool m_hasResourceFolders = false;  ///< Whether valid resource folders were found

    /**
     * @brief Get configured maximum display name length
     * @return Max length from settings (default 60 characters)
     */
    static int getMaxDisplayLength();
};

} // namespace platformInfo
