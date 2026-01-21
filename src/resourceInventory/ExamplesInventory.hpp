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

#include <QAbstractItemModel>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QList>
#include <QDirListing>

namespace resourceInventory {

/**
 * @brief Tree model inventory for example scripts with category grouping
 * 
 * Implements QAbstractItemModel for QTreeView display.
 * Tree structure:
 * - Root level: Categories (folders) + loose files (no category)
 * - Child level: Example files within categories
 * 
 * Storage: QHash for O(1) lookup by unique ID
 * Categories merged across all tiers (Installation, Machine, User)
 */
class PLATFORMINFO_API ExamplesInventory : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit ExamplesInventory(QObject* parent = nullptr);
    ~ExamplesInventory() override = default;
    
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
    void clear();

    // QAbstractItemModel overrides
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

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
     * @brief Primary storage - indexed by unique ID
     * 
     * Key: uniqueID from ResourceIndexer (e.g., "0", "1", "2")
     * Value: ResourceScript object
     */
    QHash<QString, ResourceScript> m_scripts;
    
    /**
     * @brief Category-to-IDs mapping for tree structure
     * 
     * Key: Category name (empty QString for loose files)
     * Value: List of script unique IDs in that category
     */
    QHash<QString, QList<QString>> m_categoryToIds;
    
    /**
     * @brief Stable ordering of category names for tree rows
     * Categories sorted alphabetically, empty category (loose files) appears first
     */
    QStringList m_categoryKeys;
    
    /**
     * @brief Helper: Get internal row number for a category
     * @param category Category name
     * @return Row index in m_categoryKeys, or -1 if not found
     */
    int categoryRow(const QString& category) const;
    
    /**
     * @brief Helper: Get internal row number for a script within category
     * @param uniqueID Script's unique ID
     * @param category Category name
     * @return Row index in m_categoryToIds[category], or -1 if not found
     */
    int scriptRowInCategory(const QString& uniqueID, const QString& category) const;
};

} // namespace resourceInventory
