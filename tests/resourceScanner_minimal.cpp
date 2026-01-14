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

QList<ResourceItem> ResourceScanner::scanTemplatesToList(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey)
{
    QList<ResourceItem> results;
    
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

// ============================================================================
// Examples Scanning (Phase 6)
// ============================================================================

ResourceScript ResourceScanner::scanScriptWithAttachments(
    const QString& scriptPath,
    ResourceType type,
    ResourceTier tier,
    const QString& locationKey)
{
    ResourceScript script(scriptPath);
    script.setType(type);
    script.setTier(tier);
    script.setScriptPath(scriptPath);
    script.setSourcePath(scriptPath);
    script.setSourceLocationKey(locationKey);
    script.setAccess(type == ResourceType::Templates ? ResourceAccess::ReadWrite 
                                                     : ResourceAccess::ReadOnly);
    
    // Look for attachments in the same folder
    QFileInfo fi(scriptPath);
    QDir dir = fi.dir();
    
    // Only look for attachments that share the script's base name or are in a data subfolder
    QString baseName = fi.baseName();
    QFileInfoList candidates = dir.entryInfoList(kScriptAttachmentFilters, QDir::Files);
    
    for (const QFileInfo& candidate : candidates) {
        // Include if filename starts with script's base name
        if (candidate.baseName().startsWith(baseName)) {
            script.addAttachment(candidate.absoluteFilePath());
        }
    }
    
    // Check for a data subfolder with same name as script
    QString dataFolder = dir.absoluteFilePath(baseName);
    if (QDir(dataFolder).exists()) {
        QDirIterator it(dataFolder, kScriptAttachmentFilters, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            script.addAttachment(it.next());
        }
    }
    
    return script;
}

void ResourceScanner::scanExamples(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey,
    ItemCallback onItemFound)
{
    if (!onItemFound) return;  // Null callback guard
    
    QDir dir(basePath);
    if (!dir.exists()) return;

    // Scan top-level .scad files (examples without category)
    QFileInfoList topLevelFiles = dir.entryInfoList({QStringLiteral("*.scad")}, QDir::Files);
    for (const QFileInfo& fi : topLevelFiles) {
        ResourceScript script = scanScriptWithAttachments(fi.absoluteFilePath(),
                                                          ResourceType::Examples,
                                                          tier,
                                                          locationKey);
        // No category for top-level
        onItemFound(script);
    }

    // Process subfolders based on resource type
    QStringList subfolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& sub : subfolders) {
        QString subPath = dir.absoluteFilePath(sub);
        QString lowerSub = sub.toLower();
        
        if (lowerSub == QStringLiteral("templates")) {
            // Delegate to templates scanner
            scanTemplates(subPath, tier, locationKey, onItemFound);
        }
        else if (lowerSub == QStringLiteral("tests")) {
            // Delegate to tests scanner (convert to callback first in Phase 6)
            // For now, scan as Group
            scanGroup(subPath, tier, locationKey, sub, onItemFound);
        }
        else {
            // It's a Group (category folder) - scan .scad files with attachments
            scanGroup(subPath, tier, locationKey, sub, onItemFound);
        }
    }
}

// Helper: Scan a Group folder (category with .scad scripts + attachments, no recursion)
void ResourceScanner::scanGroup(
    const QString& groupPath,
    ResourceTier tier,
    const QString& locationKey,
    const QString& category,
    ItemCallback onItemFound)
{
    if (!onItemFound) return;
    
    QDir dir(groupPath);
    if (!dir.exists()) return;
    
    // Scan all .scad files in this Group folder
    QFileInfoList files = dir.entryInfoList({QStringLiteral("*.scad")}, QDir::Files);
    for (const QFileInfo& fi : files) {
        ResourceScript script = scanScriptWithAttachments(fi.absoluteFilePath(),
                                                          ResourceType::Examples,
                                                          tier,
                                                          locationKey);
        script.setCategory(category);
        onItemFound(script);
    }
}


QList<ResourceItem> ResourceScanner::scanExamplesToList(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey)
{
    QList<ResourceItem> results;
    
    scanExamples(basePath, tier, locationKey, [&results](const ResourceItem& item) {
        results.append(item);
    });
    
    return results;
}

void ResourceScanner::scanExamplesToModel(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey,
    QStandardItemModel* model)
{
    if (!model) return;
    
    scanExamples(basePath, tier, locationKey, [this, model](const ResourceItem& item) {
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
// Phase 2: High-Level scanToModel() API
// ============================================================================

void ResourceScanner::scanToModel(QStandardItemModel* model,
                                  const QList<platformInfo::ResourceLocation>& locations)
{
    if (!model) {
        return;
    }
    
    // Define callback that adds items to model
    auto addToModel = [this, model](const ResourceItem& item) {
        addItemToModel(model, item);
    };
    
    // Scan all locations (tier is encoded in each location)
    for (const auto& loc : locations) {
        QString basePath = loc.path();
        if (!QDir(basePath).exists()) continue;
        
        QString displayName = loc.getDisplayName();
        ResourceTier tier = loc.tier();
        
        // Scan templates (only type implemented so far)
        QString templatesPath = QDir::cleanPath(basePath + QStringLiteral("/templates"));
        if (QDir(templatesPath).exists()) {
            scanTemplates(templatesPath, tier, displayName, addToModel);
        }
    }
}

// ============================================================================
// STUBBED OUT LEGACY METHODS (not used in Phase 1 tests)
// ============================================================================

QList<ResourceItem> ResourceScanner::scanLocation(
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
    const QList<platformInfo::ResourceLocation>& locations,
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
    const QList<platformInfo::ResourceLocation>& installLocs,
    const QList<platformInfo::ResourceLocation>& machineLocs,
    const QList<platformInfo::ResourceLocation>& userLocs,
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
    const QList<platformInfo::ResourceLocation>& locations,
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
