/**
 * @file resourceScanner_test.hpp
 * @brief Minimal ResourceScanner header for Phase 1 callback testing
 * 
 * No Q_OBJECT, no signals, no GUI - just the callback API
 */

#ifndef RESOURCESCANNER_TEST_H
#define RESOURCESCANNER_TEST_H

#include <QString>
#include <QVector>
#include <functional>
#include "resourceInventory/resourceItem.hpp"
#include "platformInfo/ResourceLocation.hpp"

class QStandardItemModel;

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
    
    QVector<ResourceItem> scanTemplatesToList(const QString& basePath,
                                              ResourceTier tier,
                                              const QString& locationKey);
    
    void scanTemplatesToModel(const QString& basePath,
                             ResourceTier tier,
                             const QString& locationKey,
                             QStandardItemModel* model);
    
    // ========================================================================
    // LEGACY API (stubbed out for tests)
    // ========================================================================
    
    QVector<ResourceItem> scanLocation(const platformInfo::ResourceLocation& location,
                                        ResourceType type,
                                        ResourceTier tier);
    
    void scanToTree(const QVector<platformInfo::ResourceLocation>& locations,
                    ResourceType type,
                    ResourceTier tier,
                    ResourceTreeWidget* tree);
    
    void scanAllTiers(const QVector<platformInfo::ResourceLocation>& installLocs,
                      const QVector<platformInfo::ResourceLocation>& machineLocs,
                      const QVector<platformInfo::ResourceLocation>& userLocs,
                      ResourceType type,
                      ResourceTreeWidget* tree);
    
    void scanLibraries(const QVector<platformInfo::ResourceLocation>& locations,
                       ResourceTier tier,
                       ResourceTreeWidget* tree);
    
    static QString resourceSubfolder(ResourceType type);
    static QStringList resourceExtensions(ResourceType type);

private:
    void addItemToModel(QStandardItemModel* model, const ResourceItem& item);
};

} // namespace resourceInventory

#endif // RESOURCESCANNER_TEST_H
