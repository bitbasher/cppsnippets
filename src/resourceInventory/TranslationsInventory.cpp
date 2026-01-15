/**
 * @file TranslationsInventory.cpp
 * @brief Implementation of TranslationsInventory
 */

#include "TranslationsInventory.hpp"
#include "../resourceMetadata/ResourceTier.hpp"

#include <QFileInfo>
#include <QDebug>

namespace resourceInventory {

bool TranslationsInventory::addTranslation(const QString& translationPath, resourceMetadata::ResourceTier tier) {
    if (translationPath.isEmpty()) {
        return false;
    }
    
    QFileInfo info(translationPath);
    if (!info.exists() || !info.isFile()) {
        qWarning() << "TranslationsInventory: Translation file does not exist:" << translationPath;
        return false;
    }
    
    // Check valid extension
    QString ext = info.suffix().toLower();
    if (ext != "qm" && ext != "ts") {
        qWarning() << "TranslationsInventory: Invalid translation extension:" << translationPath;
        return false;
    }
    
    // Use absolute path as key (unique identifier)
    QString key = info.absoluteFilePath();
    
    // Skip if already added
    if (m_translations.contains(key)) {
        return false;
    }
    
    // Create ResourceItem and store in QVariant
    ResourceItem translation(translationPath);
    translation.setTier(tier);
    m_translations.insert(key, QVariant::fromValue(translation));
    
    return true;
}

QList<QVariant> TranslationsInventory::getAll() const {
    return m_translations.values();
}

} // namespace resourceInventory
