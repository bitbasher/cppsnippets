/**
 * @file TranslationsInventory.hpp
 * @brief Translation resources inventory using QVariant storage
 * 
 * Simple file-based resources (no metadata, no attachments)
 * Translation files: .qm, .ts
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
 * @brief Inventory for translation resources
 * 
 * Stores translations as ResourceItem in QVariant containers.
 * Key: Absolute file path (unique identifier)
 */
class PLATFORMINFO_API TranslationsInventory {
public:
    TranslationsInventory() = default;
    ~TranslationsInventory() = default;
    
    /**
     * @brief Add a translation to inventory
     * @param translationPath Absolute path to .qm or .ts file
     * @param tier Resource tier (Installation, Machine, User)
     * @return true if added, false if already exists or invalid
     */
    bool addTranslation(const QString& translationPath, resourceMetadata::ResourceTier tier);
    
    /**
     * @brief Get all translations as QVariant list
     * @return List of QVariants containing ResourceItem objects
     */
    QList<QVariant> getAll() const;
    
    /**
     * @brief Get total count of translations
     * @return Number of translations in inventory
     */
    int count() const { return m_translations.size(); }
    
    /**
     * @brief Clear all translations from inventory
     */
    void clear() { m_translations.clear(); }
    
private:
    // Translation storage: Key = absolute path, Value = QVariant(ResourceItem)
    QHash<QString, QVariant> m_translations;
};

} // namespace resourceInventory
