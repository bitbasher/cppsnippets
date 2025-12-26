/**
 * @file templateTreeModel.cpp
 * @brief Implementation of TemplateTreeModel
 */

#include "resInventory/templateTreeModel.h"
#include <QIcon>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

namespace resInventory {

// ============================================================================
// TemplateTreeNode implementation
// ============================================================================

TemplateTreeNode::TemplateTreeNode(NodeType type, TemplateTreeNode* parent)
    : m_nodeType(type)
    , m_parent(parent)
{
}

TemplateTreeNode::~TemplateTreeNode()
{
    qDeleteAll(m_children);
}

TemplateTreeNode* TemplateTreeNode::child(int row) const
{
    if (row < 0 || row >= m_children.size()) {
        return nullptr;
    }
    return m_children.at(row);
}

int TemplateTreeNode::row() const
{
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<TemplateTreeNode*>(this));
    }
    return 0;
}

void TemplateTreeNode::appendChild(TemplateTreeNode* child)
{
    child->m_parent = this;
    m_children.append(child);
}

void TemplateTreeNode::removeChild(int row)
{
    if (row >= 0 && row < m_children.size()) {
        delete m_children.takeAt(row);
    }
}

void TemplateTreeNode::clearChildren()
{
    qDeleteAll(m_children);
    m_children.clear();
}

void TemplateTreeNode::sortChildren(const std::function<bool(const TemplateTreeNode*, const TemplateTreeNode*)>& less)
{
    std::sort(m_children.begin(), m_children.end(), less);
}


// ============================================================================
// TemplateTreeModel implementation
// ============================================================================

TemplateTreeModel::TemplateTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootNode(std::make_unique<TemplateTreeNode>(TemplateTreeNode::NodeType::Root))
{
}

TemplateTreeModel::~TemplateTreeModel() = default;

void TemplateTreeModel::setResourceStore(ResourceStore* store)
{
    if (m_store == store) {
        return;
    }
    
    // Disconnect old store
    if (m_store) {
        disconnect(m_store, nullptr, this, nullptr);
    }
    
    m_store = store;
    
    // Connect new store
    if (m_store) {
        connect(m_store, &ResourceStore::resourceAdded,
                this, &TemplateTreeModel::onResourceAdded);
        connect(m_store, &ResourceStore::resourceRemoved,
                this, &TemplateTreeModel::onResourceRemoved);
        connect(m_store, &ResourceStore::resourcesCleared,
                this, &TemplateTreeModel::onResourcesCleared);
    }
    
    rebuild();
}

// ============================================================================
// QAbstractItemModel interface
// ============================================================================

QModelIndex TemplateTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    
    TemplateTreeNode* parentNode;
    if (!parent.isValid()) {
        parentNode = m_rootNode.get();
    } else {
        parentNode = static_cast<TemplateTreeNode*>(parent.internalPointer());
    }
    
    TemplateTreeNode* childNode = parentNode->child(row);
    if (childNode) {
        return createIndex(row, column, childNode);
    }
    return QModelIndex();
}

QModelIndex TemplateTreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }
    
    TemplateTreeNode* childNode = static_cast<TemplateTreeNode*>(child.internalPointer());
    TemplateTreeNode* parentNode = childNode->parent();
    
    if (parentNode == m_rootNode.get() || parentNode == nullptr) {
        return QModelIndex();
    }
    
    return createIndex(parentNode->row(), 0, parentNode);
}

int TemplateTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0) {
        return 0;
    }
    
    TemplateTreeNode* parentNode;
    if (!parent.isValid()) {
        parentNode = m_rootNode.get();
    } else {
        parentNode = static_cast<TemplateTreeNode*>(parent.internalPointer());
    }
    
    return parentNode->childCount();
}

int TemplateTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 3;  // Name, Category, Path
}

QVariant TemplateTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    
    TemplateTreeNode* node = static_cast<TemplateTreeNode*>(index.internalPointer());
    
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:  // Name column
            switch (node->nodeType()) {
            case TemplateTreeNode::NodeType::Tier:
                return tierDisplayName(node->tier());
            case TemplateTreeNode::NodeType::Location:
                return node->displayName();
            case TemplateTreeNode::NodeType::Template: {
                // Strip .json extension from display name
                QString name = node->resource().name;
                if (name.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
                    name.chop(5);
                }
                return name;
            }
            default:
                return QVariant();
            }
        case 1:  // Category column
            if (node->nodeType() == TemplateTreeNode::NodeType::Template) {
                return node->resource().category;
            }
            return QVariant();
        case 2:  // Description column
            if (node->nodeType() == TemplateTreeNode::NodeType::Template) {
                // Show JSON 'description' field if present
                const QString path = node->resource().path;
                QFile f(path);
                if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    const auto doc = QJsonDocument::fromJson(f.readAll());
                    f.close();
                    if (doc.isObject()) {
                        const auto obj = doc.object();
                        
                        // Try root-level description first (legacy or single-snippet format)
                        auto descVal = obj.value(QStringLiteral("description"));
                        if (descVal.isString()) {
                            return descVal.toString();
                        }
                        
                        // Try VSCode snippet format: look in first non-metadata key
                        for (auto it = obj.begin(); it != obj.end(); ++it) {
                            if (it.key().startsWith('_')) continue;  // Skip metadata keys
                            if (it.value().isObject()) {
                                descVal = it.value().toObject().value(QStringLiteral("description"));
                                if (descVal.isString()) {
                                    return descVal.toString();
                                }
                            }
                        }
                    }
                }
                return QString();  // No description
            }
            return QVariant();
        }
        break;
        
    case Qt::ToolTipRole:
        if (node->nodeType() == TemplateTreeNode::NodeType::Template) {
            // Show display name and canonical path to help user distinguish
            // between standard templates and those from newresources container
            const DiscoveredResource& res = node->resource();
                QString tooltip = res.name;
            if (!res.path.isEmpty()) {
                tooltip += QStringLiteral("\n") + QDir::fromNativeSeparators(res.path);
            }
            return tooltip;
        } else if (node->nodeType() == TemplateTreeNode::NodeType::Location) {
            return node->locationKey();
        }
        break;
        
    case Qt::DecorationRole:
        if (index.column() == 0) {
            // Could add icons here based on node type
            // return QIcon(":/icons/...");
        }
        break;
        
    // Custom roles
    case PathRole:
        if (node->nodeType() == TemplateTreeNode::NodeType::Template) {
            return node->resource().path;
        }
        return QVariant();
        
    case TierRole:
        if (node->nodeType() == TemplateTreeNode::NodeType::Tier) {
            return static_cast<int>(node->tier());
        } else if (node->nodeType() == TemplateTreeNode::NodeType::Location ||
                   node->nodeType() == TemplateTreeNode::NodeType::Template) {
            // Walk up to find tier
            TemplateTreeNode* p = node->parent();
            while (p && p->nodeType() != TemplateTreeNode::NodeType::Tier) {
                p = p->parent();
            }
            if (p) {
                return static_cast<int>(p->tier());
            }
        }
        return QVariant();
        
    case LocationKeyRole:
        if (node->nodeType() == TemplateTreeNode::NodeType::Location) {
            return node->locationKey();
        } else if (node->nodeType() == TemplateTreeNode::NodeType::Template) {
            return node->resource().locationKey;
        }
        return QVariant();
        
    case IsLibraryRole:
        if (node->nodeType() == TemplateTreeNode::NodeType::Location) {
            return node->isLibrary();
        }
        return false;
        
    case NodeTypeRole:
        return static_cast<int>(node->nodeType());
        
    case ResourceRole:
        if (node->nodeType() == TemplateTreeNode::NodeType::Template) {
            return QVariant::fromValue(node->resource());
        }
        return QVariant();
    }
    
    return QVariant();
}

QVariant TemplateTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("Name");
        case 1: return tr("Category");
        case 2: return tr("Description");
        }
    }
    return QVariant();
}

Qt::ItemFlags TemplateTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    // Only template leaves can be selected for editing
    TemplateTreeNode* node = static_cast<TemplateTreeNode*>(index.internalPointer());
    if (node->nodeType() == TemplateTreeNode::NodeType::Template) {
        flags |= Qt::ItemNeverHasChildren;
    }
    
    return flags;
}

QHash<int, QByteArray> TemplateTreeModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[PathRole] = "path";
    roles[TierRole] = "tier";
    roles[LocationKeyRole] = "locationKey";
    roles[IsLibraryRole] = "isLibrary";
    roles[NodeTypeRole] = "nodeType";
    roles[ResourceRole] = "resource";
    return roles;
}

// ============================================================================
// Convenience methods
// ============================================================================

TemplateTreeNode* TemplateTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return m_rootNode.get();
    }
    return static_cast<TemplateTreeNode*>(index.internalPointer());
}

QModelIndex TemplateTreeModel::indexFromNode(TemplateTreeNode* node, int column) const
{
    if (!node || node == m_rootNode.get()) {
        return QModelIndex();
    }
    return createIndex(node->row(), column, node);
}

QModelIndex TemplateTreeModel::findTemplatePath(const QString& path) const
{
    // Search all template nodes
    std::function<TemplateTreeNode*(TemplateTreeNode*)> findRecursive;
    findRecursive = [&](TemplateTreeNode* node) -> TemplateTreeNode* {
        if (node->nodeType() == TemplateTreeNode::NodeType::Template) {
            if (node->resource().path == path) {
                return node;
            }
        }
        for (int i = 0; i < node->childCount(); ++i) {
            if (auto found = findRecursive(node->child(i))) {
                return found;
            }
        }
        return nullptr;
    };
    
    if (auto node = findRecursive(m_rootNode.get())) {
        return indexFromNode(node);
    }
    return QModelIndex();
}

// ============================================================================
// Slots
// ============================================================================

void TemplateTreeModel::rebuild()
{
    beginResetModel();
    m_rootNode->clearChildren();
    if (m_store) {
        buildTree();
        sortTree(m_rootNode.get());
    }
    endResetModel();
}

void TemplateTreeModel::onResourceAdded(const DiscoveredResource& resource)
{
    // Only care about templates
    if (resource.type != ResourceType::Template) {
        return;
    }
    
    // For simplicity, just rebuild
    // A more sophisticated implementation would insert just the new node
    rebuild();
}

void TemplateTreeModel::onResourceRemoved(const QString& path)
{
    Q_UNUSED(path);
    // For simplicity, just rebuild
    rebuild();
}

void TemplateTreeModel::onResourcesCleared(ResourceType type)
{
    if (type == ResourceType::Template || type == ResourceType::Unknown) {
        rebuild();
    }
}

// ============================================================================
// Tree building
// ============================================================================

void TemplateTreeModel::buildTree()
{
    if (!m_store) {
        return;
    }
    
    // Get all templates
    auto templates = m_store->resourcesOfType(ResourceType::Template);
    
    for (const auto& tmpl : templates) {
        // Find or create tier node
        TemplateTreeNode* tierNode = findOrCreateTierNode(tmpl.tier);
        
        // Determine if this is inside a library
        bool isLib = isLibraryPath(tmpl.path, tmpl.locationKey);
        QString locationDisplay = extractLocationDisplayName(tmpl.tier, tmpl.locationKey, isLib, tmpl.path);
        
        // Find or create location node
        QString locationKey = isLib ? extractLibraryName(tmpl.path) : tmpl.locationKey;
        TemplateTreeNode* locationNode = findOrCreateLocationNode(tierNode, locationKey, isLib);
        
        if (locationNode->displayName().isEmpty()) {
            locationNode->setDisplayName(locationDisplay);
        }
        
        // Create template leaf node
        auto* templateNode = new TemplateTreeNode(TemplateTreeNode::NodeType::Template, locationNode);
        templateNode->setResource(tmpl);
        locationNode->appendChild(templateNode);
    }
}

TemplateTreeNode* TemplateTreeModel::findOrCreateTierNode(ResourceTier tier)
{
    // Search existing tier nodes
    for (int i = 0; i < m_rootNode->childCount(); ++i) {
        auto* node = m_rootNode->child(i);
        if (node->nodeType() == TemplateTreeNode::NodeType::Tier && node->tier() == tier) {
            return node;
        }
    }
    
    // Create new tier node
    auto* tierNode = new TemplateTreeNode(TemplateTreeNode::NodeType::Tier, m_rootNode.get());
    tierNode->setTier(tier);
    tierNode->setDisplayName(tierDisplayName(tier));
    m_rootNode->appendChild(tierNode);
    return tierNode;
}

TemplateTreeNode* TemplateTreeModel::findOrCreateLocationNode(TemplateTreeNode* tierNode,
                                                               const QString& locationKey,
                                                               bool isLibrary)
{
    // Search existing location nodes
    for (int i = 0; i < tierNode->childCount(); ++i) {
        auto* node = tierNode->child(i);
        if (node->nodeType() == TemplateTreeNode::NodeType::Location && 
            node->locationKey() == locationKey) {
            return node;
        }
    }
    
    // Create new location node
    auto* locationNode = new TemplateTreeNode(TemplateTreeNode::NodeType::Location, tierNode);
    locationNode->setLocationKey(locationKey);
    locationNode->setIsLibrary(isLibrary);
    tierNode->appendChild(locationNode);
    return locationNode;
}

QString TemplateTreeModel::extractLibraryName(const QString& path)
{
    // Look for "libraries/<name>/templates" pattern
    // e.g., "/path/to/libraries/MCAD/templates/foo.json" -> "MCAD"
    
    static const QStringList markers = { 
        QStringLiteral("/libraries/"),
        QStringLiteral("\\libraries\\")
    };
    
    for (const auto& marker : markers) {
        int idx = path.indexOf(marker, 0, Qt::CaseInsensitive);
        if (idx >= 0) {
            int start = idx + marker.length();
            int end = path.indexOf('/', start);
            if (end < 0) {
                end = path.indexOf('\\', start);
            }
            if (end > start) {
                return path.mid(start, end - start);
            }
        }
    }
    
    return QString();
}

bool TemplateTreeModel::isLibraryPath(const QString& path, const QString& locationKey)
{
    // If path contains "libraries" between locationKey and the file, it's a library
    QString relativePart = path;
    if (path.startsWith(locationKey)) {
        relativePart = path.mid(locationKey.length());
    }
    
    return relativePart.contains(QStringLiteral("/libraries/"), Qt::CaseInsensitive) ||
           relativePart.contains(QStringLiteral("\\libraries\\"), Qt::CaseInsensitive);
}

QString TemplateTreeModel::tierDisplayName(ResourceTier tier)
{
    switch (tier) {
    case ResourceTier::Installation:
        return tr("Installation");
    case ResourceTier::Machine:
        return tr("Machine");
    case ResourceTier::User:
        return tr("User");
    }
    return QString();
}

QString TemplateTreeModel::extractLocationDisplayName(ResourceTier tier, const QString& locationKey,
                                                      bool isLibrary, const QString& templatePath)
{
    if (isLibrary) {
        return extractLibraryName(templatePath);
    }

    QString normalized = locationKey;
    normalized.replace(QLatin1Char('\\'), QLatin1Char('/'));
    while (normalized.endsWith(QLatin1Char('/'))) {
        normalized.chop(1);
    }

    switch (tier) {
    case ResourceTier::Installation: {
        // Use the installation folder name (e.g., "OpenSCAD" or "OpenSCAD (Nightly)")
        return QFileInfo(normalized).fileName();
    }

    case ResourceTier::Machine: {
        // Show path up to but excluding "openscad" (case-insensitive)
        QString display = normalized;
        int idx = display.lastIndexOf(QLatin1String("/openscad"), -1, Qt::CaseInsensitive);
        if (idx >= 0) {
            display = display.left(idx);
        }
        while (display.endsWith(QLatin1Char('/'))) {
            display.chop(1);
        }
        if (!display.endsWith(QLatin1Char('/'))) {
            display += QLatin1Char('/');
        }
        return display;
    }

    case ResourceTier::User: {
        // Remove home prefix and "openscad", then show first two folders
        QString display = normalized;

        QString home = QDir::homePath();
        QString normHome = home;
        normHome.replace(QLatin1Char('\\'), QLatin1Char('/'));
        while (normHome.endsWith(QLatin1Char('/'))) {
            normHome.chop(1);
        }

        if (display.startsWith(normHome, Qt::CaseInsensitive)) {
            display = display.mid(normHome.length());
        }
        while (display.startsWith(QLatin1Char('/'))) {
            display.remove(0, 1);
        }

        QStringList parts = display.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        QStringList filtered;
        for (const auto& part : parts) {
            if (part.compare(QLatin1String("openscad"), Qt::CaseInsensitive) == 0) {
                continue;
            }
            if (filtered.size() < 2) {
                filtered.append(part);
            }
        }

        if (!filtered.isEmpty()) {
            return filtered.join(QLatin1Char('/'));
        }

        // Fallback: show the last component
        return QFileInfo(normalized).fileName();
    }
    }

    // Should not reach here, but return empty string as fallback
    return QString();

}

QString TemplateTreeModel::templateDisplayName(const TemplateTreeNode* node)
{
    if (!node || node->nodeType() != TemplateTreeNode::NodeType::Template) {
        return QString();
    }
    QString name = node->resource().name;
    if (name.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
        name.chop(5);
    }
    return name;
}

void TemplateTreeModel::sortTree(TemplateTreeNode* node)
{
    if (!node) {
        return;
    }

    switch (node->nodeType()) {
    case TemplateTreeNode::NodeType::Root:
    case TemplateTreeNode::NodeType::Tier:
        node->sortChildren([](const TemplateTreeNode* a, const TemplateTreeNode* b) {
            // Tier nodes live under Root; Tier children live under Tier nodes as Locations.
            // Sort by display name as a fallback, but preserve tier numeric order when applicable.
            if (a->nodeType() == TemplateTreeNode::NodeType::Tier && b->nodeType() == TemplateTreeNode::NodeType::Tier) {
                return static_cast<int>(a->tier()) < static_cast<int>(b->tier());
            }
            return QString::localeAwareCompare(a->displayName(), b->displayName()) < 0;
        });
        break;

    case TemplateTreeNode::NodeType::Location:
        node->sortChildren([](const TemplateTreeNode* a, const TemplateTreeNode* b) {
            const QString an = TemplateTreeModel::templateDisplayName(a);
            const QString bn = TemplateTreeModel::templateDisplayName(b);
            return QString::localeAwareCompare(an, bn) < 0;
        });
        break;

    case TemplateTreeNode::NodeType::Template:
    default:
        break;
    }

    for (int i = 0; i < node->childCount(); ++i) {
        sortTree(node->child(i));
    }
}

} // namespace resInventory
