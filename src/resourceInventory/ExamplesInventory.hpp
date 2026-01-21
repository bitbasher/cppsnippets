/**
 * @file ExamplesInventory.hpp
 * @brief Inventory storage for OpenSCAD example scripts
 * 
 * Stores examples in QHash<QString, QVariant> for O(1) lookup by path.
 * Handles attachment detection and category organization.
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

namespace resourceInventory {

/**
 * @brief Inventory for example scripts with attachment detection
 * 
 * Stores ResourceScript objects in QHash using file path as key.
 * Provides O(1) lookup and filtered list generation for GUI.
 */
class PLATFORMINFO_API ExamplesInventory {
public:
    ExamplesInventory() = default;
    ~ExamplesInventory() = default;
    
    /**
     * @brief Add an example script to inventory
     * 
     * The ResourceLocation provides the location index and tier.
     * Scans for attachments (same baseName, different extensions).
     * 
     * @param entry QDirEntry for the .scad file
     * @param location ResourceLocation containing the example
     * @param category Category/group name (or "uncategorized")
     * @return true if added successfully, false if duplicate or invalid
     */
    bool addExample(const QDirListing::DirEntry& entry, 
                   const platformInfo::ResourceLocation& location,
                   const QString& category = QStringLiteral("uncategorized"));
    
    /**
     * @brief Add a category folder (auto-detect category from basename)
     * 
     * Scans folder for .scad files. If found, treats as category.
     * If no .scad files, ignores (empty group folder).
     * Category name is extracted from the folder's basename.
     * If basename matches resource type name, treats as uncategorized root folder.
     * 
     * @param dirEntry QDirEntry for the folder to scan
     * @param location ResourceLocation containing the examples folder
     * @return Number of examples added
     */
    int addFolder(const QDirListing::DirEntry& dirEntry,
                  const platformInfo::ResourceLocation& location);
    
    /**
     * @brief Get example by hierarchical key
     * @param key Hierarchical key "tier-category-name"
     * @return QVariant containing ResourceScript, or invalid QVariant if not found
     */
    QVariant get(const QString& key) const;
    
    /**
     * @brief Check if example exists by key
     * @param key Hierarchical key "tier-category-name"
     * @return true if example in inventory
     */
    bool contains(const QString& key) const;
    
    /**
     * @brief Get all examples (for GUI list building)
     * @return Ephemeral list of all ResourceScript QVariants
     */
    QList<QVariant> getAll() const;
    
    /**
     * @brief Get examples by category (filtered)
     * @param category Category/group name
     * @return Ephemeral list of matching ResourceScript QVariants
     */
    QList<QVariant> getByCategory(const QString& category) const;
    
    /**
     * @brief Get all unique categories
     * @return List of category names found in inventory
     */
    QStringList getCategories() const;
    
    /**
     * @brief Get count of examples
     * @return Number of examples in inventory
     */
    int count() const { return m_scripts.size(); }
    
    /**
     * @brief Clear all examples
     */
    void clear() { m_scripts.clear(); }

private:
    /**
     * @brief Scan for attachments for a script file
     * 
     * Looks for files with same baseName but different extensions.
     * Uses s_attachments list from ResourceTypeInfo for allowed extensions.
     * 
     * @param scriptPath Path to .scad file
     * @return List of attachment file paths
     */
    QStringList scanAttachments(const QString& scriptPath) const;
    
    /**
     * @brief Primary storage - indexed by file path
     * 
     * Key: Absolute file path to .scad file
     * Value: QVariant containing ResourceScript
     */
    QHash<QString, QVariant> m_scripts;
};

} // namespace resourceInventory
