# Qt Model/View Architecture for Resource Scanner

**Date:** January 19, 2026  
**Author:** AI Assistant + User Collaboration  
**Status:** Draft - Design Discussion  
**Branch:** resScannerOption1

---

## 1. Executive Summary

This document explores how to properly integrate resource scanning with Qt's Model/View architecture. We address:

1. **Misconception:** QStandardItemModel is NOT required - it's a convenience class
2. **Qt's actual architecture:** Any QAbstractItemModel subclass works with views
3. **The dispatch problem:** Why generic scanning still needs per-type logic
4. **Proposed architecture:** Options for clean integration

---

## 2. Qt Model/View Architecture Clarified

### 2.1 The Misconception

> "I thought we had to use QStandardItemModel to connect to the view"

**This is incorrect.** QStandardItemModel is just ONE implementation of QAbstractItemModel.

### 2.2 Qt's Actual Hierarchy

```
QAbstractItemModel (abstract base class)
    ├── QAbstractListModel (for list-like data)
    ├── QAbstractTableModel (for table-like data)
    ├── QStandardItemModel (convenience: stores QStandardItems)
    ├── QFileSystemModel (wraps filesystem)
    ├── QSqlTableModel (wraps database)
    └── YourCustomModel (wraps YOUR data structure)
```

### 2.3 What Views Actually Need

QTreeView, QListView, QTableView all work with **ANY** QAbstractItemModel:

```cpp
// These ALL work:
QTreeView* view = new QTreeView;

view->setModel(new QStandardItemModel());      // Convenience model
view->setModel(new QFileSystemModel());        // Filesystem model
view->setModel(new TemplatesListModel());      // YOUR custom model
```

### 2.4 QStandardItemModel: Pros and Cons

**Pros:**
- Easy to use - just add QStandardItem objects
- No subclassing required
- Good for prototyping

**Cons:**
- **Data duplication** - copies your data into QStandardItem objects
- **Memory overhead** - each item is a full QObject-like structure
- **No direct access** - to get original data back, must store in UserRole
- **Manual synchronization** - if source data changes, must rebuild model

**Custom Model Pros:**
- **Zero duplication** - model IS a view of your data
- **Memory efficient** - no intermediate storage
- **Direct access** - data() returns values directly from source
- **Automatic sync** - emit signals when source changes

---

## 3. The Dispatch Problem

### 3.1 User's Observation

> "Inside scanResourceAt you call scanFolderForExtensions(resourcePath, extensions, type, location); and that method is going to have to dispatch to a sub-method that can handle the resType given. Does this not just push the dispatch down a level?"

**Yes, exactly!** You identified the core issue.

### 3.2 Why Dispatch Exists

Each resource type has different:
- **Storage structure** (flat vs hierarchical)
- **Item construction** (ResourceTemplate vs ResourceScript vs ResourceFont)
- **Metadata extraction** (JSON parsing vs file attributes)
- **Inventory operations** (addTemplate vs addExample vs addFont)

### 3.3 The Fundamental Question

**Can we eliminate dispatch entirely with metadata-driven scanning?**

Let's analyze what differs per resource type:

| Resource | Folder | Extensions | Item Class | Has Categories | Has Attachments |
|----------|--------|------------|------------|----------------|-----------------|
| Templates | templates | .json | ResourceTemplate | No | No |
| Examples | examples | .scad | ResourceScript | Yes | Yes |
| Tests | tests | .scad | ResourceScript | No | Yes |
| Fonts | fonts | .ttf, .otf | ResourceFont | No | No |
| Shaders | shaders | .frag, .vert | ResourceShader | No | No |
| Translations | locale | .qm, .ts | ResourceTranslation | No | No |

**Observation:** Most differences are in the **Item Class** construction!

### 3.4 Options for Handling Dispatch

#### Option A: Keep Per-Type Scanners (Current)
```cpp
// Explicit dispatch - what we have now
const QMap<ResourceType, ScannerFunc> scannerDispatch = {
    {ResourceType::Examples, &ResourceScanner::scanExamplesAt},
    {ResourceType::Templates, &ResourceScanner::scanTemplatesAt},
    // ...
};
```
**Verdict:** Clear, explicit, but verbose

#### Option B: Factory Pattern for Items
```cpp
// Generic scanner + item factory
ResourceItem* ResourceItemFactory::create(ResourceType type, const QDirEntry& entry) {
    switch (type) {
        case ResourceType::Templates: return new ResourceTemplate(entry);
        case ResourceType::Examples: return new ResourceScript(entry);
        // ...
    }
}
```
**Verdict:** Centralizes dispatch but still has switch

#### Option C: Type-Erased Inventory
```cpp
// Single inventory that stores QVariant
class ResourceInventory {
    QHash<QString, QVariant> m_items;
    
    template<typename T>
    void add(const QString& key, const T& item) {
        m_items.insert(key, QVariant::fromValue(item));
    }
};
```
**Verdict:** Generic storage but loses type safety

#### Option D: Polymorphic Items with Virtual Construction
```cpp
class ResourceItem {
public:
    virtual ResourceItem* clone() const = 0;
    virtual void populateFromEntry(const QDirEntry& entry) = 0;
};

// Register prototypes
QHash<ResourceType, ResourceItem*> prototypes = {
    {ResourceType::Templates, new ResourceTemplate()},
    {ResourceType::Examples, new ResourceScript()},
};

// Generic scan creates items via prototype
ResourceItem* item = prototypes[type]->clone();
item->populateFromEntry(entry);
```
**Verdict:** Proper OOP but adds complexity

### 3.5 Conclusion on Dispatch

**Dispatch is unavoidable** because resource types genuinely differ in structure. The question is WHERE to put the dispatch logic:

1. **Scanner level** (current) - scanTemplatesAt, scanExamplesAt, etc.
2. **Factory level** - ResourceItemFactory::create()
3. **Item level** - Virtual populate methods

The current approach (scanner-level dispatch) is **actually fine** - it's explicit, debuggable, and follows Qt patterns.

---

## 4. Proposed Architecture: Custom Model Wrapping Inventory

### 4.1 Concept

Instead of:
```
Inventory → (copy data) → QStandardItemModel → QTreeView
```

Do this:
```
Inventory ← TemplatesListModel → QTreeView
              (wraps, doesn't copy)
```

### 4.2 TemplatesListModel Implementation

```cpp
/**
 * @brief Custom model that wraps TemplatesInventory
 * 
 * QTreeView sees this as its model, but data comes directly
 * from TemplatesInventory - no duplication.
 */
class TemplatesListModel : public QAbstractItemModel {
    Q_OBJECT
    
public:
    explicit TemplatesListModel(TemplatesInventory* inventory, QObject* parent = nullptr)
        : QAbstractItemModel(parent)
        , m_inventory(inventory)
        , m_keys(inventory->keys())  // Cache keys for index stability
    {}
    
    // === Required overrides ===
    
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override {
        if (parent.isValid()) return {};  // Flat list, no children
        if (row < 0 || row >= m_keys.size()) return {};
        return createIndex(row, column);
    }
    
    QModelIndex parent(const QModelIndex&) const override {
        return {};  // Flat list, no parent
    }
    
    int rowCount(const QModelIndex& parent = {}) const override {
        if (parent.isValid()) return 0;  // Flat list
        return m_keys.size();
    }
    
    int columnCount(const QModelIndex& = {}) const override {
        return 2;  // Name, ID
    }
    
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid()) return {};
        
        const QString& key = m_keys.at(index.row());
        ResourceTemplate tmpl = m_inventory->get(key).value<ResourceTemplate>();
        
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case 0: return tmpl.displayName();
                case 1: return tmpl.uniqueID();
            }
        }
        else if (role == Qt::UserRole) {
            return QVariant::fromValue(tmpl);
        }
        else if (role == Qt::ToolTipRole && index.column() == 0) {
            return QString("Path: %1\nTier: %2\nID: %3")
                .arg(tmpl.path())
                .arg(resourceMetadata::tierToString(tmpl.tier()))
                .arg(tmpl.uniqueID());
        }
        
        return {};
    }
    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
                case 0: return "Name";
                case 1: return "ID";
            }
        }
        return {};
    }
    
    // === Update methods ===
    
    void refresh() {
        beginResetModel();
        m_keys = m_inventory->keys();
        endResetModel();
    }
    
    void itemAdded(const QString& key) {
        int row = m_keys.size();
        beginInsertRows({}, row, row);
        m_keys.append(key);
        endInsertRows();
    }
    
    void itemRemoved(const QString& key) {
        int row = m_keys.indexOf(key);
        if (row >= 0) {
            beginRemoveRows({}, row, row);
            m_keys.removeAt(row);
            endRemoveRows();
        }
    }
    
private:
    TemplatesInventory* m_inventory;  // Non-owning
    QList<QString> m_keys;            // Cached for stable indexing
};
```

### 4.3 Usage in MainWindow

```cpp
// In MainWindow constructor or init:
m_scanner = new ResourceScanner();
m_scanner->scanToModel(locations);  // Populates inventories

// Create custom model wrapping the inventory
m_templatesModel = new TemplatesListModel(
    &m_scanner->templatesInventory(), 
    this
);

// Connect to view - works exactly like QStandardItemModel
m_templateTree->setModel(m_templatesModel);

// Selection handling - identical to before
connect(m_templateTree->selectionModel(), 
        &QItemSelectionModel::selectionChanged,
        this, &MainWindow::onInventorySelectionChanged);
```

### 4.4 Benefits

1. **No data duplication** - model reads directly from inventory
2. **Type-safe access** - get() returns ResourceTemplate directly
3. **Easy updates** - call refresh() after inventory changes
4. **Standard Qt** - works with QSortFilterProxyModel for search/filter
5. **Memory efficient** - no QStandardItem overhead

### 4.5 What Changes in ResourceScanner

Almost nothing! We keep:
- The dispatch table for scanning
- The per-type scan methods
- The inventories

We remove:
- `populateModel()` method entirely
- `populateExamplesModel()` and `populateTemplatesModel()`

The model creation moves to MainWindow (where it belongs).

---

## 5. Addressing Hard-Coded Folder Names

### 5.1 Current Code

```cpp
int ResourceScanner::scanTemplatesAt(const platformInfo::ResourceLocation& location)
{
    QString templatesPath = location.path() + "/templates";  // ← HARD-CODED
    return m_templatesInventory.addFolder(templatesPath, location);
}
```

### 5.2 Fix Using ResourceTypeInfo

```cpp
int ResourceScanner::scanTemplatesAt(const platformInfo::ResourceLocation& location)
{
    using resourceMetadata::ResourceType;
    using resourceMetadata::ResourceTypeInfo;
    
    // Get folder name from metadata
    const QString& folder = ResourceTypeInfo::s_resourceTypes[ResourceType::Templates].getSubDir();
    QString templatesPath = location.path() + "/" + folder;
    
    return m_templatesInventory.addFolder(templatesPath, location);
}
```

### 5.3 Why Not Generic Scanning?

Even with metadata-driven folder names, we still need per-type methods because:

1. **Templates:** Single call to `inventory.addFolder()`
2. **Examples:** Loop with category detection + loose file handling
3. **Fonts:** Loop filtering by extension
4. **Tests:** Loop with attachment detection

The scanning LOGIC differs, not just the folder name.

---

## 6. About scanUnknownAt

### 6.1 User's Request

> "Add a scanUnknownAt method for Unknown resType to dispatch table... all it should do is emit a warning... use the pretty name for the resource"

### 6.2 Implementation

```cpp
int ResourceScanner::scanUnknownAt(const platformInfo::ResourceLocation& location, 
                                    resourceMetadata::ResourceType type)
{
    QString prettyName = resourceMetadata::ResourceTypeInfo::getResTypeString(type);
    qWarning() << "ResourceScanner: Not scanning" << prettyName 
               << "- no scanner implemented for this resource type";
    return 0;
}
```

### 6.3 Dispatch Table Change

```cpp
// New: Use Unknown scanner as fallback for all types
// The dispatch lookup no longer needs contains() check

for (const auto& dirEntry : QDirListing(location.path(), ItFlag::DirsOnly)) {
    QString folderName = dirEntry.fileName();
    
    if (s_topLevelReverse.contains(folderName)) {
        ResourceType resType = s_topLevelReverse[folderName];
        
        // Direct dispatch - no contains() check needed
        (this->*(scannerDispatch.value(resType, &ResourceScanner::scanUnknownAt)))(location);
    }
}
```

**Wait** - this doesn't work cleanly because scanUnknownAt needs the resType parameter!

### 6.4 Better Approach: Default in Dispatch Table

```cpp
const QMap<ResourceType, ScannerFunc> scannerDispatch = {
    {ResourceType::Unknown, &ResourceScanner::scanUnknownAt},  // Fallback
    {ResourceType::Examples, &ResourceScanner::scanExamplesAt},
    {ResourceType::Templates, &ResourceScanner::scanTemplatesAt},
    // ... etc
};

// Then in loop - but Unknown doesn't know what type was requested!
```

**Problem:** The fallback scanner doesn't know which type was requested.

### 6.5 Solution: Log Before Dispatch

```cpp
// Check if this is a known resource folder
if (s_topLevelReverse.contains(folderName)) {
    ResourceType resType = s_topLevelReverse[folderName];
    
    // Dispatch to scanner if implemented
    if (scannerDispatch.contains(resType)) {
        (this->*(scannerDispatch[resType]))(location);
    } else {
        // Log warning with pretty name
        QString prettyName = ResourceTypeInfo::getResTypeString(resType);
        qWarning() << "ResourceScanner: Skipping" << prettyName 
                   << "at" << location.path() 
                   << "- no scanner implemented";
    }
}
```

This is actually cleaner than adding a dummy scanner method.

---

## 7. Summary of Recommendations

### 7.1 Immediate Changes (Low Risk)

1. ✅ Keep dispatch table architecture (it's correct)
2. ✅ Fix hard-coded folder names using ResourceTypeInfo
3. ✅ Add warning for unimplemented scanners (inline, not separate method)
4. ✅ Keep per-type scan methods (they're genuinely different)

### 7.2 Medium-Term Refactoring

1. Create `TemplatesListModel` custom model class
2. Remove `populateModel()` and helpers from ResourceScanner
3. Move model creation to MainWindow
4. Add `refresh()` method for inventory updates

### 7.3 What NOT to Change

1. Don't try to eliminate dispatch - it's needed
2. Don't create generic "scan all types" - logic differs
3. Don't remove inventories - they're useful for non-GUI access

---

## 8. Open Questions

1. **Should we create a base `ResourceListModel` class** that all resource types share?

2. **How to handle inventory updates?** 
   - Signals from inventory?
   - Manual refresh() calls?

3. **Do we need tree structure** (tier → resources) or is flat list sufficient?

4. **What about search/filter?**
   - QSortFilterProxyModel on top of custom model?
   - Built into custom model?

---

## 9. User Feedback - Simplification

### 9.1 Key Observations

> "There is no need to have scanner be a class... it has no data and its only function is to scan"

**Correct!** ResourceScanner has no persistent state. It's a stateless operation masquerading as a class.

> "Why not just put the info about the resource item in the model... why have a storage structure (inventory) AND also make a model?"

**Excellent point!** Looking at Qt's own patterns:
- `QFileSystemModel` - IS a model AND stores filesystem data
- `QSqlTableModel` - IS a model AND stores database data  
- `QStringListModel` - IS a model AND stores string list

There is no `QFileSystemInventory` that `QFileSystemModel` wraps!

> "Why not just make TemplatesInventory into a Qt model?"

**This is the correct Qt architecture.**

---

## 10. Revised Architecture: Inventory IS the Model

### 10.1 Current Architecture (Overly Complex)

```
main.cpp
  → ResourceScanner (class - unnecessary!)
    → m_templatesInventory (storage)
      → populateModel() (duplication!)
        → QStandardItemModel (more storage!)
          → QTreeView
```

**Problems:**
- Scanner class has no state - should be a function
- Data exists in THREE places: inventory + QStandardItemModel + QVariant in UserRole
- populateModel() is just copying data from one container to another

### 10.2 Proposed Architecture (Qt-Native)

```
main.cpp
  → scanResources() function (not a class)
    → TemplatesInventory (IS a QAbstractItemModel)
      → QTreeView
```

**Benefits:**
- No scanner class
- No separate model wrapper
- No data duplication
- No populateModel()
- Inventory IS the model - view reads directly from it

### 10.3 TemplatesInventory as QAbstractItemModel

```cpp
/**
 * @brief Template storage that IS a Qt model
 * 
 * Stores templates AND provides model interface for views.
 * No separate model wrapper needed.
 */
class TemplatesInventory : public QAbstractItemModel {
    Q_OBJECT
    
public:
    explicit TemplatesInventory(QObject* parent = nullptr);
    ~TemplatesInventory() = default;
    
    // ============ Inventory Operations (existing) ============
    
    bool addTemplate(const QDirListing::DirEntry& entry, 
                    const ResourceLocation& location);
    
    int addFolder(const QString& folderPath, 
                  const ResourceLocation& location);
    
    ResourceTemplate get(const QString& key) const;
    bool contains(const QString& key) const;
    int count() const;
    void clear();
    
    // ============ Scanning (moved from ResourceScanner) ============
    
    /**
     * @brief Scan a single location for templates
     * @param location Resource location to scan
     * @return Number of templates added
     */
    int scanLocation(const ResourceLocation& location);
    
    /**
     * @brief Scan multiple locations for templates
     * @param locations List of resource locations
     * @return Total number of templates added
     */
    int scanLocations(const QList<ResourceLocation>& locations);
    
    // ============ Model Interface (QAbstractItemModel) ============
    
    QModelIndex index(int row, int column, 
                      const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, 
                        int role = Qt::DisplayRole) const override;
    
private:
    QHash<QString, ResourceTemplate> m_templates;  // Primary storage
    QList<QString> m_keys;                         // For stable row indexing
};
```

### 10.4 Implementation Sketch

```cpp
// === Scanning (moved from ResourceScanner) ===

int TemplatesInventory::scanLocation(const ResourceLocation& location)
{
    using resourceMetadata::ResourceType;
    using resourceMetadata::ResourceTypeInfo;
    
    // Get folder name from metadata (not hard-coded!)
    const QString& folder = ResourceTypeInfo::s_resourceTypes[ResourceType::Templates].getSubDir();
    QString templatesPath = location.path() + "/" + folder;
    
    return addFolder(templatesPath, location);
}

int TemplatesInventory::scanLocations(const QList<ResourceLocation>& locations)
{
    int total = 0;
    for (const auto& location : locations) {
        total += scanLocation(location);
    }
    return total;
}

// === Model Interface ===

QModelIndex TemplatesInventory::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid()) return {};  // Flat list
    if (row < 0 || row >= m_keys.size()) return {};
    return createIndex(row, column);
}

QModelIndex TemplatesInventory::parent(const QModelIndex&) const
{
    return {};  // Flat list, no parent
}

int TemplatesInventory::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_keys.size();
}

int TemplatesInventory::columnCount(const QModelIndex&) const
{
    return 2;  // Name, ID
}

QVariant TemplatesInventory::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_keys.size()) return {};
    
    const QString& key = m_keys.at(index.row());
    const ResourceTemplate& tmpl = m_templates[key];
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return tmpl.displayName();
            case 1: return tmpl.uniqueID();
        }
    }
    else if (role == Qt::UserRole) {
        return QVariant::fromValue(tmpl);
    }
    else if (role == Qt::ToolTipRole && index.column() == 0) {
        return QString("Path: %1\nTier: %2\nID: %3")
            .arg(tmpl.path())
            .arg(resourceMetadata::tierToString(tmpl.tier()))
            .arg(tmpl.uniqueID());
    }
    
    return {};
}

QVariant TemplatesInventory::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return "Name";
            case 1: return "ID";
        }
    }
    return {};
}

// === Inventory Operations (update to maintain m_keys) ===

bool TemplatesInventory::addTemplate(const QDirListing::DirEntry& entry, 
                                     const ResourceLocation& location)
{
    // ... existing logic to create ResourceTemplate ...
    
    QString key = /* generate key */;
    
    if (m_templates.contains(key)) {
        return false;  // Duplicate
    }
    
    // Notify model BEFORE inserting
    int row = m_keys.size();
    beginInsertRows({}, row, row);
    
    m_templates.insert(key, tmpl);
    m_keys.append(key);
    
    endInsertRows();
    return true;
}
```

### 10.5 Usage in main.cpp

```cpp
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    // Discover resource locations
    QList<ResourceLocation> locations = discoverResourceLocations();
    
    // Create inventory (which IS the model)
    TemplatesInventory* templatesModel = new TemplatesInventory(&app);
    
    // Scan - inventory scans itself
    templatesModel->scanLocations(locations);
    
    // Create main window
    MainWindow window;
    
    // Connect model to view - DIRECTLY, no intermediaries
    window.setTemplatesModel(templatesModel);
    
    window.show();
    return app.exec();
}
```

### 10.6 What Gets Eliminated

| Component | Status | Reason |
|-----------|--------|--------|
| ResourceScanner class | ❌ DELETE | Stateless - scanning moves to inventory |
| ResourceScanner.hpp | ❌ DELETE | No longer needed |
| ResourceScanner.cpp | ❌ DELETE | No longer needed |
| populateModel() | ❌ DELETE | Inventory IS the model |
| populateTemplatesModel() | ❌ DELETE | Inventory IS the model |
| populateExamplesModel() | ❌ DELETE | Inventory IS the model |
| QStandardItemModel usage | ❌ DELETE | Inventory IS the model |

### 10.7 What Gets Modified

| Component | Change |
|-----------|--------|
| TemplatesInventory | Add QAbstractItemModel inheritance + scanLocation() |
| ExamplesInventory | Add QAbstractItemModel inheritance + scanLocation() |
| main.cpp | Simple function calls instead of scanner class |
| MainWindow | Accept model pointer, no internal scanner |

---

## 11. About the Model Wrapper Question

> "Does this have to be a class on its own?"

**Answer: NO!** 

The model interface is just 5-6 method overrides. When the inventory IS the model, there's no separate wrapper class at all.

The reason my earlier proposal had a separate `TemplatesListModel` class was because I was trying to WRAP an existing inventory without modifying it. But if we can modify the inventory class to inherit from QAbstractItemModel, we don't need a wrapper.

**Qt's pattern:**
- `QStringListModel` stores strings AND is a model
- `QFileSystemModel` indexes files AND is a model
- `TemplatesInventory` stores templates AND is a model

---

## 12. Dispatch Table for Scanning

Even without ResourceScanner class, we still need dispatch for scanning different resource types. This can be a simple function:

```cpp
// In resourceScanning namespace or main.cpp

void scanAllResources(const QList<ResourceLocation>& locations,
                      TemplatesInventory& templates,
                      ExamplesInventory& examples,
                      FontsInventory& fonts,
                      /* etc */)
{
    using resourceMetadata::ResourceType;
    using resourceMetadata::s_topLevelReverse;
    
    // Dispatch table using lambdas
    using ScanFunc = std::function<void(const ResourceLocation&)>;
    const QMap<ResourceType, ScanFunc> dispatch = {
        {ResourceType::Templates, [&](const auto& loc) { templates.scanLocation(loc); }},
        {ResourceType::Examples, [&](const auto& loc) { examples.scanLocation(loc); }},
        {ResourceType::Fonts, [&](const auto& loc) { fonts.scanLocation(loc); }},
        // ... etc
    };
    
    for (const auto& location : locations) {
        for (const auto& dirEntry : QDirListing(location.path(), QDirListing::IteratorFlag::DirsOnly)) {
            QString folderName = dirEntry.fileName();
            
            if (s_topLevelReverse.contains(folderName)) {
                ResourceType resType = s_topLevelReverse[folderName];
                
                if (dispatch.contains(resType)) {
                    dispatch[resType](location);
                } else {
                    // Unknown type warning
                    QString prettyName = ResourceTypeInfo::getResTypeString(resType);
                    qWarning() << "scanAllResources: Skipping" << prettyName 
                               << "- no scanner implemented";
                }
            }
        }
    }
}
```

Or even simpler - each inventory scans itself when given locations:

```cpp
// In main.cpp
templates.scanLocations(locations);
examples.scanLocations(locations);
fonts.scanLocations(locations);
```

Each inventory knows its own folder name from ResourceTypeInfo.

---

## 13. Final Architecture Summary

```
┌─────────────────────────────────────────────────────────┐
│                      main.cpp                           │
│                                                         │
│  locations = discoverResourceLocations()                │
│                                                         │
│  templatesModel = new TemplatesInventory()              │
│  templatesModel->scanLocations(locations)               │
│                                                         │
│  window.setTemplatesModel(templatesModel)               │
└─────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────┐
│            TemplatesInventory                           │
│            (QAbstractItemModel)                         │
│                                                         │
│  Storage:  QHash<QString, ResourceTemplate>             │
│  Index:    QList<QString> m_keys                        │
│                                                         │
│  Inventory: addTemplate(), get(), contains(), count()   │
│  Scanning:  scanLocation(), scanLocations()             │
│  Model:     index(), parent(), rowCount(), data(), etc  │
└─────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────┐
│                    QTreeView                            │
│                                                         │
│  setModel(templatesModel)  ← Direct connection!         │
│                                                         │
│  View calls model->data() to get display values         │
│  View calls model->rowCount() to know row count         │
│  No intermediate storage, no copying                    │
└─────────────────────────────────────────────────────────┘
```

**Eliminated:**
- ResourceScanner class
- Separate model wrapper class
- populateModel() methods
- Data duplication

**Retained:**
- Dispatch table pattern (now in function or each inventory)
- Per-type scanning logic (in each inventory's scanLocation)
- ResourceTypeInfo for folder names

---

## 14. Next Steps

1. Modify TemplatesInventory to inherit from QAbstractItemModel
2. Add scanLocation() method to TemplatesInventory
3. Update main.cpp to use new pattern
4. Delete ResourceScanner class
5. Update MainWindow to accept model pointer

### File Locations for Implementation

| File | Action | Path |
|------|--------|------|
| TemplatesInventory.hpp | MODIFY | `src/resourceInventory/TemplatesInventory.hpp` |
| TemplatesInventory.cpp | MODIFY | `src/resourceInventory/TemplatesInventory.cpp` |
| main.cpp | MODIFY | `src/app/main.cpp` |
| MainWindow.cpp | MODIFY | `src/app/mainwindow.cpp` |
| MainWindow.hpp | MODIFY | `src/app/mainwindow.hpp` |
| ResourceScanner.hpp | DELETE | `src/resourceScanning/resourceScanner.hpp` |
| ResourceScanner.cpp | DELETE | `src/resourceScanning/resourceScanner.cpp` |
| CMakeLists.txt | MODIFY | Remove ResourceScanner from build |

---

## Revision History

| Date | Author | Changes |
|------|--------|---------|
| 2026-01-19 | AI + User | Initial draft |
| 2026-01-19 | User feedback | Simplified: Inventory IS the model, eliminate scanner class |

