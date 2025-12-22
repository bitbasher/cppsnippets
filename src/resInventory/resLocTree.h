#ifndef RESLOCTREE_H
#define RESLOCTREE_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QString>
#include "platformInfo/export.h"
#include "resInventory/ResourceLocation.h"

/**
 * @brief Custom tree widget item that holds a ResourceLocation
 * 
 * Extends QTreeWidgetItem to store ResourceLocation data alongside
 * the standard tree item properties.
 */
class PLATFORMINFO_API ResLocTreeItem : public QTreeWidgetItem {
public:
    // Column indices
    enum Column {
        ColName = 0,
        ColPath = 1,
        ColDescription = 2
    };
    
    explicit ResLocTreeItem(QTreeWidget* parent = nullptr);
    explicit ResLocTreeItem(QTreeWidgetItem* parent);
    ResLocTreeItem(QTreeWidget* parent, const platformInfo::ResourceLocation& loc);
    ResLocTreeItem(QTreeWidgetItem* parent, const platformInfo::ResourceLocation& loc);
    
    void setLocation(const platformInfo::ResourceLocation& loc);
    platformInfo::ResourceLocation location() const;
    
    // Convenience accessors
    QString name() const;
    QString path() const;
    
private:
    platformInfo::ResourceLocation m_location;
    void updateDisplay();
};

/**
 * @brief Tree widget for hierarchical resource locations
 * 
 * Uses Qt's QTreeWidget/QTreeWidgetItem pattern to display resources
 * in a tree structure. Suitable for libraries with examples, tests,
 * and other nested resources.
 */
class PLATFORMINFO_API ResLocTree : public QTreeWidget {
    Q_OBJECT

public:
    explicit ResLocTree(QWidget* parent = nullptr);
    
    // Add items
    ResLocTreeItem* addTopLevelLocation(const platformInfo::ResourceLocation& loc);
    ResLocTreeItem* addChildLocation(ResLocTreeItem* parent, const platformInfo::ResourceLocation& loc);
    
    // Find items
    ResLocTreeItem* findItemByPath(const QString& path) const;
    ResLocTreeItem* findItemByName(const QString& name) const;
    
    // Remove items
    void removeItem(ResLocTreeItem* item);
    
    // Get all locations (flattened)
    QList<platformInfo::ResourceLocation> allLocations() const;
    
    // Get selected location
    platformInfo::ResourceLocation selectedLocation() const;

private:
    void collectLocations(QTreeWidgetItem* item, QList<platformInfo::ResourceLocation>& list) const;
};

#endif // RESLOCTREE_H
