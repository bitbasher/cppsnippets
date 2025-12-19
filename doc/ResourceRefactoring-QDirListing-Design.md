# Resource Management Refactoring Design Document

**Date**: December 20, 2025  
**Status**: Planning Phase  
**Qt Version**: 6.10.1 (installed at `C:\bin\Qt`)

---

## 1. Overview

This document captures the design decisions for refactoring the OpenSCAD resource management classes to use modern Qt 6 patterns, specifically `QDirListing`, Qt Model/View architecture, and `QDataWidgetMapper`.

---

## 2. User Inputs: Qt Classes Discovered

The following Qt 6 classes were identified as candidates for the refactoring:

### 2.1 QDirListing (Qt 6.8+)
**Documentation**: https://doc.qt.io/qt-6/qdirlisting.html

> You can use QDirListing to navigate entries of a directory one at a time. It is similar to QDir::entryList() and QDir::entryInfoList(), but because it lists entries one at a time instead of all at once, it scales better and is more suitable for large directories. It also supports listing directory contents recursively, and following symbolic links. Unlike QDir::entryList(), QDirListing does not support sorting.

**Key Features**:
- Forward-only, single-pass iterator (C++20 `std::input_iterator`)
- Recursive directory traversal with `IteratorFlag::Recursive`
- Filter by type: `FilesOnly`, `DirsOnly`, `ExcludeOther`
- `DirEntry` provides efficient access to file metadata without constructing `QFileInfo`

**Example**:
```cpp
using F = QDirListing::IteratorFlag;
for (const auto& dirEntry : QDirListing(u"/etc"_s, F::Recursive)) {
    qDebug() << dirEntry.filePath();
}
```

### 2.2 QVariant
**Documentation**: https://doc.qt.io/qt-6/qvariant.html

> A QVariant object holds a single value of a single typeId() at a time. You can even store QList<QVariant> and QMap<QString, QVariant> values in a variant, so you can easily construct arbitrarily complex data structures of arbitrary types. This is very powerful and versatile, but may prove less memory and speed efficient than storing specific types in standard data structures.

**Key Features**:
- Type-safe union for Qt data types
- Custom types via `Q_DECLARE_METATYPE`
- `QVariant::fromValue<T>()` and `value<T>()` for custom types
- `QVariantList` and `QVariantMap` aliases for complex structures

### 2.3 QDataWidgetMapper
**Documentation**: https://doc.qt.io/qt-6/qdatawidgetmapper.html

> QDataWidgetMapper can be used to create data-aware widgets by mapping them to sections of an item model. A section is a column of a model if the orientation is horizontal (the default), otherwise a row.

**Key Features**:
- Maps model columns/rows to UI widgets
- `AutoSubmit` or `ManualSubmit` policies
- `addMapping(widget, section)` binds widgets to model columns
- Works with any `QAbstractItemModel`

**Example**:
```cpp
QDataWidgetMapper *mapper = new QDataWidgetMapper;
mapper->setModel(model);
mapper->addMapping(nameLineEdit, 0);
mapper->addMapping(ageSpinBox, 1);
mapper->toFirst();
```

### 2.4 QSignalMapper (Deprecated Pattern)
**Documentation**: https://doc.qt.io/qt-6/qsignalmapper.html

> This class was mostly useful before lambda functions could be used as slots.

**Recommendation**: Do not use for new code. Use lambda connections instead.

---

## 3. Current Architecture Analysis

### 3.1 Existing Class Roles

| Class | Purpose | Storage Type |
|-------|---------|--------------|
| `ResLocMap` | Flat storage of resource locations by tier | `QMap<QString, ResourceLocation>` × 3 tiers |
| `ResLocTree` | Hierarchical storage of resource locations | `QTreeWidget` with `ResLocTreeItem` |
| `ResourceItem` | Base class for discovered resources | Data class with path, name, tier, type |
| `ResourceScript` | Resources with attachments (examples, tests) | Extends `ResourceItem` with `QStringList m_attachments` |
| `ResourceTemplate` | Template resources with metadata | Extends `ResourceItem` with format, source, version |
| `ResourceScanner` | File system scanner | Uses `QDir`/`QFileInfo` |
| `ResourceTreeWidget` | UI widget for discovered resources | `QTreeWidget` with `ResourceTreeItem` |
| `ResourceInventoryManager` | Coordinates scanning across tiers | Owns `ResourceScanner` and `ResourceTreeWidget` instances |

### 3.2 Identified Issues

1. **Mixing Concerns**: 
   - `ResLocTree` stores *where to look* (`ResourceLocation`)
   - `ResourceTreeWidget` stores *what was found* (`ResourceItem`)
   - Two separate hierarchies with similar purposes

2. **Scanner Scalability**: 
   - Uses `QDir::entryList()` which loads all entries at once
   - Not ideal for large resource directories

3. **No Unified Storage**: 
   - Discovered results scattered across `ResourceTreeWidget` instances per type
   - Difficult to query across resource types

4. **Widget Coupling**: 
   - Discovery results tightly coupled to Qt widgets
   - Unit testing requires widget instantiation

---

## 4. Refactoring Plan

### Phase 1: Adopt QDirListing for Scanning

Replace `QDir::entryList()` with `QDirListing` for streaming iteration:

```cpp
void ResourceScanner::scanLocationWithDirListing(
    const QString& basePath,
    ResourceTier tier,
    std::function<void(const DiscoveredResource&)> callback)
{
    using F = QDirListing::IteratorFlag;
    const auto flags = F::Recursive | F::FilesOnly;
    
    for (const auto& entry : QDirListing(basePath, flags)) {
        ResourceType type = categorizeByPath(entry.filePath());
        
        if (type != ResourceType::Unknown) {
            DiscoveredResource res;
            res.path = entry.filePath();
            res.name = entry.fileName();
            res.tier = tier;
            res.type = type;
            res.lastModified = entry.lastModified();
            
            callback(res);  // Stream results
        }
    }
}
```

### Phase 2: Create Unified ResourceStore

Replace scattered `ResourceTreeWidget` instances with a unified store:

```cpp
class ResourceStore {
    QVector<ResourceItem> m_colorSchemes;
    QVector<ResourceItem> m_fonts;
    QVector<ResourceScript> m_examples;    // Has attachments
    QVector<ResourceScript> m_tests;       // Has attachments
    QVector<ResourceTemplate> m_templates;
    QVector<ResourceItem> m_translations;
    // ...
    
    void addDiscoveredResource(const DiscoveredResource& res);
    
    // Filtered access
    const QVector<ResourceScript>& examples() const { return m_examples; }
    QVector<ResourceItem> itemsForTier(ResourceTier tier) const;
    QVector<ResourceItem> itemsForLocation(const QString& locationKey) const;
};
```

### Phase 3: Implement QAbstractItemModel

Create a model that wraps `ResourceStore` for Qt views:

```cpp
class ResourceModel : public QAbstractItemModel {
    Q_OBJECT
    
    ResourceStore* m_store;
    ResourceType m_filter;
    
public:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        TypeRole,
        TierRole,
        AttachmentsRole,
        LocationKeyRole
    };
    
    // Standard model interface
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
};
```

### Phase 4: Add QDataWidgetMapper for Editing

Use `QDataWidgetMapper` for resource detail editing:

```cpp
class ResourceEditor : public QWidget {
    QDataWidgetMapper* m_mapper;
    QLineEdit* m_nameEdit;
    QLineEdit* m_pathEdit;
    QPlainTextEdit* m_descriptionEdit;
    QListWidget* m_attachmentsList;
    
    void setModel(ResourceModel* model) {
        m_mapper = new QDataWidgetMapper(this);
        m_mapper->setModel(model);
        m_mapper->addMapping(m_nameEdit, 0);
        m_mapper->addMapping(m_pathEdit, 1);
        m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    }
};
```

### Phase 5: Deprecate Legacy Classes

After validation, deprecate:
- `ResLocTree` → Merge functionality into `ResLocMap` or remove
- `ResourceTreeWidget` → Replace with `QTreeView` + `ResourceModel`

---

## 5. Proposed Class Structure

```
src/include/resInventory/
├── ResLocMap.h                 # KEEP: Reference map (where to look)
├── ResourceStore.h             # NEW: Discovered resource storage (typed vectors)
├── ResourceModel.h             # NEW: QAbstractItemModel for views
├── ResourceItem.h              # KEEP: Data classes (ResourceItem, ResourceScript, ResourceTemplate)
├── ResourceScanner.h           # REFACTOR: Use QDirListing
├── ResourceEditor.h            # NEW: QDataWidgetMapper-based editor
└── [deprecated]
    ├── ResLocTree.h            # DEPRECATE: After migration
    └── ResourceTreeWidget.h    # DEPRECATE: Use QTreeView + ResourceModel
```

---

## 6. Design Decisions (Q&A)

### Q1: QDirListing Availability
**Question**: QDirListing requires Qt 6.8+. What version is available?

**Answer**: Qt 6.10.1 is installed at `C:\bin\Qt`. QDirListing is available.

### Q2: QVariant Usage Strategy
**Question**: Should we store all resources in a single `QList<QVariant>` or use separate typed vectors?

**Answer**: **Use separate typed vectors** in `ResourceStore` for efficiency. Only use `QVariant` at the model/view boundary through data roles. This avoids runtime type checking during iteration.

### Q3: Attachment Storage
**Question**: How should attachments be stored for examples and tests?

**Answer**: **Keep current design**. Attachments remain in `ResourceScript::m_attachments` as `QStringList`. The model exposes them via `AttachmentsRole` for views.

### Q4: ResLocMap vs ResLocTree
**Question**: What is the relationship between `ResLocMap` (flat) and `ResLocTree` (hierarchical)?

**Answer**: 
- **Keep `ResLocMap`** as the **reference** of where resources *can be* found (platform-specific location definitions)
- **Use `ResourceStore`** for **discovered** resources (what was actually found)
- **Deprecate `ResLocTree`** unless a tree-based reference view is needed

---

## 7. Migration Path

| Step | Action | Risk Level |
|------|--------|------------|
| 1 | Add `QDirListing` scanner alongside existing | Low |
| 2 | Create `ResourceStore` and populate from new scanner | Low |
| 3 | Create `ResourceModel` wrapping `ResourceStore` | Low |
| 4 | Add unit tests for store/model | None |
| 5 | Update UI to use `QTreeView` + `ResourceModel` | Medium |
| 6 | Add `QDataWidgetMapper`-based editor | Low |
| 7 | Deprecate `ResLocTree`, `ResourceTreeWidget` | Low |

---

## 8. CMake Configuration Note

The project was started with Qt 6.10.0. Update `CMakeLists.txt` to reference Qt 6.10.1 if version-specific issues arise.

---

## 9. Next Steps

1. ✅ Document design decisions (this document)
2. ⏳ Review existing unit tests before coding
3. 🔜 Implement Phase 1: QDirListing scanner
4. 🔜 Implement Phase 2: ResourceStore
5. 🔜 Implement Phase 3: ResourceModel
6. 🔜 Add comprehensive unit tests
7. 🔜 Update UI components

---

## Appendix: Reference Links

- [QDirListing Documentation](https://doc.qt.io/qt-6/qdirlisting.html)
- [QVariant Documentation](https://doc.qt.io/qt-6/qvariant.html)
- [QDataWidgetMapper Documentation](https://doc.qt.io/qt-6/qdatawidgetmapper.html)
- [QAbstractItemModel Documentation](https://doc.qt.io/qt-6/qabstractitemmodel.html)
