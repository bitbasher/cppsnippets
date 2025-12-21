#ifndef RESOURCESCANNER_H
#define RESOURCESCANNER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <functional>
#include "resInventory/resourceItem.h"
#include "platformInfo/ResourceLocation.h"

namespace platformInfo {
class ResourceLocationManager;
}

namespace resInventory {

class ResourceTreeWidget;

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
class ResourceScanner : public QObject {
    Q_OBJECT

public:
    explicit ResourceScanner(QObject* parent = nullptr);
    
    /**
     * @brief Scan a single location for a specific resource type
     * @param location The ResourceLocation to scan
     * @param type The type of resource to look for
     * @param tier The tier this location belongs to
     * @return Vector of discovered ResourceItems
     */
    QVector<ResourceItem> scanLocation(const platformInfo::ResourceLocation& location,
                                        ResourceType type,
                                        ResourceTier tier);
    
    /**
     * @brief Scan multiple locations and populate a tree widget
     * @param locations Vector of locations to scan
     * @param type The type of resource to look for
     * @param tier The tier these locations belong to
     * @param tree The tree widget to populate
     */
    void scanToTree(const QVector<platformInfo::ResourceLocation>& locations,
                    ResourceType type,
                    ResourceTier tier,
                    ResourceTreeWidget* tree);
    
    /**
     * @brief Scan all tiers for a resource type
     * @param installLocs Installation tier locations
     * @param machineLocs Machine tier locations  
     * @param userLocs User tier locations
     * @param type The type of resource to look for
     * @param tree The tree widget to populate
     */
    void scanAllTiers(const QVector<platformInfo::ResourceLocation>& installLocs,
                      const QVector<platformInfo::ResourceLocation>& machineLocs,
                      const QVector<platformInfo::ResourceLocation>& userLocs,
                      ResourceType type,
                      ResourceTreeWidget* tree);
    
    /**
     * @brief Scan for libraries (hierarchical with nested examples/tests)
     * @param locations Vector of locations to scan
     * @param tier The tier these locations belong to
     * @param tree The tree widget to populate
     */
    void scanLibraries(const QVector<platformInfo::ResourceLocation>& locations,
                       ResourceTier tier,
                       ResourceTreeWidget* tree);
    
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
    // Scanning methods for specific types
    QVector<ResourceItem> scanColorSchemes(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QVector<ResourceItem> scanRenderColors(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QVector<ResourceItem> scanEditorColors(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QVector<ResourceItem> scanFonts(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QVector<ResourceItem> scanExamples(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QVector<ResourceItem> scanTests(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QVector<ResourceItem> scanTemplates(const QString& basePath, ResourceTier tier, const QString& locationKey);
    QVector<ResourceItem> scanTranslations(const QString& basePath, ResourceTier tier, const QString& locationKey);
    
    // Helper for scanning script files with attachments
    ResourceScript scanScriptWithAttachments(const QString& scriptPath, 
                                              ResourceType type,
                                              ResourceTier tier, 
                                              const QString& locationKey);
    
    // Helper for recursive folder scanning
    void scanFolderRecursive(const QString& folderPath,
                             const QStringList& extensions,
                             ResourceType type,
                             ResourceTier tier,
                             const QString& locationKey,
                             const QString& category,
                             QVector<ResourceItem>& results);
};

/**
 * @brief Manages inventory scanning and storage for all resource types
 * 
 * High-level manager that coordinates scanning across all tiers
 * and maintains separate inventories for each resource type.
 */
class ResourceInventoryManager : public QObject {
    Q_OBJECT

public:
    explicit ResourceInventoryManager(QObject* parent = nullptr);
    
    /**
     * @brief Set the locations for each tier
     */
    void setInstallLocations(const QVector<platformInfo::ResourceLocation>& locs);
    void setMachineLocations(const QVector<platformInfo::ResourceLocation>& locs);
    void setUserLocations(const QVector<platformInfo::ResourceLocation>& locs);
    
    /**
     * @brief Initialize locations from a ResourceLocationManager
     * @param manager The resource location manager
     */
    void setLocationsFrom(const platformInfo::ResourceLocationManager& manager);
    
    /**
     * @brief Build all inventories from a ResourceLocationManager
     * @param manager The resource location manager
     * 
     * Convenience method that sets locations and builds inventories
     * for all resource types.
     */
    void buildInventory(const platformInfo::ResourceLocationManager& manager);
    
    /**
     * @brief Scan and build inventory for a specific resource type
     * @param type The resource type to scan
     * @return Tree widget containing discovered resources
     */
    ResourceTreeWidget* buildInventory(ResourceType type);
    
    /**
     * @brief Get inventory for a resource type (returns existing or scans)
     * @param type The resource type
     * @return Tree widget containing resources
     */
    ResourceTreeWidget* inventory(ResourceType type);
    
    /**
     * @brief Refresh inventory for a resource type
     * @param type The resource type to rescan
     */
    void refreshInventory(ResourceType type);
    
    /**
     * @brief Clear all inventories
     */
    void clearAll();
    
    /**
     * @brief Get count of items in a specific inventory
     * @param type The resource type
     * @return Number of items, or 0 if inventory not built
     */
    int itemCount(ResourceType type) const;
    
    /**
     * @brief Get summary counts for all resource types
     * @return Map of resource type to item count
     */
    QMap<ResourceType, int> allCounts() const;
    
    /**
     * @brief Get a formatted string with all resource counts
     * @return Human-readable summary like "111 Examples, 56 Libraries"
     */
    QString countSummary() const;
    
signals:
    void inventoryBuilt(ResourceType type, int itemCount);
    void inventoryRefreshed(ResourceType type);

private:
    ResourceScanner* m_scanner;
    QVector<platformInfo::ResourceLocation> m_installLocs;
    QVector<platformInfo::ResourceLocation> m_machineLocs;
    QVector<platformInfo::ResourceLocation> m_userLocs;
    
    QMap<ResourceType, ResourceTreeWidget*> m_inventories;
};

} // namespace resInventory

#endif // RESOURCESCANNER_H
