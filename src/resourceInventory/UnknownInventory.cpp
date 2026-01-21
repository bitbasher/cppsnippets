/**
 * @file UnknownInventory.cpp
 * @brief Implementation of placeholder inventory for unimplemented resource types
 */

#include "UnknownInventory.hpp"
#include <QDebug>

namespace resourceInventory {

int UnknownInventory::addFolder(const QDirListing::DirEntry& dirEntry,
                                  const platformInfo::ResourceLocation& location) {
    qWarning() << "UnknownInventory::addFolder() called - unimplemented resource scanner for folder:"
               << dirEntry.fileName() << "at location:" << location.path();
    return 0;
}

QModelIndex UnknownInventory::index(int row, int column, const QModelIndex& parent) const {
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);
    return QModelIndex();
}

QModelIndex UnknownInventory::parent(const QModelIndex& child) const {
    Q_UNUSED(child);
    return QModelIndex();
}

int UnknownInventory::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 0;
}

int UnknownInventory::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant UnknownInventory::data(const QModelIndex& index, int role) const {
    Q_UNUSED(index);
    Q_UNUSED(role);
    return QVariant();
}

} // namespace resourceInventory
