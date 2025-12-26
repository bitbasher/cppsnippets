#pragma once

#include <QString>

#include "platformInfo/export.h"
#include "resourceInfo/resourceTier.h"

namespace platformInfo {

// Forward declaration
class ResourcePaths;

class RESOURCEMGMT_API ResourceLocation {
public:
    QString path;           ///< Expanded absolute path (for backward compatibility)
    QString displayName;
    QString description;
    resourceInfo::ResourceTier tier = resourceInfo::ResourceTier::Installation;
    bool isEnabled = true;
    bool exists = false;
    bool isWritable = false;
    bool hasResourceFolders = false;  // true if path contains resource folders (examples, fonts, libraries, locale)

    ResourceLocation() = default;
    ResourceLocation(const QString& p, const QString& name, const QString& desc = QString(),
                     resourceInfo::ResourceTier t = resourceInfo::ResourceTier::Installation)
        : path(p), displayName(name), description(desc), tier(t) {}
    
    /**
     * @brief Construct with template path containing env var placeholders
     * @param rawPath Template path with ${VAR} or %VAR% placeholders
     * @param name Display name
     * @param desc Description
     */
    ResourceLocation(const QString& rawPath, const QString& name, const QString& desc, bool isTemplate,
                     resourceInfo::ResourceTier t = resourceInfo::ResourceTier::Installation)
        : displayName(name), description(desc), tier(t), m_rawPath(isTemplate ? rawPath : QString())
    {
        if (!isTemplate) {
            path = rawPath;  // Direct path, no template
        }
    }
    
    // Copy constructor
    ResourceLocation(const ResourceLocation& other) :
        path(other.path),
        displayName(other.displayName),
        description(other.description),
        tier(other.tier),
        isEnabled(other.isEnabled),
        exists(other.exists),
        isWritable(other.isWritable),
        hasResourceFolders(other.hasResourceFolders),
        m_rawPath(other.m_rawPath) {}
    
    /**
     * @brief Get the raw path template (may contain env var placeholders)
     * @return Raw path string with ${VAR} or %VAR% placeholders, empty if not templated
     */
    QString rawPath() const { return m_rawPath; }
    
    /**
     * @brief Set the raw path template
     * @param rawPath Template string with env var placeholders
     */
    void setRawPath(const QString& rawPath) { m_rawPath = rawPath; }
    
    /**
     * @brief Get the expanded canonical absolute path
     * @param paths ResourcePaths instance for env var expansion
     * @return Canonical absolute path (resolved, no relative segments)
     * 
     * If m_rawPath is set, expands env vars and canonicalizes.
     * Otherwise returns the existing path member.
     */
    QString expandedPath(const ResourcePaths& paths) const;

    bool operator==(const ResourceLocation& other) const { return path == other.path; }

private:
    QString m_rawPath;  ///< Original template string with env var placeholders (empty if not templated)
};

} // namespace platformInfo
