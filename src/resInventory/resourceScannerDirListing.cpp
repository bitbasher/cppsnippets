/**
 * @file resourceScannerDirListing.cpp
 * @brief Implementation of QDirListing-based resource scanner
 * 
 * Uses Qt 6.8+ QDirListing for efficient streaming directory iteration.
 */

#include "resInventory/resourceScannerDirListing.h"

#include <QDir>
#include <QDirListing>
#include <QFileInfo>
#include <QTimeZone>

namespace resInventory {

// ============================================================================
// Static helper functions
// ============================================================================

QString ResourceScannerDirListing::resourceSubfolder(ResourceType type)
{
    switch (type) {
        case ResourceType::RenderColors:
            return QStringLiteral("color-schemes/render");
        case ResourceType::EditorColors:
            return QStringLiteral("color-schemes/editor");
        case ResourceType::Font:
            return QStringLiteral("fonts");
        case ResourceType::Library:
            return QStringLiteral("libraries");
        case ResourceType::Example:
            return QStringLiteral("examples");
        case ResourceType::Test:
            return QStringLiteral("tests");
        case ResourceType::Template:
            return QStringLiteral("templates");
        case ResourceType::Translation:
            return QStringLiteral("locale");
        default:
            return QString();
    }
}

QStringList ResourceScannerDirListing::resourceFilters(ResourceType type)
{
    switch (type) {
        case ResourceType::RenderColors:
        case ResourceType::EditorColors:
        case ResourceType::Template:      // Templates are JSON only
            return {QStringLiteral("*.json")};
        case ResourceType::Font:
            return {QStringLiteral("*.ttf"), QStringLiteral("*.otf"), 
                    QStringLiteral("*.woff"), QStringLiteral("*.woff2")};
        case ResourceType::Library:
        case ResourceType::Example:       // .scad primary, attachments (.json etc) are not scanned as resources
        case ResourceType::Test:
            return {QStringLiteral("*.scad")};
        case ResourceType::Translation:
            return {QStringLiteral("*.qm"), QStringLiteral("*.ts")};
        default:
            return {};
    }
}

ResourceType ResourceScannerDirListing::categorizeByPath(const QString& filePath)
{
    // Normalize path separators
    QString normalized = QDir::fromNativeSeparators(filePath).toLower();
    
    // Check for known resource subfolders in the path
    if (normalized.contains(QLatin1String("/color-schemes/"))) {
        // Distinguish render vs editor colors by filename pattern
        if (normalized.contains(QLatin1String("editor")) || 
            normalized.contains(QLatin1String("syntax"))) {
            return ResourceType::EditorColors;
        }
        return ResourceType::RenderColors;
    }
    
    if (normalized.contains(QLatin1String("/fonts/"))) {
        return ResourceType::Font;
    }
    
    if (normalized.contains(QLatin1String("/libraries/"))) {
        return ResourceType::Library;
    }
    
    if (normalized.contains(QLatin1String("/examples/"))) {
        return ResourceType::Example;
    }
    
    if (normalized.contains(QLatin1String("/tests/"))) {
        return ResourceType::Test;
    }
    
    // Templates from both standard location and newresources drop zone
    if (normalized.contains(QLatin1String("/templates/"))) {
        return ResourceType::Template;
    }
    
    if (normalized.contains(QLatin1String("/locale/"))) {
        return ResourceType::Translation;
    }
    
    return ResourceType::Unknown;
}

// ============================================================================
// ResourceScannerDirListing
// ============================================================================

ResourceScannerDirListing::ResourceScannerDirListing(QObject* parent)
    : QObject(parent)
{
}

int ResourceScannerDirListing::scanLocation(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey,
    const ScanCallback& callback)
{
    QDir baseDir(basePath);
    if (!baseDir.exists()) {
        emit scanError(basePath, QStringLiteral("Directory does not exist"));
        return 0;
    }
    
    int totalCount = 0;
    
    // Scan for each known resource type
    const QVector<ResourceType> typesToScan = {
        ResourceType::RenderColors,
        ResourceType::EditorColors,
        ResourceType::Font,
        ResourceType::Library,
        ResourceType::Example,
        ResourceType::Test,
        ResourceType::Template,
        ResourceType::Translation
    };
    
    for (ResourceType type : typesToScan) {
        totalCount += scanLocationForType(basePath, type, tier, locationKey, callback);
    }
    
    return totalCount;
}

int ResourceScannerDirListing::scanLocationForType(
    const QString& basePath,
    ResourceType type,
    ResourceTier tier,
    const QString& locationKey,
    const ScanCallback& callback)
{
    QString subfolder = resourceSubfolder(type);
    if (subfolder.isEmpty()) {
        return 0;
    }
    
    QString scanPath = QDir::cleanPath(basePath + QLatin1Char('/') + subfolder);
    QDir scanDir(scanPath);
    
    int count = 0;
    
    // Scan standard resource type directory
    if (scanDir.exists()) {
        emit scanStarted(scanPath, type);
        
        // Determine if this type needs recursive scanning
        bool recursive = false;
        switch (type) {
            case ResourceType::Library:
            case ResourceType::Example:
            case ResourceType::Test:
            case ResourceType::Template:
                recursive = true;
                break;
            default:
                recursive = false;
                break;
        }
        
        count += scanWithDirListing(scanPath, type, tier, locationKey, 
                                    QString(), callback, recursive);
        
        emit scanCompleted(scanPath, count);
    }
    
    // For templates, also scan the newresources drop zone
    if (type == ResourceType::Template) {
        QString newResourcesPath = QDir::cleanPath(basePath + QLatin1String("/newresources/templates"));
        QDir newResourcesDir(newResourcesPath);
        
        if (newResourcesDir.exists()) {
            emit scanStarted(newResourcesPath, type);
            
            int newResourcesCount = scanWithDirListing(newResourcesPath, type, tier, locationKey,
                                                        QString(), callback, true);
            count += newResourcesCount;
            
            emit scanCompleted(newResourcesPath, newResourcesCount);
        }
    }
    
    return count;
}

QVector<DiscoveredResource> ResourceScannerDirListing::collectAll(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey)
{
    QVector<DiscoveredResource> results;
    
    scanLocation(basePath, tier, locationKey, 
                 [&results](const DiscoveredResource& res) {
                     results.append(res);
                 });
    
    return results;
}

int ResourceScannerDirListing::scanWithDirListing(
    const QString& scanPath,
    ResourceType type,
    ResourceTier tier,
    const QString& locationKey,
    const QString& category,
    const ScanCallback& callback,
    bool recursive)
{
    QStringList filters = resourceFilters(type);
    if (filters.isEmpty()) {
        return 0;
    }
    
    int count = 0;
    
    // Configure QDirListing iterator flags
    using F = QDirListing::IteratorFlag;
    QDirListing::IteratorFlags flags = F::FilesOnly;
    if (recursive) {
        flags |= F::Recursive;
    }
    
    // Create QDirListing with name filters
    QDirListing dirListing(scanPath, filters, flags);
    
    for (const auto& entry : dirListing) {
        DiscoveredResource res;
        res.path = entry.absoluteFilePath();
        res.name = entry.fileName();
        res.type = type;
        res.tier = tier;
        res.locationKey = locationKey;
        res.lastModified = entry.lastModified(QTimeZone::LocalTime);
        res.size = entry.size();
        
        // Compute category from path relative to scanPath
        QDir scanDir(scanPath);
        QString relativePath = scanDir.relativeFilePath(res.path);
        int lastSep = relativePath.lastIndexOf(QLatin1Char('/'));
        if (lastSep > 0) {
            res.category = relativePath.left(lastSep);
        } else if (!category.isEmpty()) {
            res.category = category;
        }
        
        // Emit signal and invoke callback
        emit resourceFound(res);
        callback(res);
        ++count;
    }
    
    return count;
}

} // namespace resInventory
