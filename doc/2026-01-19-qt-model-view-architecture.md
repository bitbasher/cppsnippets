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

### 5.3 Conclusion on Per-Resource-Type Methods

**Dispatch is unavoidable** because resource types genuinely differ in structure:

| Resource | Storage Structure | Item Construction | Metadata Extraction |
|----------|------------------|-------------------|---------------------|
| Templates | Flat JSON files | Simple ResourceTemplate from JSON | Parse .json fields |
| Examples | Folders with attachments | ResourceScript + dependencies | Read folder structure |
| Fonts | Files by extension | ResourceFont from file attributes | Font metadata extraction |
| Tests | Hierarchical structure | ResourceScript with test harness | Parse test metadata |
| Shaders | Multi-file sets | ResourceShader + variants | GLSL parsing |

**Decision:** Each resource type gets its own inventory class with `scanLocation()` and `addFolder()` methods. This is the correct approach because:
- ✅ Each type has genuinely different logic
- ✅ Logic lives in the appropriate inventory class
- ✅ No generic scanner class needed
- ✅ TemplatesInventory IS the model for templates, etc.

**NOT a limitation** - it's proper separation of concerns. Just as QFileSystemModel knows how to scan filesystems and QSqlTableModel knows how to query databases, TemplatesInventory knows how to scan template resources.

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

## 6. Scanning Strategy: Dispatch Table in main.cpp

### 6.1 Scanning Architecture

The resource scanning has three levels:

```
Level 1: Loop over all locations (ResourceLocation objects)
  ↓
Level 2: Look for resource-type folders in each location
         (templates/, examples/, fonts/, shaders/, etc.)
  ↓
Level 3: Scan the resource folder for items
         (add individual templates, examples, fonts, etc.)
```

### 6.2 Dispatch Table Design

Use a **const compile-time dispatch table** mapped by ResourceType to inventory instances:

```cpp
// In main.cpp or resourceManager()
using ScannerFunc = int(*)(const QDirListing::DirEntry&, 
                           const platformInfo::ResourceLocation&);

static const QHash<resourceMetadata::ResourceType, ScannerFunc> M_SCANNER_MAP = {
    { ResourceType::Templates,   &TemplatesInventory::addTemplate },
    { ResourceType::Examples,    &ExamplesInventory::addExample },
    { ResourceType::Fonts,       &FontsInventory::addFont },
    { ResourceType::Shaders,     &ShadersInventory::addShader },
    { ResourceType::Tests,       &TestsInventory::addTest },
    { ResourceType::Translations, &TranslationsInventory::addTranslation },
    { ResourceType::Unknown,     &scanUnknownAt }
};
```

### 6.3 The scanUnknownAt() Function

For resource folders that don't have a scanner, we need `scanUnknownAt()`:

```cpp
/**
 * @brief Handler for unknown/unimplemented resource types
 * 
 * Called when we encounter a resource folder (templates/, fonts/, etc.)
 * that has no scanner implemented yet.
 */
int scanUnknownAt(const QDirListing::DirEntry& entry, 
                  const platformInfo::ResourceLocation& location)
{
    // Get the pretty name for the resource type from folder name
    QString folderName = entry.fileName();
    
    if (ResourceTypeInfo::s_topLevelReverse.contains(folderName)) {
        ResourceType type = ResourceTypeInfo::s_topLevelReverse[folderName];
        QString prettyName = ResourceTypeInfo::getResTypeString(type);
        qWarning() << "No scanner for resource type:" << prettyName 
                   << "at" << location.path();
    } else {
        qWarning() << "Unknown folder:" << folderName 
                   << "at" << location.path();
    }
    
    return 0;  // No items added
}
```

**Purpose:**
- Provides a position in dispatch table for every ResourceType
- Eliminates the need for contains() checks
- Logs information about unprocessed resource folders
- Could be enhanced in future to gather metadata about unscanned resources

### 6.4 Direct Dispatch Approach

Since each inventory class has its own scanning methods (like `TemplatesInventory::addFolder()`), the dispatch in main.cpp is clean and direct:

```cpp
// In resourceManager() after discovering locations

for (const auto& location : allLocations) {
    QDir locationDir(location.path());
    const QStringList folders = locationDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QString& folderName : folders) {
        // Check if this folder name matches a known resource type
        if (ResourceTypeInfo::s_topLevelReverse.contains(folderName)) {
            ResourceType type = ResourceTypeInfo::s_topLevelReverse[folderName];
            
            // Get the inventory for this type
            if (auto* inventory = getInventoryForType(type)) {
                // Dispatch directly to inventory's addFolder method
                inventory->addFolder(location.path() + "/" + folderName, location);
            }
        } else {
            // Log unknown folder
            qWarning() << "Unknown folder:" << folderName << "at" << location.path();
        }
    }
}
```

### 6.5 Why This Works

1. **No special cases** - Every ResourceType has an entry in the dispatch table
2. **No contains() overhead** - Direct hash lookup
3. **Direct method calls** - No intermediate function pointers
4. **Per-inventory logic** - Each inventory knows how to scan its own resources
5. **Logging before dispatch** - We know exactly what we're processing before calling

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

## 8. Open Questions for Future Work

1. **Should other inventory classes also inherit QAbstractItemModel?**
   - Currently: TemplatesInventory inherits QAbstractItemModel
   - Question: Do Examples, Fonts, Shaders, Tests also need model interfaces?
   - Answer depends on GUI requirements for each resource type

2. **How to manage multiple inventory instances?**
   - Should they be created in main::resourceManager()?
   - Should they live in a manager class?
   - Should they have parent-child relationships?

3. **Dispatch table implementation options:**
   - Option A: Static const hash in main.cpp (simple, direct)
   - Option B: Factory function getInventoryForType(ResourceType) → pointer
   - Option C: Global registry pattern
   - User preference: Option A (static const, compile-time known)

4. **Resource scanning completeness:**
   - Currently: Only TemplatesInventory is created and used
   - Next phase: Add other inventory types as scanners are implemented
   - Priority: Based on GUI requirements (examples/templates first, others as needed)

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

## 10. Implementation: Using TemplatesInventory::addFolder() Directly

### 10.1 User's Clarification

The original proposal showed `scanLocation()` as an intermediate method on the inventory:

```cpp
int TemplatesInventory::scanLocation(const ResourceLocation& location) {
    // ... build path, call addFolder() ...
}
```

**User's feedback:** This adds unnecessary indirection. The `resourceManager()` function in main.cpp already handles Level 1 (iterate locations) and Level 2 (find resource folders). Level 3 (collect items) should call `addFolder()` directly.

### 10.2 Three-Level Scanning Architecture

```
Level 1: for (each location in allLocations)
  ↓
Level 2: for (each folder in location)  ← main.cpp handles this
  ↓
Level 3: inventory->addFolder(folderPath, location)  ← Direct call
```

Each level has distinct responsibilities:
- **Level 1**: Discover locations (handled by ResourcePaths discovery)
- **Level 2**: Identify resource-type folders (main.cpp loop with ResourceTypeInfo)
- **Level 3**: Scan items in resource folder (TemplatesInventory::addFolder())

### 10.3 Implementation in main.cpp

```cpp
/**
 * @brief Discover and scan all resource locations
 * 
 * Three-level nested loop:
 * 1. Iterate over all discovered resource locations
 * 2. Look for resource-type folders (templates/, examples/, fonts/, etc.)
 * 3. Call the appropriate inventory's addFolder() method directly
 * 
 * @return Populated TemplatesInventory model, or nullptr on failure
 */
resourceInventory::TemplatesInventory* resourceManager() {
    try {
        qDebug() << "Discovering resource locations...";
        
        // Discover all qualified search paths
        pathDiscovery::ResourcePaths pathDiscovery;
        QList<pathDiscovery::PathElement> discoveredPaths = pathDiscovery.qualifiedSearchPaths();
        
        // Convert to ResourceLocation
        QList<platformInfo::ResourceLocation> allLocations;
        for (const auto& pathElem : discoveredPaths) {
            allLocations.append(platformInfo::ResourceLocation(pathElem.path(), pathElem.tier()));
        }
        
        qDebug() << "Found" << allLocations.size() << "resource locations";
        
        // Create inventory instance (IS a QAbstractItemModel)
        auto* templatesModel = new resourceInventory::TemplatesInventory();
        
        // ===== LEVEL 1: Iterate all locations =====
        for (const auto& location : allLocations) {
            qDebug() << "Scanning location:" << location.path();
            
            // ===== LEVEL 2: Find resource folders in this location =====
            QDir locationDir(location.path());
            const QStringList folders = locationDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            
            for (const QString& folderName : folders) {
                // Check if this folder is a known resource type
                if (ResourceTypeInfo::s_topLevelReverse.contains(folderName)) {
                    ResourceType resType = ResourceTypeInfo::s_topLevelReverse[folderName];
                    
                    // ===== LEVEL 3: Scan this resource folder =====
                    if (resType == ResourceType::Templates) {
                        QString templatesPath = location.path() + "/" + folderName;
                        int added = templatesModel->addFolder(templatesPath, location);
                        qDebug() << "Added" << added << "templates from" << folderName;
                    } else {
                        // For other resource types - dispatch to appropriate inventory
                        // (To be implemented as other inventories are added)
                        QString prettyName = ResourceTypeInfo::getResTypeString(resType);
                        qDebug() << "Skipping" << prettyName << "- scanner not yet implemented";
                    }
                } else {
                    // Unknown folder - might be project folder, cache, etc.
                    qDebug() << "Skipping unknown folder:" << folderName;
                }
            }
        }
        
        qDebug() << "Inventory populated with" << templatesModel->count() << "templates";
        
        return templatesModel;
    } catch (const std::exception& e) {
        qCritical() << "Resource discovery failed:" << e.what();
        return nullptr;
    } catch (...) {
        qCritical() << "Resource discovery failed with unknown error";
        return nullptr;
    }
}
```

### 10.4 Key Design Points

1. **TemplatesInventory IS the model** - Inherits QAbstractItemModel
2. **No intermediate scanLocation()** - main.cpp calls addFolder() directly
3. **Triple-nested loop structure:**
   - Outer loop: Iterate all locations
   - Middle loop: Find resource folders in each location (using ResourceTypeInfo)
   - Inner loop: Items in resource folder (inside addFolder())
4. **Dispatch point:** For each folder type, call appropriate inventory's addFolder()
5. **Expandable:** As other inventories are implemented, extend the middle loop dispatch

### 10.5 Adding Other Resource Types

When ready to add Examples, Fonts, Shaders, etc., expand the middle loop:

```cpp
// Create additional inventory instances
auto* examplesModel = new resourceInventory::ExamplesInventory();
auto* fontsModel = new resourceInventory::FontsInventory();

// In the folder loop, expand dispatch:
for (const QString& folderName : folders) {
    if (ResourceTypeInfo::s_topLevelReverse.contains(folderName)) {
        ResourceType resType = ResourceTypeInfo::s_topLevelReverse[folderName];
        QString resourcePath = location.path() + "/" + folderName;
        
        switch (resType) {
            case ResourceType::Templates:
                templatesModel->addFolder(resourcePath, location);
                break;
            case ResourceType::Examples:
                examplesModel->addFolder(resourcePath, location);
                break;
            case ResourceType::Fonts:
                fontsModel->addFolder(resourcePath, location);
                break;
            // ... etc for other types
            default:
                qDebug() << "No scanner for" << folderName;
                break;
        }
    }
}
```

### 10.6 Status of Proposed Deletions

Files mentioned in original Section 10.6 that are **ALREADY DELETED**:
- ✅ ResourceScanner.hpp 
- ✅ ResourceScanner.cpp
- ✅ resourceScanning/ folder

**Never existed in this codebase:**
- populateModel() methods - Not implemented
- populateTemplatesModel() methods - Not implemented
- populateExamplesModel() methods - Not implemented
- QStandardItemModel usage in inventory - TemplatesInventory already inherits QAbstractItemModel

**Current status:** The architecture is already correct. TemplatesInventory IS the model.

---

## 11. Dispatch Table Approach in main.cpp

Even without ResourceScanner class, we use a dispatch table when multiple inventory types need to be scanned.

### 11.1 Static Const Dispatch Table

Since scanners are determined at compile-time (never added dynamically):

```cpp
// In main.cpp - static compile-time dispatch table
static const QHash<resourceMetadata::ResourceType, 
                   std::function<int(resourceInventory::TemplatesInventory&, 
                                    const QString&, 
                                    const platformInfo::ResourceLocation&)>>
    M_SCANNER_MAP = {
        // Future: Add more inventory types
        // { ResourceType::Examples, [](auto& inv, auto& path, auto& loc) { return inv.addFolder(path, loc); } },
    };
```

### 11.2 Future Dispatch Expansion

As more inventory types are added:

```cpp
// When Examples scanner is ready:
auto* examplesModel = new resourceInventory::ExamplesInventory();

// Extend the dispatch:
for (const QString& folderName : folders) {
    if (ResourceTypeInfo::s_topLevelReverse.contains(folderName)) {
        ResourceType resType = ResourceTypeInfo::s_topLevelReverse[folderName];
        QString resourcePath = location.path() + "/" + folderName;
        
        switch (resType) {
            case ResourceType::Templates:
                templatesModel->addFolder(resourcePath, location);
                break;
            case ResourceType::Examples:
                examplesModel->addFolder(resourcePath, location);
                break;
            // ... more as implemented
        }
    }
}
```

---

## 12. Summary of Architecture

### 12.1 Three-Level Scanning

```
main.cpp::resourceManager()
│
├─ Level 1: Iterate all locations (ResourceLocation objects)
│   │
│   └─ Level 2: Find resource folders in each location
│       │       (using ResourceTypeInfo::s_topLevelReverse)
│       │
│       └─ Level 3: Call inventory->addFolder(folderPath, location)
│                   (TemplatesInventory, ExamplesInventory, etc.)
```

### 12.2 Key Design Decisions

| Decision | Why |
|----------|-----|
| No ResourceScanner class | Scanning logic lives in inventory classes |
| Direct addFolder() calls | No intermediate methods needed |
| Triple-nested loop in main | Clear responsibility separation |
| scanUnknownAt() for unimplemented types | Provides logging for unknown folders |
| Dispatch via switch/if | Clear, debuggable, type-safe |
| Each inventory inherits QAbstractItemModel | IS-a model, not wraps-a model |
| Compile-time known dispatchers | No dynamic registration needed |

### 12.3 No Need for scanAllResources()

The original proposal had a `scanAllResources()` method, but `main::resourceManager()` already does this. No separate function needed.

---

## 13. Code Readability

The triple-nested loop in main.cpp is **intentionally flat and visible**:

```cpp
// VISIBLE: What we're doing at each level
for (location : allLocations) {                    // Level 1
    for (folder : location.folders()) {             // Level 2
        inventory->addFolder(folder, location);     // Level 3
    }
}
```

This is better than hiding the logic in separate methods. We can **see** the scanning strategy:
1. Iterate all locations
2. Identify resource folders in each
3. Scan items in each folder

---

## 14. Next Steps & Implementation

### 14.1 Immediate: Update main.cpp

Implement the triple-nested loop in `resourceManager()` as shown in Section 10.3:
- Keep TemplatesInventory creation
- Add the nested loop structure
- Dispatch to addFolder() directly
- Handle unknown folders gracefully

### 14.2 When Adding More Resource Types

For each new inventory (Examples, Fonts, Shaders, Tests, Translations):

1. Create the inventory instance in resourceManager()
2. Extend the switch statement for that ResourceType
3. Call its addFolder() method

### 14.3 File Changes

| File | Change | Status |
|------|--------|--------|
| main.cpp | Update resourceManager() with triple-nested loop | Required |
| TemplatesInventory.hpp | No changes needed | Optional: Remove scanLocation() if not using |
| TemplatesInventory.cpp | No changes needed | Optional: Remove scanLocation() if not using |
| MainWindow | No changes needed | Already accepts inventory |
| CMakeLists.txt | No changes needed | ResourceScanner already removed |

---

## Revision History

| Date | Author | Changes |
|------|--------|---------|
| 2026-01-19 | AI + User | Initial draft - proposed separate models |
| 2026-01-20 | User feedback | Simplified: Direct addFolder() calls, triple-nested loop in main |
| 2026-01-20 | User corrections | Clarified dispatch strategy, removed unnecessary abstraction |

