/**
 * @file ShadersInventory.hpp
 * @brief Shader resources inventory using QVariant storage
 * 
 * Simple file-based resources (no metadata, no attachments)
 * Shader files: .frag, .vert
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
 * @brief Inventory for shader resources
 * 
 * Stores shaders as ResourceItem in QVariant containers.
 * Key: Absolute file path (unique identifier)
 */
class PLATFORMINFO_API ShadersInventory {
public:
    ShadersInventory() = default;
    ~ShadersInventory() = default;
    
    /**
     * @brief Add a shader to inventory
     * @param shaderPath Absolute path to .frag or .vert file
     * @param tier Resource tier (Installation, Machine, User)
     * @return true if added, false if already exists or invalid
     */
    bool addShader(const QString& shaderPath, resourceMetadata::ResourceTier tier);
    
    /**
     * @brief Get all shaders as QVariant list
     * @return List of QVariants containing ResourceItem objects
     */
    QList<QVariant> getAll() const;
    
    /**
     * @brief Get total count of shaders
     * @return Number of shaders in inventory
     */
    int count() const { return m_shaders.size(); }
    
    /**
     * @brief Clear all shaders from inventory
     */
    void clear() { m_shaders.clear(); }
    
private:
    // Shader storage: Key = absolute path, Value = QVariant(ResourceItem)
    QHash<QString, QVariant> m_shaders;
};

} // namespace resourceInventory
