/**
 * @file templateTreeModel.h
 * @brief QAbstractItemModel for hierarchical template display
 * 
 * Provides a tree model for displaying templates organized by:
 *   Tier (Installation/Machine/User)
 *     └── Location or Library
 *         └── Template (leaf)
 * 
 * Works with ResourceStore as data source and responds to its signals.
 */

#ifndef TEMPLATETREEMODEL_H
#define TEMPLATETREEMODEL_H

#include <QAbstractItemModel>
#include <QVector>
#include <memory>
#include <functional>

#include "platformInfo/export.h"
#include "resInventory/resourceStore.h"

namespace resInventory {

/**
 * @brief Internal node for tree structure
 * 
 * Represents either:
 * - A tier node (Installation/Machine/User) - level 0
 * - A location/library node - level 1
 * - A template resource (leaf) - level 2
 */
class RESOURCEMGMT_API TemplateTreeNode
{
public:
    enum class NodeType {
        Root,       ///< Invisible root
        Tier,       ///< Installation/Machine/User
        Location,   ///< Search location path or library name
        Template    ///< Actual template resource (leaf)
    };
    
    explicit TemplateTreeNode(NodeType type, TemplateTreeNode* parent = nullptr);
    ~TemplateTreeNode();
    
    // Tree structure
    TemplateTreeNode* parent() const { return m_parent; }
    TemplateTreeNode* child(int row) const;
    int childCount() const { return m_children.size(); }
    int row() const;
    
    void appendChild(TemplateTreeNode* child);
    void removeChild(int row);
    void clearChildren();
    void sortChildren(const std::function<bool(const TemplateTreeNode*, const TemplateTreeNode*)>& less);
    
    // Node data
    NodeType nodeType() const { return m_nodeType; }
    
    // For Tier nodes
    void setTier(ResourceTier tier) { m_tier = tier; }
    ResourceTier tier() const { return m_tier; }
    
    // For Location nodes
    void setLocationKey(const QString& key) { m_locationKey = key; }
    QString locationKey() const { return m_locationKey; }
    
    void setDisplayName(const QString& name) { m_displayName = name; }
    QString displayName() const { return m_displayName; }
    
    void setIsLibrary(bool isLib) { m_isLibrary = isLib; }
    bool isLibrary() const { return m_isLibrary; }
    
    // For Template nodes (leaves)
    void setResource(const DiscoveredResource& res) { m_resource = res; }
    const DiscoveredResource& resource() const { return m_resource; }
    bool hasResource() const { return m_nodeType == NodeType::Template; }
    
private:
    NodeType m_nodeType;
    TemplateTreeNode* m_parent;
    QVector<TemplateTreeNode*> m_children;
    
    // Data depending on node type
    ResourceTier m_tier = ResourceTier::Installation;
    QString m_locationKey;
    QString m_displayName;
    bool m_isLibrary = false;
    DiscoveredResource m_resource;
};


/**
 * @brief Tree model for template display in QTreeView
 * 
 * Implements QAbstractItemModel to display templates from ResourceStore
 * in a hierarchical tree: Tier → Location/Library → Template
 * 
 * Usage:
 * @code
 *   ResourceStore store;
 *   // ... populate store with scanner ...
 *   
 *   TemplateTreeModel model;
 *   model.setResourceStore(&store);
 *   
 *   QTreeView* view = new QTreeView;
 *   view->setModel(&model);
 * @endcode
 */
class RESOURCEMGMT_API TemplateTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /// Custom data roles
    enum Roles {
        PathRole = Qt::UserRole + 1,    ///< Full file path
        TierRole,                        ///< ResourceTier enum value
        LocationKeyRole,                 ///< Location key string
        IsLibraryRole,                   ///< True if location is a library
        NodeTypeRole,                    ///< TemplateTreeNode::NodeType
        ResourceRole                     ///< Full DiscoveredResource (for leaves)
    };
    
    explicit TemplateTreeModel(QObject* parent = nullptr);
    ~TemplateTreeModel() override;
    
    // Data source
    void setResourceStore(ResourceStore* store);
    ResourceStore* resourceStore() const { return m_store; }
    
    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    
    // Role names for QML (if ever needed)
    QHash<int, QByteArray> roleNames() const override;
    
    // Convenience
    TemplateTreeNode* nodeFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromNode(TemplateTreeNode* node, int column = 0) const;
    
    // Find template by path
    QModelIndex findTemplatePath(const QString& path) const;
    
public slots:
    /// Rebuild entire tree from ResourceStore
    void rebuild();
    
private slots:
    void onResourceAdded(const DiscoveredResource& resource);
    void onResourceRemoved(const QString& path);
    void onResourcesCleared(ResourceType type);
    
private:
    ResourceStore* m_store = nullptr;
    std::unique_ptr<TemplateTreeNode> m_rootNode;
    
    // Helper to build tree structure
    void buildTree();
    TemplateTreeNode* findOrCreateTierNode(ResourceTier tier);
    TemplateTreeNode* findOrCreateLocationNode(TemplateTreeNode* tierNode, 
                                                const QString& locationKey,
                                                bool isLibrary);
    void sortTree(TemplateTreeNode* node);
    static QString templateDisplayName(const TemplateTreeNode* node);
    
    // Extract library name from path if applicable
    static QString extractLibraryName(const QString& path);
    static bool isLibraryPath(const QString& path, const QString& locationKey);
    
    // Tier display names
    static QString tierDisplayName(ResourceTier tier);

    // Extract display name for location based on tier
    static QString extractLocationDisplayName(ResourceTier tier, const QString& locationKey,
                                               bool isLibrary, const QString& templatePath);
};

} // namespace resInventory

#endif // TEMPLATETREEMODEL_H
