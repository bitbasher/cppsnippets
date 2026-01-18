/**
 * @file inventoryOperations.cpp
 * @brief Implementation of inventory operation functions
 */

#include "inventoryOperations.hpp"
#include "resourceItem.hpp"
#include "../scadtemplates/template_parser.hpp"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileInfo>
#include <QFile>

namespace resourceInventory {

bool addItemToInventory(QStandardItemModel* inventory, const ResourceItem& item)
{
    // Create display item for the model
    QStandardItem* displayItem = new QStandardItem(item.name());
    displayItem->setData(item.description(), Qt::ToolTipRole);
    
    // Store full ResourceItem in UserRole for later retrieval
    QVariant itemData = QVariant::fromValue(item);
    displayItem->setData(itemData, Qt::UserRole);
    
    // Add to model
    inventory->appendRow(displayItem);
    
    return true;
}

bool addTemplateFromFile(QStandardItemModel* inventory, 
                         const QString& filePath,
                         QString* errorMessage)
{
    return addTemplateFromFile(inventory, QFileInfo(filePath), errorMessage);
}

bool addTemplateFromFile(QStandardItemModel* inventory, 
                         const QFileInfo& fileInfo,
                         QString* errorMessage)
{
    if (!inventory) {
        if (errorMessage) *errorMessage = QStringLiteral("Invalid inventory model");
        return false;
    }
    
    if (!fileInfo.exists()) {
        if (errorMessage) *errorMessage = QStringLiteral("File does not exist: %1").arg(fileInfo.filePath());
        return false;
    }
    
    // Load template using instance method
    ResourceTemplate tmpl;
    if (!tmpl.loadFromJson(fileInfo)) {
        if (errorMessage) *errorMessage = tmpl.lastError();
        return false;
    }
    
    // Add to inventory
    return addItemToInventory(inventory, tmpl);
}

bool loadTemplatesFromFile(QStandardItemModel* inventory, 
                           const QString& filePath,
                           QString* errorMessage)
{
    return loadTemplatesFromFile(inventory, QFileInfo(filePath), errorMessage);
}

bool loadTemplatesFromFile(QStandardItemModel* inventory, 
                           const QFileInfo& fileInfo,
                           QString* errorMessage)
{
    if (!inventory) {
        if (errorMessage) *errorMessage = QStringLiteral("Invalid inventory model");
        return false;
    }
    
    if (!fileInfo.exists()) {
        if (errorMessage) *errorMessage = QStringLiteral("File does not exist: %1").arg(fileInfo.filePath());
        return false;
    }
    
    // Parse all templates from file
    scadtemplates::TemplateParser parser;
    auto result = parser.parseFile(fileInfo.filePath());
    
    if (!result.success) {
        if (errorMessage) *errorMessage = result.errorMessage;
        return false;
    }
    
    if (result.templates.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("No templates found in file");
        return false;
    }
    
    // Add each template to inventory
    for (const auto& tmpl : result.templates) {
        // Set file metadata
        ResourceTemplate item = tmpl;
        item.setPath(fileInfo.filePath());
        item.setSourcePath(fileInfo.filePath());
        item.setLastModified(fileInfo.lastModified());
        
        if (!addItemToInventory(inventory, item)) {
            if (errorMessage) *errorMessage = QStringLiteral("Failed to add template: %1").arg(tmpl.name());
            return false;
        }
    }
    
    return true;
}

bool saveTemplatesToFile(const QStandardItemModel* inventory, 
                        const QString& filePath,
                        QString* errorMessage)
{
    return saveTemplatesToFile(inventory, QFileInfo(filePath), errorMessage);
}

bool saveTemplatesToFile(const QStandardItemModel* inventory, 
                        const QFileInfo& fileInfo,
                        QString* errorMessage)
{
    if (!inventory) {
        if (errorMessage) *errorMessage = QStringLiteral("Invalid inventory model");
        return false;
    }
    
    // Extract all templates from the model
    QList<ResourceTemplate> templates;
    
    for (int row = 0; row < inventory->rowCount(); ++row) {
        QStandardItem* item = inventory->item(row);
        if (!item) continue;
        
        QVariant itemData = item->data(Qt::UserRole);
        // FIXME : there was as check for (!itemData.isValid()) continue ?? why??
        
        // Try to extract as ResourceTemplate
        if (itemData.canConvert<ResourceTemplate>()) {
            ResourceTemplate tmpl = itemData.value<ResourceTemplate>();
            templates.append(tmpl);
        }
    }
    
    if (templates.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("No templates to save");
        return false;
    }
    
    // Convert to JSON
    scadtemplates::TemplateParser parser;
    QString json = parser.toJson(templates);
    
    // Write to file
    QFile file(fileInfo.filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = QStringLiteral("Could not open file for writing: %1").arg(file.errorString());
        return false;
    }
    
    file.write(json.toUtf8());
    
    if (file.error() != QFile::NoError) {
        if (errorMessage) *errorMessage = QStringLiteral("Error writing file: %1").arg(file.errorString());
        return false;
    }
    
    return true;
}

} // namespace resourceInventory
