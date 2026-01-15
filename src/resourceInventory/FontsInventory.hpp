/**
 * @file FontsInventory.hpp
 * @brief Font resources inventory using QVariant storage
 * 
 * Simple file-based resources (no metadata, no attachments)
 * Font files: .ttf, .otf
 */

#pragma once

#include "../platformInfo/export.hpp"
#include "resourceItem.hpp"

#include <QHash>
#include <QString>
#include <QVariant>
#include <QList>

namespace resourceInventory {

/**
 * @brief Inventory for font resources
 * 
 * Stores fonts as ResourceItem in QVariant containers.
 * Key: Absolute file path (unique identifier)
 */
class PLATFORMINFO_API FontsInventory {
public:
    FontsInventory() = default;
    ~FontsInventory() = default;
    
    /**
     * @brief Add a font to inventory
     * @param fontPath Absolute path to .ttf or .otf file
     * @param tier Resource tier (Installation, Machine, User)
     * @return true if added, false if already exists or invalid
     */
    bool addFont(const QString& fontPath, resourceMetadata::ResourceTier tier);
    
    /**
     * @brief Get all fonts as QVariant list
     * @return List of QVariants containing ResourceItem objects
     */
    QList<QVariant> getAll() const;
    
    /**
     * @brief Get total count of fonts
     * @return Number of fonts in inventory
     */
    int count() const { return m_fonts.size(); }
    
    /**
     * @brief Clear all fonts from inventory
     */
    void clear() { m_fonts.clear(); }
    
private:
    // Font storage: Key = absolute path, Value = QVariant(ResourceItem)
    QHash<QString, QVariant> m_fonts;
};

} // namespace resourceInventory
