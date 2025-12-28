#include "gui/resourceLocationModel.h"

ResourceLocationModel::ResourceLocationModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void ResourceLocationModel::setLocations(const QList<platformInfo::ResourceLocation>& locations)
{
    beginResetModel();
    m_locations = locations;
    endResetModel();
}

QList<platformInfo::ResourceLocation> ResourceLocationModel::locations() const
{
    return m_locations;
}

QStringList ResourceLocationModel::enabledPaths() const
{
    QStringList enabled;
    for (const auto& loc : m_locations) {
        if (loc.isEnabled && !loc.path.isEmpty()) {
            enabled.append(loc.path);
        }
    }
    return enabled;
}

int ResourceLocationModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;  // List model has no children
    }
    return m_locations.size();
}

QVariant ResourceLocationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_locations.size()) {
        return QVariant();
    }

    const auto& loc = m_locations[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        // Show display name and path on a single line
        if (!loc.displayName.isEmpty() && !loc.path.isEmpty()) {
            return QStringLiteral("%1 - %2").arg(loc.displayName, loc.path);
        }
        return loc.displayName.isEmpty() ? loc.path : loc.displayName;
    
    case Qt::ToolTipRole:
        // Show path and description in tooltip
        if (!loc.description.isEmpty()) {
            return QStringLiteral("%1\n%2").arg(loc.path, loc.description);
        }
        return loc.path;
    
    case Qt::CheckStateRole:
        return loc.isEnabled ? Qt::Checked : Qt::Unchecked;
    
    case Qt::DecorationRole:
        // Could add icons here based on exists/isWritable status
        return QVariant();
    
    default:
        return QVariant();
    }
}

bool ResourceLocationModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_locations.size()) {
        return false;
    }

    if (role == Qt::CheckStateRole) {
        m_locations[index.row()].isEnabled = (value.toInt() == Qt::Checked);
        emit dataChanged(index, index, {role});
        return true;
    }

    return false;
}

Qt::ItemFlags ResourceLocationModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    return itemFlags;
}
