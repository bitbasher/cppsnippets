#include "resInventory/resourceScanner.h"
#include "resInventory/resourceTreeWidget.h"
#include <platformInfo/resourceLocationManager.h>

#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logResourceScanner, "openscad.scanner", QtInfoMsg)

// Allowed attachments for script-like resources (examples/tests/templates)
static const QStringList kScriptAttachmentFilters = {
    QStringLiteral("*.png"),  QStringLiteral("*.jpg"),  QStringLiteral("*.jpeg"),
    QStringLiteral("*.svg"),  QStringLiteral("*.gif"),
    QStringLiteral("*.json"), QStringLiteral("*.txt"),  QStringLiteral("*.csv"),
    QStringLiteral("*.stl"),  QStringLiteral("*.off"),  QStringLiteral("*.dxf"),
    QStringLiteral("*.dat"),  QStringLiteral("*.csg")
};

namespace resInventory {

// Static member initialization
bool ResourceScanner::m_loggingEnabled = false;

// Shared no-op scanner for resource types that are handled elsewhere
static QVector<ResourceItem> scanNoOp(const QString& basePath,
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

void ResourceScanner::enableLogging(bool enable)
{
    m_loggingEnabled = enable;
    if (enable) {
        QLoggingCategory::setFilterRules(QStringLiteral("openscad.scanner=true"));
        qCInfo(logResourceScanner) << "=== SCANNER LOGGING ENABLED ===";
    } else {
        QLoggingCategory::setFilterRules(QStringLiteral("openscad.scanner=false"));
    }
}

bool ResourceScanner::isLoggingEnabled()
{
    return m_loggingEnabled;
}

QString ResourceScanner::resourceSubfolder(ResourceType type)
{
    switch (type) {
        case ResourceType::ColorSchemes: return QStringLiteral("color-schemes");
        case ResourceType::RenderColors: return QStringLiteral("color-schemes/render");
        case ResourceType::EditorColors: return QStringLiteral("color-schemes/editor");
        case ResourceType::Font:        return QStringLiteral("fonts");
        case ResourceType::Library:     return QStringLiteral("libraries");
        case ResourceType::Example:     return QStringLiteral("examples");
        case ResourceType::Test:        return QStringLiteral("tests");
        case ResourceType::Template:    return QStringLiteral("templates");
        case ResourceType::Shader:      return QStringLiteral("shaders");
        case ResourceType::Translation: return QStringLiteral("locale");
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
        case ResourceType::Font:
            return {QStringLiteral(".ttf"), QStringLiteral(".otf"), 
                    QStringLiteral(".woff"), QStringLiteral(".woff2")};
        case ResourceType::Library:
        case ResourceType::Example:
        case ResourceType::Test:
        case ResourceType::Template:
            return {QStringLiteral(".scad")};
        case ResourceType::Shader:
            return {QStringLiteral(".vert"), QStringLiteral(".frag"), 
                    QStringLiteral(".glsl")};
        case ResourceType::Translation:
            return {QStringLiteral(".qm"), QStringLiteral(".ts")};
        default:
            return {};
    }
}

QVector<ResourceItem> ResourceScanner::scanLocation(
    const platformInfo::ResourceLocation& location,
    ResourceType type,
    ResourceTier tier)
{
    if (!location.exists || !location.isEnabled) {
        if (m_loggingEnabled) {
            qCInfo(logResourceScanner) << "SKIP:" << location.displayName << "exists=" << location.exists << "enabled=" << location.isEnabled;
        }
        return {};
    }
    
    QString basePath = location.path;
    QString locationKey = location.displayName;
    
    // Determine the subfolder for this resource type
    QString subfolder = resourceSubfolder(type);
    if (!subfolder.isEmpty()) {
        basePath = QDir::cleanPath(basePath + QLatin1Char('/') + subfolder);
    }
    
    QDir dir(basePath);
    if (!dir.exists()) {
        if (m_loggingEnabled) {
            qCInfo(logResourceScanner) << "NOT FOUND:" << basePath;
        }
        return {};
    }
    
    if (m_loggingEnabled) {
        qCInfo(logResourceScanner) << "SCAN:" << basePath << "type=" << static_cast<int>(type) << "tier=" << static_cast<int>(tier);
    }
    switch (type) {
        case ResourceType::ColorSchemes:
            return scanColorSchemes(basePath, tier, locationKey);
        case ResourceType::RenderColors:
            return scanRenderColors(basePath, tier, locationKey);
        case ResourceType::EditorColors:
            return scanEditorColors(basePath, tier, locationKey);
        case ResourceType::Font:
            return scanFonts(basePath, tier, locationKey);
        case ResourceType::Example:
            return scanExamples(basePath, tier, locationKey);
        case ResourceType::Test:
            return scanTests(basePath, tier, locationKey);
        case ResourceType::Template:
            return scanTemplates(basePath, tier, locationKey);
        case ResourceType::Translation:
            return scanTranslations(basePath, tier, locationKey);
        case ResourceType::Library:
            // Libraries need special handling - each subfolder is a library
            return {};  // Use scanLibraries() instead
        default:
            return {};
    }
}

void ResourceScanner::scanToTree(
    const QVector<platformInfo::ResourceLocation>& locations,
    ResourceType type,
    ResourceTier tier,
    ResourceTreeWidget* tree)
{
    if (!tree) return;
    
    emit scanStarted(type, locations.size());
    
    // Libraries require hierarchical handling; delegate to dedicated scanner
    if (type == ResourceType::Library) {
        scanLibraries(locations, tier, tree);
        emit scanCompleted(type, tree->allItems().size());
        return;
    }

    int totalItems = 0;
    for (const auto& loc : locations) {
        QVector<ResourceItem> items = scanLocation(loc, type, tier);
        
        for (const auto& item : items) {
            tree->addResource(item);
        }
        
        emit locationScanned(loc.path, items.size());
        totalItems += items.size();
    }
    
    emit scanCompleted(type, totalItems);
}

void ResourceScanner::scanAllTiers(
    const QVector<platformInfo::ResourceLocation>& installLocs,
    const QVector<platformInfo::ResourceLocation>& machineLocs,
    const QVector<platformInfo::ResourceLocation>& userLocs,
    ResourceType type,
    ResourceTreeWidget* tree)
{
    if (!tree) return;
    
    tree->setResourceType(type);
    
    // Templates use categories (subfolder-based grouping)
    tree->setShowCategories(type == ResourceType::Template);
    
    scanToTree(installLocs, type, ResourceTier::Installation, tree);
    scanToTree(machineLocs, type, ResourceTier::Machine, tree);
    scanToTree(userLocs, type, ResourceTier::User, tree);
}

void ResourceScanner::scanLibraries(
    const QVector<platformInfo::ResourceLocation>& locations,
    ResourceTier tier,
    ResourceTreeWidget* tree)
{
    if (!tree) return;
    
    tree->setResourceType(ResourceType::Library);
    
    for (const auto& loc : locations) {
        if (!loc.exists || !loc.isEnabled) continue;

        const QString librariesPath = QDir::cleanPath(loc.path + QStringLiteral("/libraries"));
        QDir librariesDir(librariesPath);

        if (!librariesDir.exists()) continue;

        // Each subfolder in libraries/ is a library
        const QStringList libraryFolders = librariesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QString& libName : libraryFolders) {
            const QString libPath = librariesDir.absoluteFilePath(libName);
            QDir libDir(libPath);

            // Create library root node regardless of content; children decide visibility
            ResourceItem libItem(libPath, ResourceType::Library, tier);
            libItem.setName(libName);
            libItem.setDisplayName(libName);
            libItem.setSourcePath(libPath);
            libItem.setSourceLocationKey(loc.displayName);
            libItem.setAccess(ResourceAccess::ReadOnly);

            ResourceTreeItem* libNode = tree->addResource(libItem);

            // Library root .scad scripts
            QFileInfoList rootScripts = libDir.entryInfoList({QStringLiteral("*.scad")}, QDir::Files);
            for (const QFileInfo& fi : rootScripts) {
                ResourceScript script = scanScriptWithAttachments(fi.absoluteFilePath(),
                                                                  ResourceType::Library,
                                                                  tier,
                                                                  loc.displayName);
                script.setCategory(libName);  // Tag with library for clarity
                tree->addChildResource(libNode, script);
            }

            // Examples
            const QString examplesPath = libDir.absoluteFilePath(QStringLiteral("examples"));
            if (QDir(examplesPath).exists()) {
                QVector<ResourceItem> examples = scanExamples(examplesPath, tier, loc.displayName);
                for (auto example : examples) {
                    example.setCategory(libName);
                    tree->addChildResource(libNode, example);
                }
            }

            // Tests
            const QString testsPath = libDir.absoluteFilePath(QStringLiteral("tests"));
            if (QDir(testsPath).exists()) {
                QVector<ResourceItem> tests = scanTests(testsPath, tier, loc.displayName);
                for (auto test : tests) {
                    test.setCategory(libName);
                    tree->addChildResource(libNode, test);
                }
            }

            // Templates bundled with the library (read-only)
            const QString templatesPath = libDir.absoluteFilePath(QStringLiteral("templates"));
            if (QDir(templatesPath).exists()) {
                QVector<ResourceItem> templates = scanTemplates(templatesPath, tier, loc.displayName);
                for (auto tmpl : templates) {
                    tmpl.setCategory(libName);
                    tmpl.setAccess(ResourceAccess::ReadOnly);
                    tree->addChildResource(libNode, tmpl);
                }
            }

            // Color schemes bundled with the library
            const QString colorSchemesPath = libDir.absoluteFilePath(QStringLiteral("color-schemes"));
            if (QDir(colorSchemesPath).exists()) {
                QVector<ResourceItem> colorSchemes = scanColorSchemes(colorSchemesPath, tier, loc.displayName);
                for (auto scheme : colorSchemes) {
                    scheme.setCategory(libName);
                    tree->addChildResource(libNode, scheme);
                }
            }
        }
    }
}

QVector<ResourceItem> ResourceScanner::scanColorSchemes(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QVector<ResourceItem> results;
    
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

QVector<ResourceItem> ResourceScanner::scanRenderColors(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QVector<ResourceItem> results;
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

QVector<ResourceItem> ResourceScanner::scanEditorColors(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QVector<ResourceItem> results;
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

QVector<ResourceItem> ResourceScanner::scanFonts(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    // Fonts are supplied by the system; only bundled font is handled elsewhere.
    return scanNoOp(basePath, tier, locationKey);
}

QVector<ResourceItem> ResourceScanner::scanExamples(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QVector<ResourceItem> results;
    
    // One-level scan: top-level .scad files plus immediate subfolders
    QDir dir(basePath);
    if (!dir.exists()) return results;

    auto processDir = [&](const QString& path, const QString& category) {
        QDir d(path);
        QFileInfoList files = d.entryInfoList({QStringLiteral("*.scad")}, QDir::Files);
        for (const QFileInfo& fi : files) {
            ResourceScript script = scanScriptWithAttachments(fi.absoluteFilePath(),
                                                              ResourceType::Example,
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

QVector<ResourceItem> ResourceScanner::scanTemplates(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QVector<ResourceItem> results;
    QDir dir(basePath);
    
    if (!dir.exists()) return results;
    
    // Templates: subfolders define categories
    // First scan top-level templates (no category)
    // Support both .scad (legacy) and .json (modern VS Code snippet format) templates
    QStringList filters = {QStringLiteral("*.scad"), QStringLiteral("*.json")};
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fi : files) {
        ResourceItem item(fi.absoluteFilePath(), ResourceType::Template, tier);
        item.setName(fi.baseName());
        item.setDisplayName(fi.baseName());
        item.setSourcePath(fi.absoluteFilePath());
        item.setSourceLocationKey(locationKey);
        item.setAccess(ResourceAccess::Writable);  // Templates are writable
        results.append(item);
    }
    
    // Scan subfolders (each is a category)
    QStringList subfolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& subfolder : subfolders) {
        QString subPath = dir.absoluteFilePath(subfolder);
        
        scanFolderRecursive(subPath, {QStringLiteral(".scad"), QStringLiteral(".json")},
                           ResourceType::Template, tier, locationKey,
                           subfolder, results);
    }
    
    return results;
}

QVector<ResourceItem> ResourceScanner::scanTranslations(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    // Translations are handled elsewhere; discovery is intentionally disabled here.
    return scanNoOp(basePath, tier, locationKey);
}

QVector<ResourceItem> ResourceScanner::scanTests(
    const QString& basePath, ResourceTier tier, const QString& locationKey)
{
    QVector<ResourceItem> results;
    
    // One-level scan: top-level .scad files plus immediate subfolders
    QDir dir(basePath);
    if (!dir.exists()) return results;

    auto processDir = [&](const QString& path, const QString& category) {
        QDir d(path);
        QFileInfoList files = d.entryInfoList({QStringLiteral("*.scad")}, QDir::Files);
        for (const QFileInfo& fi : files) {
            ResourceScript script = scanScriptWithAttachments(fi.absoluteFilePath(),
                                                              ResourceType::Test,
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
    script.setAccess(type == ResourceType::Template ? ResourceAccess::Writable 
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
    QVector<ResourceItem>& results)
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

// ============================================================================
// ResourceInventoryManager
// ============================================================================

ResourceInventoryManager::ResourceInventoryManager(QObject* parent)
    : QObject(parent)
    , m_scanner(new ResourceScanner(this))
{
}

void ResourceInventoryManager::setInstallLocations(const QVector<platformInfo::ResourceLocation>& locs)
{
    m_installLocs = locs;
}

void ResourceInventoryManager::setMachineLocations(const QVector<platformInfo::ResourceLocation>& locs)
{
    m_machineLocs = locs;
}

void ResourceInventoryManager::setUserLocations(const QVector<platformInfo::ResourceLocation>& locs)
{
    m_userLocs = locs;
}

void ResourceInventoryManager::setLocationsFrom(const platformInfo::ResourceLocationManager& manager)
{
    m_installLocs = manager.findSiblingInstallations();
    m_machineLocs = manager.enabledMachineLocations();
    m_userLocs = manager.enabledUserLocations();
}

void ResourceInventoryManager::buildInventory(const platformInfo::ResourceLocationManager& manager)
{
    setLocationsFrom(manager);
    
    // Build inventories for all resource types
    buildInventory(ResourceType::Example);
    buildInventory(ResourceType::Library);
    buildInventory(ResourceType::Font);
    buildInventory(ResourceType::RenderColors);
    buildInventory(ResourceType::EditorColors);
    buildInventory(ResourceType::Template);
    buildInventory(ResourceType::Translation);
}

ResourceTreeWidget* ResourceInventoryManager::buildInventory(ResourceType type)
{
    // Remove existing inventory if any
    if (m_inventories.contains(type)) {
        delete m_inventories.take(type);
    }
    
    auto* tree = new ResourceTreeWidget();
    tree->setResourceType(type);
    
    if (type == ResourceType::Library) {
        // Libraries need special hierarchical scanning
        m_scanner->scanLibraries(m_installLocs, ResourceTier::Installation, tree);
        m_scanner->scanLibraries(m_machineLocs, ResourceTier::Machine, tree);
        m_scanner->scanLibraries(m_userLocs, ResourceTier::User, tree);
    } else {
        m_scanner->scanAllTiers(m_installLocs, m_machineLocs, m_userLocs, type, tree);
    }
    
    m_inventories.insert(type, tree);
    
    int count = tree->allItems().size();
    emit inventoryBuilt(type, count);
    
    return tree;
}

ResourceTreeWidget* ResourceInventoryManager::inventory(ResourceType type)
{
    if (!m_inventories.contains(type)) {
        return buildInventory(type);
    }
    return m_inventories.value(type);
}

void ResourceInventoryManager::refreshInventory(ResourceType type)
{
    buildInventory(type);
    emit inventoryRefreshed(type);
}

void ResourceInventoryManager::clearAll()
{
    qDeleteAll(m_inventories);
    m_inventories.clear();
}

int ResourceInventoryManager::itemCount(ResourceType type) const
{
    if (m_inventories.contains(type)) {
        return m_inventories.value(type)->allItems().size();
    }
    return 0;
}

QMap<ResourceType, int> ResourceInventoryManager::allCounts() const
{
    QMap<ResourceType, int> counts;
    for (auto it = m_inventories.constBegin(); it != m_inventories.constEnd(); ++it) {
        counts.insert(it.key(), it.value()->allItems().size());
    }
    return counts;
}

QString ResourceInventoryManager::countSummary() const
{
    QStringList parts;
    
    auto addCount = [&](ResourceType type, const QString& label) {
        int count = itemCount(type);
        if (count > 0) {
            parts << QString("%1 %2").arg(count).arg(label);
        }
    };
    
    addCount(ResourceType::Example, "Examples");
    addCount(ResourceType::Library, "Libraries");
    addCount(ResourceType::Font, "Fonts");
    addCount(ResourceType::RenderColors, "Render Color Schemes");
    addCount(ResourceType::EditorColors, "Editor Color Schemes");
    addCount(ResourceType::Template, "Templates");
    addCount(ResourceType::Translation, "Translations");
    
    if (parts.isEmpty()) {
        return QStringLiteral("No resources found");
    }
    
    return parts.join(QStringLiteral(", "));
}

} // namespace resInventory
