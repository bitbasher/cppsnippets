/**
 * @file ShadersInventory.cpp
 * @brief Implementation of ShadersInventory
 */

#include "ShadersInventory.hpp"
#include "../resourceMetadata/ResourceTier.hpp"

#include <QFileInfo>
#include <QDebug>

namespace resourceInventory {

bool ShadersInventory::addShader(const QString& shaderPath, resourceMetadata::ResourceTier tier) {
    if (shaderPath.isEmpty()) {
        return false;
    }
    
    QFileInfo info(shaderPath);
    if (!info.exists() || !info.isFile()) {
        qWarning() << "ShadersInventory: Shader file does not exist:" << shaderPath;
        return false;
    }
    
    // Check valid extension
    QString ext = info.suffix().toLower();
    if (ext != "frag" && ext != "vert") {
        qWarning() << "ShadersInventory: Invalid shader extension:" << shaderPath;
        return false;
    }
    
    // Use absolute path as key (unique identifier)
    QString key = info.absoluteFilePath();
    
    // Skip if already added
    if (m_shaders.contains(key)) {
        return false;
    }
    
    // Create ResourceItem and store in QVariant
    ResourceItem shader(shaderPath);
    shader.setTier(tier);
    m_shaders.insert(key, QVariant::fromValue(shader));
    
    return true;
}

QList<QVariant> ShadersInventory::getAll() const {
    return m_shaders.values();
}

} // namespace resourceInventory
