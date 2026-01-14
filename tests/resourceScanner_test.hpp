/**
 * @file resourceScanner_test.hpp
 * @brief Minimal ResourceScanner header for Phase 1 callback testing
 * 
 * No Q_OBJECT, no signals, no GUI - just the callback API
 */

#ifndef RESOURCESCANNER_TEST_H
#define RESOURCESCANNER_TEST_H

#include <QString>
#include <QList>
#include <functional>
#include "resourceInventory/resourceItem.hpp"
#include "platformInfo/ResourceLocation.hpp"

class QStandardItemModel;

namespace platformInfo {
class ResourceLocationManager;
}

namespace resourceInventory {

class ResourceTreeWidget;  // Forward declaration (not used)

/**
 * @brief Minimal ResourceScanner for testing callback API
 */
class ResourceScanner {
public:
    // Callback for streaming resource items as they're discovered
    using ItemCallback = std::function<void(const ResourceItem&)>;
    
    ResourceScanner();
    
    // ========================================================================
    // NEW CALLBACK-BASED API (Phase 1)
    // ========================================================================
    
    void scanTemplates(const QString& basePath,
                      ResourceTier tier,
                      const QString& locationKey,
                      ItemCallback onItemFound);
    
    QList<ResourceItem> scanTemplatesToList(const QString& basePath,
                                              ResourceTier tier,
                                              const QString& locationKey);
    
    void scanTemplatesToModel(const QString& basePath,
                             ResourceTier tier,
                             const QString& locationKey,
                             QStandardItemModel* model);
    
    void scanExamples(const QString& basePath,
                     ResourceTier tier,
                     const QString& locationKey,
                     ItemCallback onItemFound);
    
    QList<ResourceItem> scanExamplesToList(const QString& basePath,
                                            ResourceTier tier,
                                            const QString& locationKey);
    
    void scanExamplesToModel(const QString& basePath,
                            ResourceTier tier,
                            const QString& locationKey,
                            QStandardItemModel* model);
    
    /**
     * @brief Scan all resource locations and types, populate model (Phase 2)
     */
    void scanToModel(QStandardItemModel* model,
                     const QList<platformInfo::ResourceLocation>& locations);
    
    // ========================================================================
    // LEGACY API (stubbed out for tests)
    // ========================================================================
    
    QList<ResourceItem> scanLocation(const platformInfo::ResourceLocation& location,
                                        ResourceType type,
                                        ResourceTier tier);
    
    void scanToTree(const QList<platformInfo::ResourceLocation>& locations,
                    ResourceType type,
                    ResourceTier tier,
                    ResourceTreeWidget* tree);
    
    void scanAllTiers(const QList<platformInfo::ResourceLocation>& installLocs,
                      const QList<platformInfo::ResourceLocation>& machineLocs,
                      const QList<platformInfo::ResourceLocation>& userLocs,
                      ResourceType type,
                      ResourceTreeWidget* tree);
    
    void scanLibraries(const QList<platformInfo::ResourceLocation>& locations,
                       ResourceTier tier,
                       ResourceTreeWidget* tree);
    
    static QString resourceSubfolder(ResourceType type);
    static QStringList resourceExtensions(ResourceType type);

private:
    void addItemToModel(QStandardItemModel* model, const ResourceItem& item);
    ResourceScript scanScriptWithAttachments(const QString& scriptPath,
                                             ResourceType type,
                                             ResourceTier tier,
                                             const QString& locationKey);
    void scanGroup(const QString& groupPath,
                   ResourceTier tier,
                   const QString& locationKey,
                   const QString& category,
                   ItemCallback onItemFound);
};

} // namespace resourceInventory

#endif // RESOURCESCANNER_TEST_H
