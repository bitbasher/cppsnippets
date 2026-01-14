#include "resourceScanner.hpp"
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QStandardItemModel>
#include <QStandardItem>

// Allowed attachments for script-like resources (examples/tests/templates)
static const QStringList kScriptAttachmentFilters = {
    QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"),
    QStringLiteral("*.svg"), QStringLiteral("*.gif"),
    QStringLiteral("*.json"), QStringLiteral("*.txt"), QStringLiteral("*.csv"),
    QStringLiteral("*.stl"), QStringLiteral("*.off"), QStringLiteral("*.dxf"),
    QStringLiteral("*.dat")
};

namespace resourceInventory {

// Shared no-op scanner for resource types that are handled elsewhere
static QList<ResourceItem> scanNoOp(const QString& basePath,
                                      ResourceTier tier,
                                      const QString& locationKey)
{
    Q_UNUSED(basePath);
    Q_UNUSED(tier);
    Q_UNUSED(locationKey);
    return {};
}

// ============================================================================
// ResourceScanner
// ============================================================================

ResourceScanner::ResourceScanner(QObject* parent)
    : QObject(parent)
{
}

QString ResourceScanner::resourceSubfolder(ResourceType type)
{
    switch (type) {
        case ResourceType::ColorSchemes: return QStringLiteral("color-schemes");
        case ResourceType::RenderColors: return QStringLiteral("color-schemes/render");
        case ResourceType::EditorColors: return QStringLiteral("color-schemes/editor");
        case ResourceType::Fonts:        return QStringLiteral("fonts");
        case ResourceType::Libraries:     return QStringLiteral("libraries");
        case ResourceType::Examples:     return QStringLiteral("examples");
        case ResourceType::Tests:        return QStringLiteral("tests");
        case ResourceType::Templates:    return QStringLiteral("templates");
        case ResourceType::Shaders:      return QStringLiteral("shaders");
        case ResourceType::Translations: return QStringLiteral("locale");
        default:                        return QString();
    }
}

QStringList ResourceScanner::resourceExtensions(ResourceType type)
{
    switch (type) {
        case ResourceType::ColorSchemes:
        case ResourceType::RenderColors:
        case ResourceType::EditorColors:
            return {QStringLiteral(".json")};
        case ResourceType::Fonts:
            return {QStringLiteral(".ttf"), QStringLiteral(".otf"), 
                    QStringLiteral(".woff"), QStringLiteral(".woff2")};
        case ResourceType::Libraries:
        case ResourceType::Examples:
        case ResourceType::Tests:
        case ResourceType::Templates:
            return {QStringLiteral(".scad")};
        case ResourceType::Shaders:
            return {QStringLiteral(".vert"), QStringLiteral(".frag"), 
                    QStringLiteral(".glsl")};
        case ResourceType::Translations:
            return {QStringLiteral(".qm"), QStringLiteral(".ts")};
        default:
            return {};
    }
}

QList<ResourceItem> ResourceScanner::scanLocation(
    const platformInfo::ResourceLocation& location,
    ResourceType type,
    ResourceTier tier)
{
    if (!location.exists() || !location.isEnabled()) {
        return {};
    }
    
    QString basePath = location.path();
    QString locationKey = location.getDisplayName();
    
    // Determine the subfolder for this resource type
    QString subfolder = resourceSubfolder(type);
    if (!subfolder.isEmpty()) {
        basePath = QDir::cleanPath(basePath + QLatin1Char('/') + subfolder);
    }
    
    QDir dir(basePath);
    if (!dir.exists()) {
        return {};
    }
    
    switch (type) {
        case ResourceType::ColorSchemes:
            return scanColorSchemes(basePath, tier, locationKey);
        case ResourceType::RenderColors:
            return scanRenderColors(basePath, tier, locationKey);
        case ResourceType::EditorColors:
            return scanEditorColors(basePath, tier, locationKey);
        case ResourceType::Fonts:
            return scanFonts(basePath, tier, locationKey);
        case ResourceType::Examples:
            return scanExamples(basePath, tier, locationKey);
        case ResourceType::Tests:
            return scanTests(basePath, tier, locationKey);
        case ResourceType::Templates:
            // Use callback-based version - this shouldn't be called
            return {};
        case ResourceType::Translations:
            return scanTranslations(basePath, tier, locationKey);
        case ResourceType::Libraries:
            // Libraries need special handling - each subfolder is a library
            return {};  // Will be converted to callback in Phase 6
        default:
            return {};
    }
}

// ============================================================================
// Legacy scan methods (will be converted to callbacks in Phase 6)
// ============================================================================

QList<ResourceItem> ResourceScanner::scanColorSchemes(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QList<ResourceItem> results;
    
    // ColorSchemes is a container type that scans for both editor and render colors
    // Scan for .json files at the basePath level and categorize them
    QDir dir(basePath);
    if (!dir.exists()) return results;
    
    QStringList filters = {QStringLiteral("*.json")};
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    // For now, create ColorSchemes items but a more sophisticated approach
    // could parse the JSON to determine if they're editor or render colors
    for (const QFileInfo& fi : files) {
        ResourceItem item(fi.absoluteFilePath(), ResourceType::ColorSchemes, tier);
        item.setName(fi.baseName());
        item.setDisplayName(fi.baseName());
        item.setSourcePath(fi.absoluteFilePath());
        item.setSourceLocationKey(locationKey);
        item.setAccess(ResourceAccess::ReadOnly);
        results.append(item);
    }
    
    return results;
}

QList<ResourceItem> ResourceScanner::scanRenderColors(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QList<ResourceItem> results;
    QDir dir(basePath);
    
    if (!dir.exists()) return results;
    
    QStringList filters = {QStringLiteral("*.json")};
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fi : files) {
        ResourceItem item(fi.absoluteFilePath(), ResourceType::RenderColors, tier);
        item.setName(fi.baseName());
        item.setDisplayName(fi.baseName());
        item.setSourcePath(fi.absoluteFilePath());
        item.setSourceLocationKey(locationKey);
        item.setAccess(ResourceAccess::ReadOnly);
        results.append(item);
    }
    
    return results;
}

QList<ResourceItem> ResourceScanner::scanEditorColors(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QList<ResourceItem> results;
    QDir dir(basePath);
    
    if (!dir.exists()) return results;
    
    QStringList filters = {QStringLiteral("*.json")};
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fi : files) {
        ResourceItem item(fi.absoluteFilePath(), ResourceType::EditorColors, tier);
        item.setName(fi.baseName());
        item.setDisplayName(fi.baseName());
        item.setSourcePath(fi.absoluteFilePath());
        item.setSourceLocationKey(locationKey);
        item.setAccess(ResourceAccess::ReadOnly);
        results.append(item);
    }
    
    return results;
}

QList<ResourceItem> ResourceScanner::scanFonts(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    // Fonts are supplied by the system; only bundled font is handled elsewhere.
    return scanNoOp(basePath, tier, locationKey);
}

QList<ResourceItem> ResourceScanner::scanExamples(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QList<ResourceItem> results;
    
    // One-level scan: top-level .scad files plus immediate subfolders
    QDir dir(basePath);
    if (!dir.exists()) return results;

    auto processDir = [&](const QString& path, const QString& category) {
        QDir d(path);
        QFileInfoList files = d.entryInfoList({QStringLiteral("*.scad")}, QDir::Files);
        for (const QFileInfo& fi : files) {
            ResourceScript script = scanScriptWithAttachments(fi.absoluteFilePath(),
                                                              ResourceType::Examples,
                                                              tier,
                                                              locationKey);
            script.setCategory(category);
            results.append(script);
        }
    };

    // Top-level examples
    processDir(basePath, QString());

    // One-level subfolders as categories
    QStringList subfolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& sub : subfolders) {
        processDir(dir.absoluteFilePath(sub), sub);
    }

    return results;
}

// ============================================================================
// NEW CALLBACK-BASED API (Phase 1)
// ============================================================================

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
    // Support both .scad (legacy) and .json (modern VS Code snippet format) templates
    QStringList filters = {QStringLiteral("*.scad"), QStringLiteral("*.json")};
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fi : files) {
        ResourceItem item(fi.absoluteFilePath(), ResourceType::Templates, tier);
        item.setName(fi.baseName());
        item.setDisplayName(fi.baseName());
        item.setSourcePath(fi.absoluteFilePath());
        item.setSourceLocationKey(locationKey);
        item.setAccess(ResourceAccess::ReadWrite);  // Templates are writable
        onItemFound(item);  // Stream to callback
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
                ResourceScript script = scanScriptWithAttachments(fi.absoluteFilePath(),
                                                                 ResourceType::Templates,
                                                                 tier,
                                                                 locationKey);
                script.setCategory(category);
                onItemFound(script);  // Stream to callback
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
        ItemDataRole = Qt::UserRole,  // Store full ResourceItem
        TypeRole = Qt::UserRole + 1,
        TierRole = Qt::UserRole + 2,
        PathRole = Qt::UserRole + 3,
        CategoryRole = Qt::UserRole + 4,
        AccessRole = Qt::UserRole + 5,
        LocationKeyRole = Qt::UserRole + 6
    };
    
    auto* standardItem = new QStandardItem(item.displayName());
    
    // Store full item as QVariant for easy retrieval
    standardItem->setData(QVariant::fromValue(item), ItemDataRole);
    
    // Store metadata in custom roles (for filtering/sorting)
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
        QString displayName = loc.getDisplayName();
        ResourceTier tier = loc.tier();
        
        // Scan templates
        QString templatesPath = QDir::cleanPath(basePath + QStringLiteral("/templates"));
        if (QDir(templatesPath).exists()) {
            scanTemplates(templatesPath, tier, displayName, addToModel);
        }
        
        // Scan examples
        QString examplesPath = QDir::cleanPath(basePath + QStringLiteral("/examples"));
        if (QDir(examplesPath).exists()) {
            scanExamples(examplesPath, tier, displayName, addToModel);
        }
    }
}

// ============================================================================
// LEGACY API (to be removed in Phase 5)
// ============================================================================

QList<ResourceItem> ResourceScanner::scanTranslations(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    // Translations are handled elsewhere; discovery is intentionally disabled here.
    return scanNoOp(basePath, tier, locationKey);
}

QList<ResourceItem> ResourceScanner::scanTests(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QList<ResourceItem> results;
    
    // One-level scan: top-level .scad files plus immediate subfolders
    QDir dir(basePath);
    if (!dir.exists()) return results;

    auto processDir = [&](const QString& path, const QString& category) {
        QDir d(path);
        QFileInfoList files = d.entryInfoList({QStringLiteral("*.scad")}, QDir::Files);
        for (const QFileInfo& fi : files) {
            ResourceScript script = scanScriptWithAttachments(fi.absoluteFilePath(),
                                                              ResourceType::Tests,
                                                              tier,
                                                              locationKey);
            script.setCategory(category);
            results.append(script);
        }
    };

    // Top-level tests
    processDir(basePath, QString());

    // One-level subfolders as categories
    QStringList subfolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& sub : subfolders) {
        processDir(dir.absoluteFilePath(sub), sub);
    }

    return results;
}

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

void ResourceScanner::scanFolderRecursive(
    const QString& folderPath,
    const QStringList& extensions,
    ResourceType type,
    ResourceTier tier,
    const QString& locationKey,
    const QString& category,
    QList<ResourceItem>& results)
{
    QDir dir(folderPath);
    if (!dir.exists()) return;
    
    // Build filter list
    QStringList filters;
    for (const QString& ext : extensions) {
        filters << (QStringLiteral("*") + ext);
    }
    
    // Scan files in this folder
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& fi : files) {
        ResourceScript script = scanScriptWithAttachments(fi.absoluteFilePath(), 
                                                           type, tier, locationKey);
        script.setCategory(category);
        results.append(script);
    }
    
    // Recurse into subfolders (for templates, this extends the category)
    QStringList subfolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& subfolder : subfolders) {
        // Skip common non-resource folders
        if (subfolder == QLatin1String("build") || 
            subfolder == QLatin1String(".git") ||
            subfolder == QLatin1String("node_modules")) {
            continue;
        }
        
        QString newCategory = category.isEmpty() ? subfolder 
                                                  : (category + QLatin1Char('/') + subfolder);
        
        scanFolderRecursive(dir.absoluteFilePath(subfolder), extensions,
                           type, tier, locationKey, newCategory, results);
    }
}

} // namespace resourceInventory
