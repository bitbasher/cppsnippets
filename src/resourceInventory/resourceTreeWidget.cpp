#include "resourceTreeWidget.hpp"
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

namespace resourceInventory {

// ============================================================================
// ResourceTreeItem
// ============================================================================

ResourceTreeItem::ResourceTreeItem(QTreeWidget* parent)
    : QTreeWidgetItem(parent)
{
}

ResourceTreeItem::ResourceTreeItem(QTreeWidgetItem* parent)
    : QTreeWidgetItem(parent)
{
}

ResourceTreeItem::ResourceTreeItem(QTreeWidget* parent, const ResourceItem& item)
    : QTreeWidgetItem(parent)
    , m_item(item)
{
    updateDisplay();
}

ResourceTreeItem::ResourceTreeItem(QTreeWidgetItem* parent, const ResourceItem& item)
    : QTreeWidgetItem(parent)
    , m_item(item)
{
    updateDisplay();
}

void ResourceTreeItem::setResourceItem(const ResourceItem& item)
{
    m_item = item;
    updateDisplay();
}

void ResourceTreeItem::setEnabled(bool enabled)
{
    m_item.setEnabled(enabled);
    updateDisplay();
}

QString ResourceTreeItem::shortenedLocation(const QString& fullPath, ResourceTier tier)
{
    QString path = QDir::toNativeSeparators(fullPath);
    
    if (tier == ResourceTier::User) {
        // Remove home directory prefix
        QString homePath = QDir::toNativeSeparators(QDir::homePath());
        if (path.startsWith(homePath)) {
            path.remove(0, homePath.length());
            if (path.startsWith(QDir::separator())) {
                path.remove(0, 1);
            }
        }
        
        // Remove /OpenSCAD/templates/ and filename
        // Path format: AppData/Local/OpenSCAD/templates/filename.ext
        // Want: AppData/Local
        int templatesIdx = path.indexOf(QStringLiteral("/OpenSCAD/templates/"), Qt::CaseInsensitive);
        if (templatesIdx < 0) {
            templatesIdx = path.indexOf(QStringLiteral("\\OpenSCAD\\templates\\"), Qt::CaseInsensitive);
        }
        if (templatesIdx >= 0) {
            return path.left(templatesIdx);
        }
        
        // Fallback: just remove filename
        return QFileInfo(path).path();
    }
    else if (tier == ResourceTier::Installation) {
        // For installation, show relative to application directory
        QString appDir = QCoreApplication::applicationDirPath();
        if (path.startsWith(appDir)) {
            path.remove(0, appDir.length());
            if (path.startsWith(QDir::separator())) {
                path.remove(0, 1);
            }
        }
        return QFileInfo(path).path();
    }
    else if (tier == ResourceTier::Machine) {
        // For machine, show directory name only
        return QFileInfo(path).path();
    }
    
    return path;
}

void ResourceTreeItem::updateDisplay()
{
    setText(ColTier, resourceTierToString(m_item.tier()));
    setText(ColName, m_item.displayName());
    setText(ColCategory, m_item.category());
    setText(ColPath, shortenedLocation(m_item.path(), m_item.tier()));
    
    // Set tooltip with full info
    setToolTip(ColName, m_item.description().isEmpty() 
                        ? m_item.path() 
                        : m_item.description());
    
    // Visual indicators based on state
    if (!m_item.exists()) {
        setForeground(ColName, QBrush(Qt::gray));
        setForeground(ColPath, QBrush(Qt::gray));
    } else {
        setForeground(ColName, QBrush());
        setForeground(ColPath, QBrush());
        
        // Show read-only items in italic
        if (m_item.access() == ResourceAccess::ReadOnly) {
            QFont font = this->font(ColName);
            font.setItalic(true);
            setFont(ColName, font);
        } else {
            QFont font = this->font(ColName);
            font.setItalic(false);
            setFont(ColName, font);
        }
    }
    
    // Checkbox for enabling/disabling
    if (m_item.exists()) {
        setFlags(flags() | Qt::ItemIsUserCheckable);
        setCheckState(ColName, m_item.isEnabled() ? Qt::Checked : Qt::Unchecked);
    } else {
        setFlags(flags() & ~Qt::ItemIsUserCheckable);
    }
}

// ============================================================================
// ResourceTreeWidget
// ============================================================================

ResourceTreeWidget::ResourceTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setColumnCount(4);
    setHeaderLabels({tr("Tier"), tr("Name"), tr("Category"), tr("Location")});
    
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(ResourceTreeItem::ColTier, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(ResourceTreeItem::ColName, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(ResourceTreeItem::ColCategory, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(ResourceTreeItem::ColPath, QHeaderView::Stretch);
    
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
    
    connect(this, &QTreeWidget::itemChanged, this, &ResourceTreeWidget::onItemChanged);
    connect(this, &QTreeWidget::itemSelectionChanged, this, &ResourceTreeWidget::onSelectionChanged);
}

ResourceTreeItem* ResourceTreeWidget::addResource(const ResourceItem& item)
{
    if (m_showCategories && !item.category().isEmpty()) {
        return addResourceToCategory(item, item.category());
    }
    
    auto* treeItem = new ResourceTreeItem(this, item);
    return treeItem;
}

ResourceTreeItem* ResourceTreeWidget::addResourceToCategory(const ResourceItem& item, const QString& category)
{
    ResourceTreeItem* categoryNode = findCategoryNode(category, true);
    if (categoryNode) {
        ResourceItem catItem = categoryNode->resourceItem();
        catItem.setTier(item.tier());
        categoryNode->setResourceItem(catItem);
    }
    return addChildResource(categoryNode, item);
}

ResourceTreeItem* ResourceTreeWidget::addChildResource(ResourceTreeItem* parent, const ResourceItem& item)
{
    if (!parent) {
        return addResource(item);
    }
    
    auto* treeItem = new ResourceTreeItem(parent, item);
    parent->setExpanded(true);
    return treeItem;
}

ResourceTreeItem* ResourceTreeWidget::findCategoryNode(const QString& category, bool autoCreate)
{
    // Look for existing category node
    for (int i = 0; i < topLevelItemCount(); ++i) {
        auto* item = dynamic_cast<ResourceTreeItem*>(topLevelItem(i));
        if (item && item->text(ResourceTreeItem::ColName) == category 
            && item->path().isEmpty()) {  // Category nodes have no path
            return item;
        }
    }
    
    if (!autoCreate) {
        return nullptr;
    }
    
    // Create category node
    ResourceItem categoryItem;
    categoryItem.setName(category);
    categoryItem.setDisplayName(category);
    categoryItem.setCategory(category);
    categoryItem.setExists(true);
    categoryItem.setEnabled(true);
    
    auto* categoryNode = new ResourceTreeItem(this, categoryItem);
    categoryNode->setFlags(categoryNode->flags() & ~Qt::ItemIsUserCheckable);  // No checkbox for categories
    return categoryNode;
}

ResourceTreeItem* ResourceTreeWidget::findItemByPath(const QString& path) const
{
    auto results = findItemsRecursive(nullptr, [&path](ResourceTreeItem* item) {
        return item->path() == path;
    });
    return results.isEmpty() ? nullptr : results.first();
}

ResourceTreeItem* ResourceTreeWidget::findItemByName(const QString& name) const
{
    auto results = findItemsRecursive(nullptr, [&name](ResourceTreeItem* item) {
        return item->name() == name;
    });
    return results.isEmpty() ? nullptr : results.first();
}

QList<ResourceTreeItem*> ResourceTreeWidget::findItemsByType(ResourceType type) const
{
    return findItemsRecursive(nullptr, [type](ResourceTreeItem* item) {
        return item->type() == type;
    });
}

QList<ResourceTreeItem*> ResourceTreeWidget::findItemsByTier(ResourceTier tier) const
{
    return findItemsRecursive(nullptr, [tier](ResourceTreeItem* item) {
        return item->tier() == tier;
    });
}

void ResourceTreeWidget::removeItem(ResourceTreeItem* item)
{
    if (!item) return;
    
    QTreeWidgetItem* parent = item->parent();
    if (parent) {
        parent->removeChild(item);
    } else {
        int index = indexOfTopLevelItem(item);
        if (index >= 0) {
            takeTopLevelItem(index);
        }
    }
    delete item;
}

void ResourceTreeWidget::removeItemsByTier(ResourceTier tier)
{
    auto items = findItemsByTier(tier);
    for (auto* item : items) {
        removeItem(item);
    }
}

QList<ResourceItem> ResourceTreeWidget::allItems() const
{
    QList<ResourceItem> items;
    collectItems(nullptr, items, false);
    return items;
}

QList<ResourceItem> ResourceTreeWidget::enabledItems() const
{
    QList<ResourceItem> items;
    collectItems(nullptr, items, true);
    return items;
}

ResourceItem ResourceTreeWidget::selectedItem() const
{
    auto selected = QTreeWidget::selectedItems();
    if (selected.isEmpty()) {
        return ResourceItem();
    }
    auto* treeItem = dynamic_cast<ResourceTreeItem*>(selected.first());
    return treeItem ? treeItem->resourceItem() : ResourceItem();
}

QList<ResourceItem> ResourceTreeWidget::selectedResourceItems() const
{
    QList<ResourceItem> items;
    for (auto* item : QTreeWidget::selectedItems()) {
        auto* treeItem = dynamic_cast<ResourceTreeItem*>(item);
        if (treeItem && !treeItem->path().isEmpty()) {  // Skip category nodes
            items.append(treeItem->resourceItem());
        }
    }
    return items;
}

void ResourceTreeWidget::setAllEnabled(bool enabled)
{
    auto items = findItemsRecursive(nullptr, [](ResourceTreeItem*) { return true; });
    for (auto* item : items) {
        if (item->exists()) {
            item->setEnabled(enabled);
        }
    }
}

void ResourceTreeWidget::setTierEnabled(ResourceTier tier, bool enabled)
{
    auto items = findItemsByTier(tier);
    for (auto* item : items) {
        if (item->exists()) {
            item->setEnabled(enabled);
        }
    }
}

void ResourceTreeWidget::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (column != ResourceTreeItem::ColName) return;
    
    auto* treeItem = dynamic_cast<ResourceTreeItem*>(item);
    if (!treeItem || treeItem->path().isEmpty()) return;  // Skip category nodes
    
    bool enabled = (treeItem->checkState(ResourceTreeItem::ColName) == Qt::Checked);
    if (treeItem->isEnabled() != enabled) {
        treeItem->setEnabled(enabled);
        emit itemEnabledChanged(treeItem->resourceItem(), enabled);
    }
}

void ResourceTreeWidget::onSelectionChanged()
{
    auto item = selectedItem();
    if (item.isValid()) {
        emit itemSelected(item);
    }
}

void ResourceTreeWidget::collectItems(QTreeWidgetItem* parent, QList<ResourceItem>& list, bool enabledOnly) const
{
    int count = parent ? parent->childCount() : topLevelItemCount();
    
    for (int i = 0; i < count; ++i) {
        QTreeWidgetItem* child = parent ? parent->child(i) : topLevelItem(i);
        auto* treeItem = dynamic_cast<ResourceTreeItem*>(child);
        
        if (treeItem && !treeItem->path().isEmpty()) {  // Skip category nodes
            if (!enabledOnly || treeItem->isEnabled()) {
                list.append(treeItem->resourceItem());
            }
        }
        
        // Recurse into children
        collectItems(child, list, enabledOnly);
    }
}

QList<ResourceTreeItem*> ResourceTreeWidget::findItemsRecursive(
    QTreeWidgetItem* parent, 
    std::function<bool(ResourceTreeItem*)> predicate) const
{
    QList<ResourceTreeItem*> results;
    int count = parent ? parent->childCount() : topLevelItemCount();
    
    for (int i = 0; i < count; ++i) {
        QTreeWidgetItem* child = parent ? parent->child(i) : topLevelItem(i);
        auto* treeItem = dynamic_cast<ResourceTreeItem*>(child);
        
        if (treeItem && predicate(treeItem)) {
            results.append(treeItem);
        }
        
        // Recurse into children
        results.append(findItemsRecursive(child, predicate));
    }
    
    return results;
}

} // namespace resourceInventory
