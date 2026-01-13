/**
 * @file resourceScanner_minimal.cpp
 * @brief Minimal ResourceScanner implementation for testing callback API only
 * 
 * This file contains ONLY the new callback-based scanning methods.
 * No GUI/Widget dependencies. For Phase 1 testing only.
 */

#include "resourceScanner_test.hpp"
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QStandardItemModel>
#include <QStandardItem>

// Allowed attachments for script-like resources
static const QStringList kScriptAttachmentFilters = {
    QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"),
    QStringLiteral("*.svg"), QStringLiteral("*.gif"),
    QStringLiteral("*.json"), QStringLiteral("*.txt"), QStringLiteral("*.csv"),
    QStringLiteral("*.stl"), QStringLiteral("*.off"), QStringLiteral("*.dxf"),
    QStringLiteral("*.dat")
};

namespace resourceInventory {

// ============================================================================
// Constructor (minimal)
// ============================================================================

ResourceScanner::ResourceScanner()
{
}

// ============================================================================
// NEW CALLBACK-BASED API (Phase 1)
// ============================================================================

void ResourceScanner::scanTemplates(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey,
    ItemCallback onItemFound)
{
    if (!onItemFound) return;  // Null callback guard
    
    QDir dir(basePath);
    if (!dir.exists()) return;
    
    // Templates: subfolders define categories
    // First scan top-level templates (no category)
    QStringList filters = {QStringLiteral("*.scad"), QStringLiteral("*.json")};
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fi : files) {
        ResourceItem item(fi.absoluteFilePath(), ResourceType::Templates, tier);
        item.setName(fi.baseName());
        item.setDisplayName(fi.baseName());
        item.setSourcePath(fi.absoluteFilePath());
        item.setSourceLocationKey(locationKey);
        item.setAccess(ResourceAccess::ReadWrite);
        onItemFound(item);
    }
    
    // Scan subfolders (each is a category)
    QStringList subfolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& subfolder : subfolders) {
        QString subPath = dir.absoluteFilePath(subfolder);
        
        // Recursive scan with callback streaming
        std::function<void(const QString&, const QString&)> scanRecursive;
        scanRecursive = [&](const QString& folderPath, const QString& category) {
            QDir d(folderPath);
            if (!d.exists()) return;
            
            // Scan files in this folder
            QFileInfoList subFiles = d.entryInfoList(filters, QDir::Files);
            for (const QFileInfo& fi : subFiles) {
                ResourceItem item(fi.absoluteFilePath(), ResourceType::Templates, tier);
                item.setName(fi.baseName());
                item.setDisplayName(fi.baseName());
                item.setSourcePath(fi.absoluteFilePath());
                item.setSourceLocationKey(locationKey);
                item.setCategory(category);
                item.setAccess(ResourceAccess::ReadWrite);
                onItemFound(item);
            }
            
            // Recurse into subfolders
            QStringList subs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString& sub : subs) {
                if (sub == QLatin1String("build") ||
                    sub == QLatin1String(".git") ||
                    sub == QLatin1String("node_modules")) {
                    continue;
                }
                
                QString newCategory = category.isEmpty() ? sub
                                                          : (category + QLatin1Char('/') + sub);
                scanRecursive(d.absoluteFilePath(sub), newCategory);
            }
        };
        
        scanRecursive(subPath, subfolder);
    }
}

QVector<ResourceItem> ResourceScanner::scanTemplatesToList(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey)
{
    QVector<ResourceItem> results;
    
    scanTemplates(basePath, tier, locationKey, [&results](const ResourceItem& item) {
        results.append(item);
    });
    
    return results;
}

void ResourceScanner::scanTemplatesToModel(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey,
    QStandardItemModel* model)
{
    if (!model) return;
    
    scanTemplates(basePath, tier, locationKey, [this, model](const ResourceItem& item) {
        addItemToModel(model, item);
    });
}

void ResourceScanner::addItemToModel(QStandardItemModel* model, const ResourceItem& item)
{
    if (!model) return;
    
    // Custom roles for metadata storage
    enum ItemRole {
        TypeRole = Qt::UserRole + 1,
        TierRole = Qt::UserRole + 2,
        PathRole = Qt::UserRole + 3,
        CategoryRole = Qt::UserRole + 4,
        AccessRole = Qt::UserRole + 5,
        LocationKeyRole = Qt::UserRole + 6
    };
    
    auto* standardItem = new QStandardItem(item.displayName());
    
    // Store metadata in custom roles
    standardItem->setData(static_cast<int>(item.type()), TypeRole);
    standardItem->setData(static_cast<int>(item.tier()), TierRole);
    standardItem->setData(item.sourcePath(), PathRole);
    standardItem->setData(item.category(), CategoryRole);
    standardItem->setData(static_cast<int>(item.access()), AccessRole);
    standardItem->setData(item.sourceLocationKey(), LocationKeyRole);
    
    // Add to model
    model->appendRow(standardItem);
}

// ============================================================================
// STUBBED OUT LEGACY METHODS (not used in Phase 1 tests)
// ============================================================================

QVector<ResourceItem> ResourceScanner::scanLocation(
    const platformInfo::ResourceLocation& location,
    ResourceType type,
    ResourceTier tier)
{
    Q_UNUSED(location);
    Q_UNUSED(type);
    Q_UNUSED(tier);
    return {};  // Stub
}

void ResourceScanner::scanToTree(
    const QVector<platformInfo::ResourceLocation>& locations,
    ResourceType type,
    ResourceTier tier,
    ResourceTreeWidget* tree)
{
    Q_UNUSED(locations);
    Q_UNUSED(type);
    Q_UNUSED(tier);
    Q_UNUSED(tree);
    // Stub - not used in callback tests
}

void ResourceScanner::scanAllTiers(
    const QVector<platformInfo::ResourceLocation>& installLocs,
    const QVector<platformInfo::ResourceLocation>& machineLocs,
    const QVector<platformInfo::ResourceLocation>& userLocs,
    ResourceType type,
    ResourceTreeWidget* tree)
{
    Q_UNUSED(installLocs);
    Q_UNUSED(machineLocs);
    Q_UNUSED(userLocs);
    Q_UNUSED(type);
    Q_UNUSED(tree);
    // Stub - not used in callback tests
}

void ResourceScanner::scanLibraries(
    const QVector<platformInfo::ResourceLocation>& locations,
    ResourceTier tier,
    ResourceTreeWidget* tree)
{
    Q_UNUSED(locations);
    Q_UNUSED(tier);
    Q_UNUSED(tree);
    // Stub - not used in callback tests
}

QString ResourceScanner::resourceSubfolder(ResourceType type)
{
    switch (type) {
        case ResourceType::Templates:    return QStringLiteral("templates");
        case ResourceType::Libraries:     return QStringLiteral("libraries");
        case ResourceType::Examples:     return QStringLiteral("examples");
        default:                        return QString();
    }
}

QStringList ResourceScanner::resourceExtensions(ResourceType type)
{
    switch (type) {
        case ResourceType::Templates:
        case ResourceType::Libraries:
        case ResourceType::Examples:
            return {QStringLiteral(".scad")};
        default:
            return {};
    }
}

} // namespace resourceInventory
