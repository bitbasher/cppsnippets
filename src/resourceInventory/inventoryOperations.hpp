/**
 * @file inventoryOperations.hpp
 * @brief Free functions for inventory operations
 * 
 * Provides high-level operations for adding/loading templates
 * into QStandardItemModel inventory.
 */

#pragma once

#include "../platformInfo/export.hpp"
#include <QString>

class QStandardItemModel;
class QFileInfo;

namespace resourceInventory {
class TemplatesInventory;
}

namespace resourceInventory {

class ResourceItem;
class ResourceTemplate;

/**
 * @brief Add a single template from a JSON file to the inventory
 * @param inventory The model to add the template to
 * @param fileInfo File info for the JSON template file
 * @param errorMessage Optional output parameter for error message
 * @return True if successful, false otherwise
 */
PLATFORMINFO_API bool addTemplateFromFile(QStandardItemModel* inventory, 
                                          const QFileInfo& fileInfo,
                                          QString* errorMessage = nullptr);

/**
 * @brief Add a single template from a JSON file to the inventory (convenience overload)
 * @param inventory The model to add the template to
 * @param filePath Path to the JSON template file
 * @param errorMessage Optional output parameter for error message
 * @return True if successful, false otherwise
 */
PLATFORMINFO_API bool addTemplateFromFile(QStandardItemModel* inventory, 
                                          const QString& filePath,
                                          QString* errorMessage = nullptr);

PLATFORMINFO_API bool addTemplateFromFile(resourceInventory::TemplatesInventory* inventory,
                                          const QFileInfo& fileInfo,
                                          QString* errorMessage = nullptr);

PLATFORMINFO_API bool addTemplateFromFile(resourceInventory::TemplatesInventory* inventory,
                                          const QString& filePath,
                                          QString* errorMessage = nullptr);

/**
 * @brief Load multiple templates from a JSON file into the inventory
 * @param inventory The model to add the templates to
 * @param fileInfo File info for the JSON templates file
 * @param errorMessage Optional output parameter for error message
 * @return True if successful, false otherwise
 */
PLATFORMINFO_API bool loadTemplatesFromFile(QStandardItemModel* inventory, 
                                            const QFileInfo& fileInfo,
                                            QString* errorMessage = nullptr);

/**
 * @brief Load multiple templates from a JSON file into the inventory (convenience overload)
 * @param inventory The model to add the templates to
 * @param filePath Path to the JSON templates file
 * @param errorMessage Optional output parameter for error message
 * @return True if successful, false otherwise
 */
PLATFORMINFO_API bool loadTemplatesFromFile(QStandardItemModel* inventory, 
                                            const QString& filePath,
                                            QString* errorMessage = nullptr);

PLATFORMINFO_API bool loadTemplatesFromFile(resourceInventory::TemplatesInventory* inventory,
                                            const QFileInfo& fileInfo,
                                            QString* errorMessage = nullptr);

PLATFORMINFO_API bool loadTemplatesFromFile(resourceInventory::TemplatesInventory* inventory,
                                            const QString& filePath,
                                            QString* errorMessage = nullptr);

/**
 * @brief Save all templates from the inventory to a JSON file
 * @param inventory The model containing templates to save
 * @param fileInfo File info for the output JSON file
 * @param errorMessage Optional output parameter for error message
 * @return True if successful, false otherwise
 */
PLATFORMINFO_API bool saveTemplatesToFile(const QStandardItemModel* inventory, 
                                          const QFileInfo& fileInfo,
                                          QString* errorMessage = nullptr);

/**
 * @brief Save all templates from the inventory to a JSON file (convenience overload)
 * @param inventory The model containing templates to save
 * @param filePath Path to the output JSON file
 * @param errorMessage Optional output parameter for error message
 * @return True if successful, false otherwise
 */
PLATFORMINFO_API bool saveTemplatesToFile(const QStandardItemModel* inventory, 
                                          const QString& filePath,
                                          QString* errorMessage = nullptr);

PLATFORMINFO_API bool saveTemplatesToFile(const resourceInventory::TemplatesInventory* inventory,
                                          const QFileInfo& fileInfo,
                                          QString* errorMessage = nullptr);

PLATFORMINFO_API bool saveTemplatesToFile(const resourceInventory::TemplatesInventory* inventory,
                                          const QString& filePath,
                                          QString* errorMessage = nullptr);

/**
 * @brief Add a ResourceItem to the inventory model
 * @param inventory The model to add the item to
 * @param item The resource item to add
 * @return True if successful, false otherwise
 */
PLATFORMINFO_API bool addItemToInventory(QStandardItemModel* inventory, const ResourceItem& item);

} // namespace resourceInventory
