/**
 * @file ResourceScanner.cpp
 * @brief Implementation of top-level resource scanner
 */

#include "ResourceScanner.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"

#include <QDir>
#include <QDirListing>
#include <QDebug>

namespace resourceScanning {

bool ResourceScanner::scanToModel(QStandardItemModel* model, 
                                   const QList<platformInfo::ResourceLocation>& locations)
{
    if (!model) {
        qWarning() << "ResourceScanner::scanToModel: null model provided";
        return false;
    }
    
    // Clear existing data
    model->clear();
    m_examplesInventory.clear();
    
    // Scan each location
    for (const auto& location : locations) {
        qDebug() << "Scanning location:" << location.path() 
                 << "tier:" << resourceMetadata::tierToString(location.tier());
        
        // Phase 3: Examples only
        int examplesAdded = scanExamplesAt(location);
        if (examplesAdded > 0) {
            qDebug() << "  Added" << examplesAdded << "examples";
        }
        
        // Future phases: scanTemplatesAt(), scanLibrariesAt(), etc.
    }
    
    // Populate model from inventories
    populateModel(model);
    
    qDebug() << "ResourceScanner: Total examples:" << m_examplesInventory.count();
    
    return true;
}

int ResourceScanner::scanExamplesAt(const platformInfo::ResourceLocation& location)
{
    QString examplesPath = location.path() + "/examples";
    
    // Check if examples folder exists
    QDir examplesDir(examplesPath);
    if (!examplesDir.exists()) {
        return 0; // Not an error - location may not have examples
    }
    
    int addedCount = 0;
    QString tierStr = resourceMetadata::tierToString(location.tier());
    
    // Traverse examples folder - one level only (category folders)
    for (const auto& entry : QDirListing(examplesPath, QDirListing::IteratorFlag::Recursive)) {
        if (entry.isDir()) {
            // Category folder (e.g., "BasicShapes", "Functions")
            QString categoryName = entry.baseName();
            if (m_examplesInventory.addFolder(entry, tierStr, categoryName)) {
                addedCount++;
            }
        } else if (entry.isFile() && entry.fileName().endsWith(".scad")) {
            // Loose .scad file in examples root (uncategorized)
            if (m_examplesInventory.addExample(entry, tierStr, "uncategorized")) {
                addedCount++;
            }
        }
    }
    
    return addedCount;
}

void ResourceScanner::populateModel(QStandardItemModel* model)
{
    // Phase 3: Simple flat list of examples for debugging
    // Future: Hierarchical model with Tier → ResourceType → Category → Items
    
    model->setHorizontalHeaderLabels({"Name", "Category", "Tier", "Path"});
    
    // Get all examples
    QList<QVariant> allExamples = m_examplesInventory.getAll();
    
    for (const QVariant& var : allExamples) {
        if (!var.canConvert<resourceInventory::ResourceScript>()) {
            continue;
        }
        
        resourceInventory::ResourceScript script = var.value<resourceInventory::ResourceScript>();
        
        QList<QStandardItem*> row;
        row.append(new QStandardItem(script.displayName()));
        row.append(new QStandardItem(script.category()));
        row.append(new QStandardItem(resourceMetadata::tierToString(script.tier())));
        row.append(new QStandardItem(script.scriptPath()));
        
        model->appendRow(row);
    }
}

} // namespace resourceScanning
