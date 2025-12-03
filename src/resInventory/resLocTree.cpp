#include "resInventory/resLocTree.h"

// ============================================================================
// ResLocTreeItem
// ============================================================================

ResLocTreeItem::ResLocTreeItem(QTreeWidget* parent)
    : QTreeWidgetItem(parent)
{
}

ResLocTreeItem::ResLocTreeItem(QTreeWidgetItem* parent)
    : QTreeWidgetItem(parent)
{
}

ResLocTreeItem::ResLocTreeItem(QTreeWidget* parent, const platformInfo::ResourceLocation& loc)
    : QTreeWidgetItem(parent)
    , m_location(loc)
{
    updateDisplay();
}

ResLocTreeItem::ResLocTreeItem(QTreeWidgetItem* parent, const platformInfo::ResourceLocation& loc)
    : QTreeWidgetItem(parent)
    , m_location(loc)
{
    updateDisplay();
}

void ResLocTreeItem::setLocation(const platformInfo::ResourceLocation& loc) {
    m_location = loc;
    updateDisplay();
}

platformInfo::ResourceLocation ResLocTreeItem::location() const {
    return m_location;
}

QString ResLocTreeItem::name() const {
    return m_location.displayName;
}

QString ResLocTreeItem::path() const {
    return m_location.path;
}

void ResLocTreeItem::updateDisplay() {
    setText(ColName, m_location.displayName.isEmpty() ? m_location.path : m_location.displayName);
    setText(ColPath, m_location.path);
    setText(ColDescription, m_location.description);
    
    // Visual indicators based on state
    if (!m_location.exists) {
        setForeground(ColName, Qt::gray);
        setToolTip(ColName, QObject::tr("Path does not exist"));
    } else if (!m_location.hasResourceFolders) {
        setForeground(ColName, Qt::darkGray);
        setToolTip(ColName, QObject::tr("Path exists but contains no resource folders"));
    } else if (!m_location.isWritable) {
        setToolTip(ColName, QObject::tr("Path is read-only"));
    }
    
    // Checkbox for enabled state if checkable
    if (m_location.exists && m_location.hasResourceFolders) {
        setFlags(flags() | Qt::ItemIsUserCheckable);
        setCheckState(ColName, m_location.isEnabled ? Qt::Checked : Qt::Unchecked);
    }
}

// ============================================================================
// ResLocTree
// ============================================================================

ResLocTree::ResLocTree(QWidget* parent)
    : QTreeWidget(parent)
{
    setColumnCount(3);
    setHeaderLabels({tr("Name"), tr("Path"), tr("Description")});
    setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Reasonable column widths
    setColumnWidth(ResLocTreeItem::ColName, 200);
    setColumnWidth(ResLocTreeItem::ColPath, 300);
    setColumnWidth(ResLocTreeItem::ColDescription, 200);
}

ResLocTreeItem* ResLocTree::addTopLevelLocation(const platformInfo::ResourceLocation& loc) {
    ResLocTreeItem* item = new ResLocTreeItem(this, loc);
    return item;
}

ResLocTreeItem* ResLocTree::addChildLocation(ResLocTreeItem* parent, const platformInfo::ResourceLocation& loc) {
    if (!parent) return nullptr;
    ResLocTreeItem* item = new ResLocTreeItem(parent, loc);
    return item;
}

ResLocTreeItem* ResLocTree::findItemByPath(const QString& path) const {
    QTreeWidgetItemIterator it(const_cast<ResLocTree*>(this));
    while (*it) {
        ResLocTreeItem* item = dynamic_cast<ResLocTreeItem*>(*it);
        if (item && item->path() == path) {
            return item;
        }
        ++it;
    }
    return nullptr;
}

ResLocTreeItem* ResLocTree::findItemByName(const QString& name) const {
    QTreeWidgetItemIterator it(const_cast<ResLocTree*>(this));
    while (*it) {
        ResLocTreeItem* item = dynamic_cast<ResLocTreeItem*>(*it);
        if (item && item->name() == name) {
            return item;
        }
        ++it;
    }
    return nullptr;
}

void ResLocTree::removeItem(ResLocTreeItem* item) {
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

QList<platformInfo::ResourceLocation> ResLocTree::allLocations() const {
    QList<platformInfo::ResourceLocation> list;
    for (int i = 0; i < topLevelItemCount(); ++i) {
        collectLocations(topLevelItem(i), list);
    }
    return list;
}

void ResLocTree::collectLocations(QTreeWidgetItem* item, QList<platformInfo::ResourceLocation>& list) const {
    ResLocTreeItem* resItem = dynamic_cast<ResLocTreeItem*>(item);
    if (resItem) {
        list.append(resItem->location());
    }
    for (int i = 0; i < item->childCount(); ++i) {
        collectLocations(item->child(i), list);
    }
}

platformInfo::ResourceLocation ResLocTree::selectedLocation() const {
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (selected.isEmpty()) {
        return platformInfo::ResourceLocation();
    }
    ResLocTreeItem* item = dynamic_cast<ResLocTreeItem*>(selected.first());
    return item ? item->location() : platformInfo::ResourceLocation();
}
