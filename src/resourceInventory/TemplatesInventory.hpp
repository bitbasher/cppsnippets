/**
 * @file TemplatesInventory.hpp
 * @brief Inventory storage for OpenSCAD template scripts
 * 
 * Stores templates in QHash<QString, QVariant> with hierarchical keys.
 * Templates are starting point scripts that users can customize.
 * May have associated .json metadata files for parameter descriptions.
 */

#pragma once

#include "../platformInfo/export.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"
#include "resourceItem.hpp"

#include <QHash>
#include <QString>
#include <QVariant>
#include <QList>
#include <QDirListing>
#include <QJsonObject>

namespace resourceInventory {

/**
 * @brief Inventory for template scripts with JSON content caching
 * 
 * Templates are .json files defining editor snippets.
 * Key format: "tier-name" for O(1) lookup.
 * JSON content is cached in memory for fast editor insertion.
 */
class PLATFORMINFO_API TemplatesInventory {
public:
    TemplatesInventory() = default;
    ~TemplatesInventory() = default;
    
    /**
     * @brief Add a template to inventory
     * 
     * Templates are .json files containing template definitions.
     * Key format: "tier-name"
     * 
     * @param entry QDirEntry for the .json file
     * @param tier Resource tier (installation/machine/user)
     * @return true if added successfully, false if already exists or invalid
     */
    bool addTemplate(const QDirListing::DirEntry& entry, const QString& tier);
    
    /**
     * @brief Add templates from a folder
     * 
     * Scans folder for .json template files.
     * 
     * @param entry QDirEntry for the folder
     * @param tier Resource tier (installation/machine/user)
     * @return Number of templates added
     */
    int addFolder(const QDirListing::DirEntry& entry, const QString& tier);
    
    /**
     * @brief Get template by hierarchical key
     * @param key Hierarchical key "tier-name"
     * @return QVariant containing ResourceScript, or invalid QVariant if not found
     */
    QVariant get(const QString& key) const;
    
    /**
     * @brief Get template by file path (slower, searches all entries)
     * @param path Absolute file path
     * @return QVariant containing ResourceItem, or invalid QVariant if not found
     */
    QVariant getByPath(const QString& path) const;
    
    /**
     * @brief Check if template exists by key
     * @param key Hierarchical key "tier-name"
     * @return true if template in inventory
     */
    bool contains(const QString& key) const;
    
    /**
     * @brief Get all templates (for GUI list building)
     * @return Ephemeral list of all ResourceItem QVariants
     */
    QList<QVariant> getAll() const;
    
    /**
     * @brief Get count of templates
     * @return Number of templates in inventory
     */
    int count() const { return m_templates.size(); }
    
    /**
     * @brief Clear all templates and JSON cache
     */
    void clear() { 
        m_templates.clear(); 
        m_jsonCache.clear();
    }
    
    // ========================================================================
    // JSON Content Access (for editor snippet insertion)
    // ========================================================================
    
    /**
     * @brief Get parsed JSON content for a template
     * 
     * Loads and caches JSON on first access. Returns cached version on
     * subsequent calls. Use for editor snippet insertion.
     * 
     * @param key Hierarchical key "tier-name"
     * @return QJsonObject with template definition (body, prefix, scopes)
     */
    QJsonObject getJsonContent(const QString& key) const;
    
    /**
     * @brief Check if JSON content is already cached
     * @param key Hierarchical key
     * @return true if JSON is in cache
     */
    bool hasJsonContent(const QString& key) const { return m_jsonCache.contains(key); }
    
    /**
     * @brief Preload JSON content for a template
     * 
     * Useful for warming the cache. addTemplate() loads JSON automatically,
     * so this is optional.
     * 
     * @param key Hierarchical key
     * @return true if JSON loaded successfully
     */
    bool loadJsonContent(const QString& key);
    
    /**
     * @brief Clear JSON cache to free memory
     * 
     * Metadata (ResourceItem) remains. JSON will be reloaded on next access.
     */
    void clearJsonCache() { m_jsonCache.clear(); }
    
    /**
     * @brief Get size of JSON cache
     * @return Number of cached JSON objects
     */
    int jsonCacheSize() const { return m_jsonCache.size(); }
    
    // ========================================================================
    // JSON Content Writing (for template updates)
    // ========================================================================
    
    /**
     * @brief Write JSON content to disk
     * 
     * Uses atomic write (QSaveFile) to ensure file integrity.
     * Updates cache if write succeeds.
     * 
     * @param key Hierarchical key \"tier-name\"
     * @param json JSON content to write
     * @param errorMsg Output parameter for error message (empty on success)
     * @return true if write succeeded
     */
    bool writeJsonContent(const QString& key, const QJsonObject& json, QString& errorMsg);

private:
    /**
     * @brief Primary storage - indexed by hierarchical key
     * 
     * Key: "tier-name" (e.g., "installation-cube")
     * Value: QVariant containing ResourceItem
     */
    QHash<QString, QVariant> m_templates;
    
    /**
     * @brief JSON content cache for fast editor access
     * 
     * Key: "tier-name" (same as m_templates)
     * Value: Parsed QJsonObject with template definition
     * 
     * Loaded lazily on first getJsonContent() call or eagerly in addTemplate().
     * Use clearJsonCache() to free memory if needed.
     */
    mutable QHash<QString, QJsonObject> m_jsonCache;
    
    /**
     * @brief Parse JSON file from disk
     * @param filePath Absolute path to .json file
     * @return Parsed QJsonObject (empty on error)
     */
    QJsonObject parseJsonFile(const QString& filePath) const;
};

} // namespace resourceInventory
