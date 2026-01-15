/**
 * @file FontsInventory.cpp
 * @brief Implementation of FontsInventory
 */

#include "FontsInventory.hpp"
#include "../resourceMetadata/ResourceTier.hpp"

#include <QFileInfo>
#include <QDebug>

namespace resourceInventory {

bool FontsInventory::addFont(const QString& fontPath, resourceMetadata::ResourceTier tier) {
    if (fontPath.isEmpty()) {
        return false;
    }
    
    QFileInfo info(fontPath);
    if (!info.exists() || !info.isFile()) {
        qWarning() << "FontsInventory: Font file does not exist:" << fontPath;
        return false;
    }
    
    // Check valid extension
    QString ext = info.suffix().toLower();
    if (ext != "ttf" && ext != "otf") {
        qWarning() << "FontsInventory: Invalid font extension:" << fontPath;
        return false;
    }
    
    // Use absolute path as key (unique identifier)
    QString key = info.absoluteFilePath();
    
    // Skip if already added
    if (m_fonts.contains(key)) {
        return false;
    }
    
    // Create ResourceItem and store in QVariant
    ResourceItem font(fontPath);
    font.setTier(tier);
    m_fonts.insert(key, QVariant::fromValue(font));
    
    return true;
}

QList<QVariant> FontsInventory::getAll() const {
    return m_fonts.values();
}

} // namespace resourceInventory
