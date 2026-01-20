/**
 * @file TemplatesInventory.hpp
 * @brief Inventory storage for OpenSCAD template scripts
 * 
 * Stores templates in QHash<QString, QVariant> with location-based unique keys.
 * Templates are starting point scripts that users can customize.
 */

#pragma once

#include "../platformInfo/export.hpp"
#include "../platformInfo/ResourceLocation.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"
#include "resourceItem.hpp"

#include <QAbstractItemModel>
#include <QDirListing>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QVariant>

namespace resourceInventory {

/**
 * @brief Inventory for template scripts
 * 
 * Templates are .json files defining editor snippets.
 * Key format: "locationIndex-filename" for O(1) lookup (e.g., "1000-cube").
 */
class PLATFORMINFO_API TemplatesInventory : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit TemplatesInventory(QObject* parent = nullptr);
    ~TemplatesInventory() override = default;
    
    /**
     * @brief Add a template to inventory
     * 
     * The ResourceLocation provides tier, status, and index for uniqueID.
     * Template file must exist (already verified by scanner).
     * 
     * @param entry QDirEntry for the .json file
     * @param location ResourceLocation containing the template
     * @return true if added successfully, false if duplicate or invalid
     */
    bool addTemplate(const QDirListing::DirEntry& entry, 
                    const platformInfo::ResourceLocation& location);
    
    /**
     * @brief Add templates from a folder
     * 
     * Scans folder for .json template files.
     * 
     * @param folderPath Absolute path to templates folder
     * @param location ResourceLocation containing the templates folder
     * @return Number of templates added
     */
    int addFolder(const QString& folderPath, 
                  const platformInfo::ResourceLocation& location);

    /**
     * @brief Scan templates at a single resource location
     * @param location Resource location containing potential templates folder
     * @return Number of templates added
     */
    int scanLocation(const platformInfo::ResourceLocation& location);

    /**
     * @brief Scan templates across multiple locations
     * @param locations List of discovered resource locations
     * @return Total number of templates added
     */
    int scanLocations(const QList<platformInfo::ResourceLocation>& locations);
    
    /**
     * @brief Get template by unique key
     * @param key Unique key "locationIndex-filename" (e.g., "1000-cube")
     * @return QVariant containing ResourceTemplate, or invalid if not found
     */
    ResourceTemplate get(const QString& key) const { return m_templates.value(key, ResourceTemplate()); }
    
    /**
     * @brief Check if template exists by key
     * @param key Unique key "locationIndex-filename"
     * @return true if template in inventory
     */
    bool contains(const QString& key) const { return m_templates.contains(key); }
    
    /**
     * @brief Get all templates (for GUI list building)
     * @return List of all ResourceTemplate QVariants
     */
    QList<QVariant> getAll() const;
    
    /**
     * @brief Get count of templates
     * @return Number of templates in inventory
     */
    int count() const { return m_templates.size(); }
    
    /**
     * @brief Clear all templates
     */
    void clear();

    // QAbstractItemModel overrides
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    /**
     * @brief Get parsed JSON content for a template
     * 
     * Loads JSON from disk each time (no caching).
     * Use for editor snippet insertion.
     * 
     * @param key Unique key "locationIndex-filename"
     * @return QJsonObject with template definition (body, prefix, scopes)
     */
    QJsonObject getJsonContent(const QString& key) const;
    
    /**
     * @brief Write JSON content to disk
     * 
     * Uses atomic write (QSaveFile) to ensure file integrity.
     * 
     * @param key Unique key "locationIndex-filename"
     * @param json JSON content to write
     * @param errorMsg Output parameter for error message (empty on success)
     * @return true if write succeeded
     */
    bool writeJsonContent(const QString& key, const QJsonObject& json, QString& errorMsg);

private:
    /**
     * @brief Primary storage - indexed by unique key
     * 
     * Key: "locationIndex-filename" (e.g., "1000-cube", "1001-pyramid")
     * Value: QVariant containing ResourceTemplate
     */
    QHash<QString, ResourceTemplate> m_templates;
    QList<QString> m_keys; // stable ordering for model rows
    
    /**
     * @brief Parse JSON file from disk
     * @param filePath Absolute path to .json file
     * @return Parsed QJsonObject (empty on error)
     */
    QJsonObject parseJsonFile(const QString& filePath) const;
};

} // namespace resourceInventory
