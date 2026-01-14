#ifndef RESOURCESCANNER_H
#define RESOURCESCANNER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <functional>
#include "resourceInventory/resourceItem.hpp"
#include "platformInfo/ResourceLocation.hpp"
#include "export.hpp"

class QStandardItemModel;

namespace resourceInventory {

/**
 * @brief Scans resource locations and builds inventories
 * 
 * Scans file system locations for resources of various types and
 * populates tree widgets or vectors with discovered items.
 * 
 * Folder structure conventions (OpenSCAD):
 * - color-schemes/   -> Color scheme JSON files
 * - examples/        -> Example .scad scripts (with optional attachments)
 * - fonts/           -> Font files (.ttf, .otf)
 * - libraries/       -> Library folders (each containing .scad files)
 * - templates/       -> Template .scad files (subfolders = categories)
 * - locale/          -> Translation files
 */
class RESOURCESCANNING_API ResourceScanner : public QObject {
    Q_OBJECT

public:
    // Callback for streaming resource items as they're discovered
    using ItemCallback = std::function<void(const ResourceItem&)>;
    
    explicit ResourceScanner(QObject* parent = nullptr);
    
    /**
     * @brief Scan a single location for a specific resource type
     * @param location The ResourceLocation to scan
     * @param type The type of resource to look for
     * @param tier The tier this location belongs to
     * @return Vector of discovered ResourceItems
     */
    QList<ResourceItem> scanLocation(const platformInfo::ResourceLocation& location,
                                        ResourceType type,
                                        ResourceTier tier);
    
    // ========================================================================
    // NEW CALLBACK-BASED API (Phase 1)
    // ========================================================================
    
    /**
     * @brief Scan templates with callback for each found item (streaming)
     * @param basePath The folder to scan
     * @param tier The resource tier
     * @param locationKey Display name of the location
     * @param onItemFound Callback invoked for each discovered template
     */
    void scanTemplates(const QString& basePath,
                      ResourceTier tier,
                      const QString& locationKey,
                      ItemCallback onItemFound);
    
    /**
     * @brief Scan examples with callback for each found item (streaming)
     * @param basePath The folder to scan (e.g., "/usr/share/openscad/examples")
     * @param tier The resource tier
     * @param locationKey Display name of the location
     * @param onItemFound Callback invoked for each discovered example
     * 
     * Scans .scad files with attachments. Structure:
     * - Top-level .scad files (no category)
     * - One level of subfolders (subfolder name = category)
     */
    void scanExamples(const QString& basePath,
                     ResourceTier tier,
                     const QString& locationKey,
                     ItemCallback onItemFound);
    
    /**
     * @brief Scan templates and capture to list (for testing)
     * @param basePath The folder to scan
     * @param tier The resource tier
     * @param locationKey Display name of the location
     * @return Vector of discovered templates
     */
    QList<ResourceItem> scanTemplatesToList(const QString& basePath,
                                              ResourceTier tier,
                                              const QString& locationKey);
    
    /**
     * @brief Scan templates and populate model directly (for production)
     * @param basePath The folder to scan
     * @param tier The resource tier
     * @param locationKey Display name of the location
     * @param model The model to populate
     */
    void scanTemplatesToModel(const QString& basePath,
                             ResourceTier tier,
                             const QString& locationKey,
                             QStandardItemModel* model);
    
    /**
     * @brief Scan examples and capture to list (for testing)
     * @param basePath The folder to scan
     * @param tier The resource tier
     * @param locationKey Display name of the location
     * @return Vector of discovered examples
     */
    QList<ResourceItem> scanExamplesToList(const QString& basePath,
                                            ResourceTier tier,
                                            const QString& locationKey);
    
    /**
     * @brief Scan examples and populate model directly (for production)
     * @param basePath The folder to scan
     * @param tier The resource tier
     * @param locationKey Display name of the location
     * @param model The model to populate
     */
    void scanExamplesToModel(const QString& basePath,
                            ResourceTier tier,
                            const QString& locationKey,
                            QStandardItemModel* model);
    
    /**
     * @brief Scan all resource locations and types, populate model (Phase 2)
     * @param model The model to populate with all discovered resources
     * @param locations All resource locations to scan (tier is in each location)
     * 
     * High-level method that iterates all locations and resource types,
     * streaming items directly to the model using callback pattern.
     */
    void scanToModel(QStandardItemModel* model,
                     const QList<platformInfo::ResourceLocation>& locations);
    
    // ========================================================================
    // LEGACY API (to be removed in Phase 5)
    // ========================================================================
    
    /**
     * @brief Get subfolder name for a resource type
     * @param type The resource type
     * @return Folder name (e.g., "color-schemes", "libraries")
     */
    static QString resourceSubfolder(ResourceType type);
    
    /**
     * @brief Get file extensions for a resource type
     * @param type The resource type
     * @return List of extensions (e.g., {".scad"}, {".ttf", ".otf"})
     */
    static QStringList resourceExtensions(ResourceType type);
    
signals:
    void scanStarted(ResourceType type, int locationCount);
    void locationScanned(const QString& path, int itemCount);
    void scanCompleted(ResourceType type, int totalItems);
    void scanError(const QString& message);

private:
    // Helper to add item to QStandardItemModel with custom roles
    void addItemToModel(QStandardItemModel* model, const ResourceItem& item);
    
    // Scanning methods for specific types (LEGACY - returns vectors)
    QList<ResourceItem> scanColorSchemes(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QList<ResourceItem> scanRenderColors(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QList<ResourceItem> scanEditorColors(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QList<ResourceItem> scanFonts(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QList<ResourceItem> scanExamples(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QList<ResourceItem> scanTests(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QList<ResourceItem> scanTranslations(const QString& basePath, ResourceTier tier, const QString& locationKey);
    
    // Helper for scanning script files with attachments
    ResourceScript scanScriptWithAttachments(const QString& scriptPath, 
                                              ResourceType type,
                                              ResourceTier tier, 
                                              const QString& locationKey);
    
    // Helper for scanning a Group folder (category with .scad files, no recursion)
    void scanGroup(const QString& groupPath,
                   ResourceTier tier,
                   const QString& locationKey,
                   const QString& category,
                   ItemCallback onItemFound);
    
    // Helper for recursive folder scanning
    void scanFolderRecursive(const QString& folderPath,
                             const QStringList& extensions,
                             ResourceType type,
                             ResourceTier tier,
                             const QString& locationKey,
                             const QString& category,
                             QList<ResourceItem>& results);
};

} // namespace resourceInventory

#endif // RESOURCESCANNER_H
