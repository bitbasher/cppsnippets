#pragma once
#include <QString>
#include "platformInfo/export.h"

namespace platformInfo {

/**
 * @brief Resource location tier/level
 * 
 * Resources are organized into three tiers based on their scope and accessibility:
 * - Installation: Built-in resources from the application installation (read-only)
 * - Machine: System-wide resources available to all users (admin-managed)
 * - User: Personal resources in the user's home directory (user-managed)
 */
enum class ResourceTier {
    Installation,   ///< Built-in application resources (read-only, hardcoded)
    Machine,        ///< System-wide resources for all users (admin-managed)
    User            ///< Per-user resources (user-managed)
};

// Forward declaration
class ResourcePaths;

class RESOURCEMGMT_API ResourceLocation {
public:
    QString path;           ///< Expanded absolute path (for backward compatibility)
    QString displayName;
    QString description;
    bool isEnabled = true;
    bool exists = false;
    bool isWritable = false;
    bool hasResourceFolders = false;  // true if path contains resource folders (examples, fonts, libraries, locale)

    ResourceLocation() = default;
    ResourceLocation(const QString& p, const QString& name, const QString& desc = QString())
        : path(p), displayName(name), description(desc) {}
    
    /**
     * @brief Construct with template path containing env var placeholders
     * @param rawPath Template path with ${VAR} or %VAR% placeholders
     * @param name Display name
     * @param desc Description
     */
    ResourceLocation(const QString& rawPath, const QString& name, const QString& desc, bool isTemplate)
        : displayName(name), description(desc), m_rawPath(isTemplate ? rawPath : QString())
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
