/**
 * @file UnknownInventory.hpp
 * @brief Placeholder inventory for unimplemented resource types
 * 
 * This class serves as a safe placeholder for resource types that don't
 * yet have dedicated inventory implementations. It emits warnings if
 * accessed to aid debugging.
 */

#pragma once

#include "../platformInfo/export.hpp"
#include <QAbstractItemModel>
#include <QDirListing>
#include "platformInfo/ResourceLocation.hpp"

namespace resourceInventory {

/**
 * @class UnknownInventory
 * @brief Placeholder inventory that warns on access
 * 
 * Note: Does not inherit Q_OBJECT to avoid MOC/export complexity
 * This is just a null-object pattern placeholder
 */
class PLATFORMINFO_API UnknownInventory : public QAbstractItemModel {
public:
    UnknownInventory() = default;
    ~UnknownInventory() override = default;

    // Prevent copying
    UnknownInventory(const UnknownInventory&) = delete;
    UnknownInventory& operator=(const UnknownInventory&) = delete;

    /**
     * @brief Placeholder addFolder that emits warning
     * @return Always returns 0 (no resources added)
     */
    int addFolder(const QDirListing::DirEntry& dirEntry, 
                  const platformInfo::ResourceLocation& location);

    // QAbstractItemModel interface - minimal implementation
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

} // namespace resourceInventory
