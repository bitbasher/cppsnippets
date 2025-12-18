#ifndef RESOURCETREEITEM_H
#define RESOURCETREEITEM_H

#include <QTreeWidgetItem>
#include "resInventory/resourceItem.h"

namespace resInventory {

/**
 * @brief Tree widget item that holds a ResourceItem
 * 
 * Extends QTreeWidgetItem to store ResourceItem data alongside
 * the standard tree item properties. Used for hierarchical display
 * of libraries, examples, and other nested resources.
 */
class ResourceTreeItem : public QTreeWidgetItem {
public:
    // Column indices
    enum Column {
        ColTier = 0,
        ColName = 1,
        ColCategory = 2,
        ColPath = 3
    };
    
    explicit ResourceTreeItem(QTreeWidget* parent = nullptr);
    explicit ResourceTreeItem(QTreeWidgetItem* parent);
    ResourceTreeItem(QTreeWidget* parent, const ResourceItem& item);
    ResourceTreeItem(QTreeWidgetItem* parent, const ResourceItem& item);
    
    void setResourceItem(const ResourceItem& item);
    ResourceItem resourceItem() const { return m_item; }
    
    // Convenience accessors
    QString name() const { return m_item.name(); }
    QString path() const { return m_item.path(); }
    QString category() const { return m_item.category(); }
    ResourceType type() const { return m_item.type(); }
    ResourceTier tier() const { return m_item.tier(); }
    
    // State
    bool isEnabled() const { return m_item.isEnabled(); }
    void setEnabled(bool enabled);
    
    bool exists() const { return m_item.exists(); }
    
    // Helper to compute shortened location path for display
    static QString shortenedLocation(const QString& fullPath, ResourceTier tier);
    
private:
    ResourceItem m_item;
    void updateDisplay();
};

/**
 * @brief Tree widget for hierarchical resource display
 * 
 * Uses Qt's QTreeWidget/QTreeWidgetItem pattern to display resources
 * in a tree structure. Supports:
 * - Libraries with nested examples, tests, attachments
 * - Category grouping (for templates)
 * - Visual indicators for tier, state, and access
 */
class ResourceTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit ResourceTreeWidget(QWidget* parent = nullptr);
    
    // Configuration
    void setResourceType(ResourceType type) { m_resourceType = type; }
    ResourceType resourceType() const { return m_resourceType; }
    
    void setShowCategories(bool show) { m_showCategories = show; }
    bool showCategories() const { return m_showCategories; }
    
    // Add items
    ResourceTreeItem* addResource(const ResourceItem& item);
    ResourceTreeItem* addResourceToCategory(const ResourceItem& item, const QString& category);
    ResourceTreeItem* addChildResource(ResourceTreeItem* parent, const ResourceItem& item);
    
    // Find category node (creates if needed when autoCreate=true)
    ResourceTreeItem* findCategoryNode(const QString& category, bool autoCreate = false);
    
    // Find items
    ResourceTreeItem* findItemByPath(const QString& path) const;
    ResourceTreeItem* findItemByName(const QString& name) const;
    QList<ResourceTreeItem*> findItemsByType(ResourceType type) const;
    QList<ResourceTreeItem*> findItemsByTier(ResourceTier tier) const;
    
    // Remove items
    void removeItem(ResourceTreeItem* item);
    void removeItemsByTier(ResourceTier tier);
    
    // Get all items (flattened)
    QList<ResourceItem> allItems() const;
    QList<ResourceItem> enabledItems() const;
    
    // Get selected
    ResourceItem selectedItem() const;
    QList<ResourceItem> selectedResourceItems() const;
    
    // Batch operations
    void setAllEnabled(bool enabled);
    void setTierEnabled(ResourceTier tier, bool enabled);
    
signals:
    void itemEnabledChanged(const ResourceItem& item, bool enabled);
    void itemSelected(const ResourceItem& item);

private slots:
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onSelectionChanged();

private:
    ResourceType m_resourceType = ResourceType::Unknown;
    bool m_showCategories = false;
    
    void collectItems(QTreeWidgetItem* item, QList<ResourceItem>& list, bool enabledOnly) const;
    QList<ResourceTreeItem*> findItemsRecursive(QTreeWidgetItem* parent, 
                                                 std::function<bool(ResourceTreeItem*)> predicate) const;
};

} // namespace resInventory

#endif // RESOURCETREEITEM_H
