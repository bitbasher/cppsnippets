/**
 * @file ExamplesInventory.cpp
 * @brief Implementation of ExamplesInventory
 */

#include "ExamplesInventory.hpp"
#include "ResourceIndexer.hpp"
#include "../platformInfo/ResourceLocation.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"

#include <QFileInfo>
#include <QDir>
#include <QDirListing>
#include <QFont>

namespace resourceInventory {

using namespace resourceMetadata;
using namespace platformInfo;

ExamplesInventory::ExamplesInventory(QObject* parent)
    : QAbstractItemModel(parent)
{
}

bool ExamplesInventory::addExample(const QDirListing::DirEntry& entry, 
                                    const platformInfo::ResourceLocation& location,
                                    const QString& category)
{
    QString scriptPath = entry.filePath();
    
    // Validate it's a .scad file FIRST
    if (!scriptPath.endsWith(QStringLiteral(".scad"), Qt::CaseInsensitive)) {
        return false;
    }
    
    QFileInfo fi(scriptPath);
    QString baseName = fi.baseName();
    
    // Create ResourceScript with convenience constructor
    ResourceScript script(scriptPath, baseName);
    script.setTier(location.tier());
    script.setCategory(category);
    script.setDisplayName(baseName);
    
    // Scan for attachments
    QStringList attachments = scanAttachments(scriptPath);
    if (!attachments.isEmpty()) {
        script.setAttachments(attachments);
    }
    
    // Generate unique ID using ResourceIndexer
    QString uniqueID = ResourceIndexer::getUniqueIDString(baseName);
    script.setUniqueID(uniqueID);
    
    // Check for duplicates
    if (m_scripts.contains(uniqueID)) {
        qWarning() << "ExamplesInventory: Duplicate example ID:" << uniqueID
                   << "at" << entry.filePath();
        return false;
    }
    
    // Insert into storage
    m_scripts.insert(uniqueID, script);
    
    // Update category mapping
    if (!m_categoryToIds.contains(category)) {
        // New category - add to category keys in sorted position
        int insertPos = 0;
        if (category.isEmpty()) {
            // Empty category (loose files) always first
            insertPos = 0;
        } else {
            // Find sorted position (skip empty if present)
            int startIdx = m_categoryKeys.isEmpty() || !m_categoryKeys.first().isEmpty() ? 0 : 1;
            for (int i = startIdx; i < m_categoryKeys.size(); ++i) {
                if (category.compare(m_categoryKeys[i], Qt::CaseInsensitive) < 0) {
                    insertPos = i;
                    break;
                }
            }
            if (insertPos == 0 && !m_categoryKeys.isEmpty()) {
                insertPos = m_categoryKeys.size(); // append
            }
        }
        
        beginInsertRows(QModelIndex(), insertPos, insertPos);
        m_categoryKeys.insert(insertPos, category);
        m_categoryToIds[category] = QList<QString>();
        endInsertRows();
    }
    
    // Add script ID to category's list
    int catRow = categoryRow(category);
    QModelIndex categoryIndex = createIndex(catRow, 0, quintptr(-1 - catRow));
    int scriptPos = m_categoryToIds[category].size();
    
    beginInsertRows(categoryIndex, scriptPos, scriptPos);
    m_categoryToIds[category].append(uniqueID);
    endInsertRows();
    
    return true;
}

int ExamplesInventory::addFolder(const QDirListing::DirEntry& dirEntry,
                                  const platformInfo::ResourceLocation& location)
{
    int sizeBefore = m_scripts.size();
    
    // Extract category name from folder basename
    // If basename matches resource type name (e.g., "examples"), it's uncategorized root
    QString folderName = dirEntry.fileName();
    QString category = folderName;
    
    // Check if this is the root examples folder (not a category subfolder)
    const QString& examplesFolder = resourceMetadata::ResourceTypeInfo::s_resourceTypes
        [resourceMetadata::ResourceType::Examples].getSubDir();
    if (folderName == examplesFolder) {
        category = QString();  // Empty for uncategorized
    }
    
    // Scan folder for .scad files (FilesOnly flag eliminates need for isFile() check)
    QDirListing listing(dirEntry.filePath(), {"*.scad"}, QDirListing::IteratorFlag::FilesOnly);
    
    for (const auto& fileEntry : listing) {
        addExample(fileEntry, location, category);  // Failures logged internally
    }
    
    return m_scripts.size() - sizeBefore;
}

QVariant ExamplesInventory::get(const QString& key) const
{
    if (m_scripts.contains(key)) {
        return QVariant::fromValue(m_scripts.value(key));
    }
    return QVariant();
}

bool ExamplesInventory::contains(const QString& path) const
{
    return m_scripts.contains(path);
}

QList<QVariant> ExamplesInventory::getAll() const
{
    QList<QVariant> all;
    for (const ResourceScript& script : m_scripts) {
        all.append(QVariant::fromValue(script));
    }
    return all;
}

QList<QVariant> ExamplesInventory::getByCategory(const QString& category) const
{
    QList<QVariant> filtered;
    
    if (m_categoryToIds.contains(category)) {
        const QList<QString>& ids = m_categoryToIds[category];
        for (const QString& id : ids) {
            if (m_scripts.contains(id)) {
                filtered.append(QVariant::fromValue(m_scripts[id]));
            }
        }
    }
    
    return filtered;
}

QStringList ExamplesInventory::getCategories() const
{
    // Return copy of sorted category keys (excluding empty for loose files)
    QStringList categories;
    for (const QString& cat : m_categoryKeys) {
        if (!cat.isEmpty()) {
            categories.append(cat);
        }
    }
    return categories;
}

void ExamplesInventory::clear()
{
    beginResetModel();
    m_scripts.clear();
    m_categoryToIds.clear();
    m_categoryKeys.clear();
    endResetModel();
}

QStringList ExamplesInventory::scanAttachments(const QString& scriptPath) const
{
    QFileInfo scriptInfo(scriptPath);
    QString baseName = scriptInfo.baseName();  // filename without extension
    QString dirPath = scriptInfo.absolutePath();
    
    QStringList attachments;
    
    // Scan directory for files with same baseName but different extensions
    QDir dir(dirPath);
    if (!dir.exists()) {
        return attachments;
    }
    
    // Get s_attachments list from ResourceTypeInfo
    const QStringList& allowedExtensions = resourceMetadata::s_attachments;
    
    // Check each allowed extension
    for (const QString& ext : allowedExtensions) {
        QString candidatePath = dir.filePath(baseName + ext);
        if (QFileInfo::exists(candidatePath)) {
            attachments.append(candidatePath);
        }
    }
    
    return attachments;
}

// ============================================================================
// QAbstractItemModel Implementation
// ============================================================================

QModelIndex ExamplesInventory::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0 || column >= 2) {
        return QModelIndex();
    }
    
    if (!parent.isValid()) {
        // Top level: category rows
        if (row >= m_categoryKeys.size()) {
            return QModelIndex();
        }
        // Use NEGATIVE values for categories to distinguish from children
        // Category index: -(row + 1), so cat 0 = -1, cat 1 = -2, etc.
        return createIndex(row, column, quintptr(-1 - row));
    } else {
        // Child level: script rows within category
        QString category = m_categoryKeys.value(parent.row());
        const QList<QString>& ids = m_categoryToIds.value(category);
        
        if (row >= ids.size()) {
            return QModelIndex();
        }
        
        // Child index: encode parent row in high bits, child row in low bits, add 1 to ensure positive
        quintptr id = (quintptr(parent.row() + 1) << 16) | quintptr(row + 1);
        return createIndex(row, column, id);
    }
}

QModelIndex ExamplesInventory::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    
    qintptr id = qintptr(index.internalId());
    
    // Category items have negative IDs
    if (id < 0) {
        return QModelIndex(); // Top-level category has no parent
    }
    
    // Child items have positive IDs with parent encoded
    if (id > 0) {
        // Extract parent row from high bits
        int parentRow = int((quintptr(id) >> 16) - 1);
        if (parentRow >= 0 && parentRow < m_categoryKeys.size()) {
            return createIndex(parentRow, 0, quintptr(-1 - parentRow));
        }
    }
    
    return QModelIndex();
}

int ExamplesInventory::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        // Top level: number of categories
        return m_categoryKeys.size();
    }
    
    if (parent.column() != 0) {
        return 0; // Only column 0 has children
    }
    
    // Check if parent is a category (negative id) or a script (positive id)
    qintptr id = qintptr(parent.internalId());
    if (id >= 0) {
        // This is a script item (positive id), scripts have no children
        return 0;
    }
    
    // Category item (negative id): number of scripts in category
    // Category row is encoded as -(row + 1), so row = -(id + 1)
    int categoryRow = -int(id + 1);
    if (categoryRow < 0 || categoryRow >= m_categoryKeys.size()) {
        return 0;
    }
    QString category = m_categoryKeys.value(categoryRow);
    return m_categoryToIds.value(category).size();
}

int ExamplesInventory::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 2; // Name, ID
}

QVariant ExamplesInventory::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    
    if (!index.parent().isValid()) {
        // Category row (parent)
        QString category = m_categoryKeys.value(index.row());
        
        if (role == Qt::DisplayRole && index.column() == 0) {
            if (category.isEmpty()) {
                return QStringLiteral("(Loose Files)");
            }
            return category;
        }
        
        if (role == Qt::FontRole && index.column() == 0) {
            QFont font;
            font.setBold(true);
            return font;
        }
        
        return QVariant();
    }
    
    // Script row (child)
    QString category = m_categoryKeys.value(index.parent().row());
    const QList<QString>& ids = m_categoryToIds.value(category);
    
    if (index.row() >= ids.size()) {
        return QVariant();
    }
    
    QString scriptId = ids.at(index.row());
    const ResourceScript& script = m_scripts.value(scriptId);
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return script.displayName();
            case 1: return script.uniqueID();
            default: return QVariant();
        }
    }
    
    if (role == Qt::UserRole) {
        return QVariant::fromValue(script);
    }
    
    if (role == Qt::ToolTipRole && index.column() == 0) {
        QString tooltip = QString("Path: %1\nTier: %2\nID: %3")
            .arg(script.path())
            .arg(tierToString(script.tier()))
            .arg(script.uniqueID());
        
        if (script.hasAttachments()) {
            tooltip += "\nAttachments: " + script.attachments().join(", ");
        }
        
        return tooltip;
    }
    
    return QVariant();
}

QVariant ExamplesInventory::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return QStringLiteral("Name");
            case 1: return QStringLiteral("ID");
            default: return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags ExamplesInventory::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    // Only child items (scripts) are editable, not categories
    if (index.parent().isValid()) {
        flags |= Qt::ItemIsEditable;
    }
    
    return flags;
}

int ExamplesInventory::categoryRow(const QString& category) const
{
    return m_categoryKeys.indexOf(category);
}

int ExamplesInventory::scriptRowInCategory(const QString& uniqueID, const QString& category) const
{
    if (!m_categoryToIds.contains(category)) {
        return -1;
    }
    
    return m_categoryToIds[category].indexOf(uniqueID);
}

} // namespace resourceInventory
