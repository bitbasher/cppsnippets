# Resource Architecture Analysis

**Date:** 2026-01-12  
**Author:** User + Agent  
**Status:** Analysis for Decision Making  

---

## User Requirements

> "we want to use a QVariant based structure for inventory  
> we want to replace the simplified QTreeWidget/QTreeItem in the GUI with the Qt model/view approach  
> we know that handling similar resources is already in view (color-schemes: .json files, editing in preferences, used in the editor and 2D view)  
> so adapting the resourceItem class to work in this future environment is the Better Way  
> AND we need a Qt bases structure for each template (currently Json objects) that allow us to map its members to edit widgets in the preference pane GUI and in the editor as well"

---

## Current Architecture Overview

### Three Parallel Implementations Exist

#### 1. **Old/Legacy System** (QTreeWidget-based)
**Location:** `src/resourceInventory/`
- `ResourceItem` - Base class with metadata (path, name, tier, type, access, etc.)
- `ResourceTemplate : ResourceItem` - Adds prefix, body, scopes, EditType
- `ResourceScript : ResourceItem` - Adds attachments support
- `ResourceTreeWidget` - Qt tree widget for display
- `ResourceTreeItem` - Wrapper around ResourceItem for QTreeWidget
- `ResourceScanner` - Static methods returning `QVector<ResourceItem>`
- `ResourceInventoryManager` - Coordinates scanning, populates tree widgets

**Pattern:**
```cpp
// Scanner creates temporary vectors
QVector<ResourceItem> results;
for (file : files) {
    ResourceItem item(path, type, tier);
    results.append(item);
}

// Results passed to tree widget
for (auto& item : results) {
    tree->addResource(item);  // Copies into QTreeWidgetItem
}
// Original vector discarded
```

**Usage:** Currently active in mainwindow.cpp

---

#### 2. **Modern System** (QAbstractItemModel-based)
**Location:** `src/resourceInventory/`
- `TemplateTreeModel : QAbstractItemModel` - **Already implemented!**
- `TemplateTreeNode` - Internal tree node (Tier → Location → Template)
- Uses Model/View pattern properly
- Works with `ResourceStore` (from bad-refactoring branch)

**Pattern:**
```cpp
// Model/View approach
TemplateTreeModel model;
model.setResourceStore(&store);

QTreeView* view = new QTreeView;
view->setModel(&model);
```

**Custom Roles Defined:**
```cpp
enum Roles {
    PathRole = Qt::UserRole + 1,
    TierRole,
    LocationKeyRole,
    IsLibraryRole,
    NodeTypeRole,
    ResourceRole  // Full DiscoveredResource
};
```

**Status:** Exists but not currently used in production code

---

#### 3. **QDirListing Experiment** (Streaming scanner)
**Location:** `cppsnippets-bad-refactoring/src/resInventory/`
- `ResourceScannerDirListing` - Qt 6.8+ streaming scanner
- `DiscoveredResource` - Lightweight struct (not class-based)
- `ResourceStore` - Typed storage with signals
- Memory-efficient callback-based scanning

**Pattern:**
```cpp
struct DiscoveredResource {
    QString path;
    QString name;
    QString category;
    QString locationKey;
    ResourceType type;
    ResourceTier tier;
    QDateTime lastModified;
    qint64 size;
};

scanner.scanLocation(path, tier, key, 
    [&store](const DiscoveredResource& res) {
        store.addResource(res);  // Signal emitted
    });

// ResourceStore emits signals
emit resourceAdded(resource);
// TemplateTreeModel listens and rebuilds tree
```

**Status:** Proof-of-concept in separate branch

---

## Role Analysis

### Q: "What is the role of templateScanner? We already have resourceScanner yes?"

**Answer:**

#### ResourceScanner (Generic)
- **Purpose:** Scans for **all resource types** (color-schemes, fonts, libraries, templates, examples)
- **Returns:** `QVector<ResourceItem>` (base class, generic metadata)
- **Location:** `src/resourceInventory/` (NOT moved to resourceScanning)
- **Pattern:** One-size-fits-all scanner, type-agnostic

**Methods:**
```cpp
QVector<ResourceItem> scanColorSchemes(...);
QVector<ResourceItem> scanRenderColors(...);
QVector<ResourceItem> scanFonts(...);
QVector<ResourceItem> scanTemplates(...);  // Generic scan
```

#### TemplateScanner (Specialized)
- **Purpose:** Scans **only templates** with full JSON parsing
- **Returns:** `QList<ResourceTemplate>` (specialized subclass with template-specific fields)
- **Location:** `src/resourceScanning/` (moved in Phase 2 refactoring)
- **Pattern:** Deep metadata extraction

**Methods:**
```cpp
static QList<ResourceTemplate> scanLocation(location);
static QList<ResourceTemplate> scanLocations(locations);
static bool validateTemplateJson(json);
static ResourceTemplate extractMetadata(json, path, location);
```

**Key Difference:**
- `ResourceScanner::scanTemplates()` - Shallow: finds .json files, returns basic ResourceItem
- `TemplateScanner::scanLocation()` - Deep: parses JSON, validates, extracts prefix/body/scopes/editType

**Redundancy:** YES - both scan templates, but at different depths.

**Recommended Consolidation:**
- Keep `TemplateScanner` for deep template analysis
- Remove template scanning from `ResourceScanner` (delegate to TemplateScanner)
- `ResourceScanner` handles generic resources (fonts, colors, libraries)

---

### Q: "What are the roles of ResourceTemplate, ResourceScript, ResourceItem? Only in the GUI?"

**Answer:**

#### ResourceItem (Base Class)
**Role:** Data Transfer Object (DTO) - NOT a storage container as originally intended

**Used By:**
1. **Scanners** - Create temporary ResourceItem instances
2. **Tree Widgets** - Copy into QTreeWidgetItem for display
3. **Temporary Vectors** - `QVector<ResourceItem>` returned from scanners, then discarded

**NOT Used For:**
- Long-term storage in QList/QTree containers (contrary to original design)
- No `QList<ResourceItem>` storage classes exist

**Fields:**
```cpp
QString m_path, m_name, m_displayName, m_description, m_category;
ResourceType m_type;
ResourceTier m_tier;
ResourceAccess m_access;
bool m_enabled, m_exists;
QDateTime m_lastModified;
QString m_sourcePath, m_sourceLocationKey;
```

#### ResourceTemplate (Subclass)
**Role:** Template-specific metadata container

**Used By:**
1. **TemplateScanner** - Returns `QList<ResourceTemplate>`
2. **TemplateParser** - Parses legacy/JSON templates into ResourceTemplate
3. **Template Management** - TemplateManager operates on ResourceTemplate objects
4. **GUI** - Can be stored in tree widgets (through base ResourceItem)

**Additional Fields:**
```cpp
QString m_prefix;           // Completion prefix (e.g., "module", "function")
QStringList m_scopes;       // Editor scopes where active
EditType m_editType;        // Module/Function/Statement/etc.
EditSubtype m_editSubtype;  // Specific variant
QString m_format;           // "legacy" or "standard"
QString m_source;           // "builtin" or "user"
QString m_version;          // Template version
QString m_body;             // OpenSCAD code
QString m_rawText;          // Original JSON text
```

**Use Cases:**
- Template editor needs body, prefix, scopes
- Completion engine needs prefix, editType, scopes
- Preferences UI needs all fields for editing

#### ResourceScript (Subclass)
**Role:** Script/example-specific metadata

**Used By:**
1. **ResourceScanner::scanScriptWithAttachments()** - Returns ResourceScript
2. **Examples browser** - Display scripts with attached files

**Additional Fields:**
```cpp
QStringList m_attachments;  // Associated files (e.g., .png, .stl)
```

**Not GUI-Only:** These classes represent **data model** across application:
- Scanner → Parser → Manager → Editor/GUI
- ResourceItem = DTO between layers
- ResourceTemplate = Domain object for template functionality
- ResourceScript = Domain object for examples/scripts

---

## Architectural Recommendations

### Path Forward: Modernize with Model/View

#### Phase 1: Adopt TemplateTreeModel (Already Built!)
**Status:** Code exists, not connected to GUI

**Actions:**
1. Replace `ResourceTreeWidget` usage in mainwindow.cpp with `TemplateTreeModel + QTreeView`
2. Connect `ResourceStore` as data source
3. Remove ResourceTreeItem wrapper (no longer needed)

**Benefit:** Proper separation of data (model) and presentation (view)

---

#### Phase 2: Replace ResourceItem with DiscoveredResource
**Rationale:** 
- `ResourceItem` is class-based, designed for inheritance
- `DiscoveredResource` is struct-based, lightweight, QVariant-friendly
- Model/View works better with POD structs + QVariant roles

**Actions:**
1. Migrate scanners to return `DiscoveredResource` structs
2. Use `ResourceStore` for storage (already has signals for model updates)
3. `TemplateTreeModel::data()` returns QVariant for each role:
   ```cpp
   case PathRole: return variant.fromValue(res.path);
   case TierRole: return variant.fromValue(res.tier);
   case ResourceRole: return variant.fromValue(res);  // Full struct
   ```

**Benefit:** Natural fit for Qt Model/View + QVariant architecture

---

#### Phase 3: Specialized Metadata Handling
**Problem:** Templates need rich metadata (prefix, body, scopes, editType)  
**Solution:** Extend DiscoveredResource with QVariant payload

**Option A: Union/Tagged Variant**
```cpp
struct DiscoveredResource {
    // ... basic fields ...
    QVariant metadata;  // Type-specific data
};

struct TemplateMetadata {
    QString prefix;
    QStringList scopes;
    EditType editType;
    QString body;
    // ... etc
};

// Usage:
resource.metadata = QVariant::fromValue(TemplateMetadata{...});
```

**Option B: Separate Stores**
```cpp
ResourceStore baseStore;           // All resources (lightweight)
TemplateMetadataStore templateStore;  // Rich template data, keyed by path

// Model queries both:
QString path = baseStore.resourceAt(index).path;
TemplateMetadata meta = templateStore.metadataForPath(path);
```

**Recommendation:** Option B - keeps base storage lightweight, template store separate

---

#### Phase 4: Qt Property-Based Template Editing
**Goal:** Map template fields to edit widgets using Qt's property system

**Approach:**
```cpp
class TemplateMetadata : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix NOTIFY prefixChanged)
    Q_PROPERTY(QStringList scopes READ scopes WRITE setScopes NOTIFY scopesChanged)
    Q_PROPERTY(QString body READ body WRITE setBody NOTIFY bodyChanged)
    // ... etc

signals:
    void prefixChanged();
    void scopesChanged();
    void bodyChanged();

private:
    QString m_prefix;
    QStringList m_scopes;
    QString m_body;
};
```

**Benefits:**
1. QDataWidgetMapper automatically connects properties to widgets
2. Change signals trigger auto-save
3. Undo/redo support via QUndoStack
4. Works in both preferences and editor

**GUI Binding:**
```cpp
QDataWidgetMapper* mapper = new QDataWidgetMapper;
mapper->setModel(templateModel);
mapper->addMapping(prefixLineEdit, TemplateModel::PrefixColumn);
mapper->addMapping(bodyTextEdit, TemplateModel::BodyColumn);
mapper->setCurrentIndex(selectedRow);
// Auto-syncs widgets ↔ model
```

---

## Scanner Consolidation Plan

### Current Redundancy

| Scanner | Returns | Purpose |
|---------|---------|---------|
| `ResourceScanner::scanTemplates()` | `QVector<ResourceItem>` | Generic file listing |
| `TemplateScanner::scanLocation()` | `QList<ResourceTemplate>` | Deep JSON parsing |

### Proposed Consolidation

**1. ResourceScanner → Generic Resources Only**
- Color schemes, fonts, libraries, examples
- Returns `DiscoveredResource` (lightweight)
- No template scanning

**2. TemplateScanner → Template Specialist**
- Deep JSON parsing, validation
- Returns rich template metadata
- Stores in `TemplateMetadataStore`

**3. Unified Interface**
```cpp
// Generic resources
ResourceStore store;
scanner.scanLocation(path, ResourceType::Font, tier, 
    [&store](const DiscoveredResource& res) {
        store.addResource(res);
    });

// Templates (specialized)
TemplateMetadataStore templateStore;
QList<TemplateMetadata> templates = TemplateScanner::scanLocation(location);
for (auto& tmpl : templates) {
    templateStore.addTemplate(tmpl);
    store.addResource(tmpl.toDiscoveredResource());  // Lightweight entry
}
```

---

## Decision Matrix

### Keep ResourceItem?

| Criterion | Keep | Simplify | Remove |
|-----------|------|----------|--------|
| **Polymorphism Value** | ✅ ResourceTemplate/ResourceScript use inheritance | ⚠️ Can use composition instead | ❌ Lose type safety |
| **Model/View Fit** | ❌ Class-based, not QVariant-friendly | ✅ Convert to struct | ✅ Use DiscoveredResource |
| **Storage Simplicity** | ❌ Wrapper overhead (ResourceTreeItem) | ✅ Flatten hierarchy | ✅ POD struct storage |
| **Code Migration** | ✅ Minimal changes needed | ⚠️ Moderate refactoring | ❌ Major rewrite |

**Recommendation:** **Simplify** - Convert to struct-based design, use QVariant for specialized metadata

---

## Implementation Phases

### ✅ Phase 0: Documentation (This Document)
- Understand current architecture
- Identify redundancies
- Plan migration path

### Phase 1: Connect TemplateTreeModel (Low Risk, High Value)
**Goal:** Use existing Model/View infrastructure

**Steps:**
1. Instantiate `TemplateTreeModel` in mainwindow.cpp
2. Replace `ResourceTreeWidget` with `QTreeView + TemplateTreeModel`
3. Test with existing ResourceStore (from bad-refactoring branch)

**Estimated Effort:** 1-2 hours  
**Risk:** Low - code already exists  
**Benefit:** Proper Model/View separation

---

### Phase 2: Consolidate Scanners (Medium Risk, Reduces Complexity)
**Goal:** Eliminate template scanning redundancy

**Steps:**
1. Remove `ResourceScanner::scanTemplates()` method
2. Update callers to use `TemplateScanner` directly
3. Document scanner responsibilities clearly

**Estimated Effort:** 2-3 hours  
**Risk:** Medium - affects multiple callsites  
**Benefit:** Clear separation of concerns

---

### Phase 3: Migrate to DiscoveredResource (High Risk, High Value)
**Goal:** Replace class-based ResourceItem with struct-based design

**Steps:**
1. Copy `DiscoveredResource` struct from bad-refactoring branch
2. Update ResourceScanner to return `DiscoveredResource`
3. Update ResourceStore to store `DiscoveredResource`
4. Remove ResourceItem/ResourceTreeItem wrappers
5. Update TemplateTreeModel to work with DiscoveredResource

**Estimated Effort:** 1-2 days  
**Risk:** High - touches entire resource system  
**Benefit:** QVariant-based architecture, lighter memory footprint

---

### Phase 4: Template Metadata with Qt Properties (Medium Risk, Enables Editor)
**Goal:** Rich template editing in GUI

**Steps:**
1. Create `TemplateMetadata : QObject` with Q_PROPERTY declarations
2. Create `TemplateMetadataStore` for rich data
3. Use QDataWidgetMapper for property ↔ widget binding
4. Implement in preferences dialog
5. Extend to editor for inline editing

**Estimated Effort:** 2-3 days  
**Risk:** Medium - new functionality, testing needed  
**Benefit:** Enables template editing, auto-save, undo/redo

---

## Answers to Your Questions

### 1. "What exactly is the role of templateScanner?"
**Answer:** Deep JSON parsing specialist for templates. Returns `QList<ResourceTemplate>` with full metadata (prefix, body, scopes, editType). Complements (but duplicates) ResourceScanner's shallow template listing.

**Recommended Action:** Keep TemplateScanner, remove template scanning from ResourceScanner.

---

### 2. "We already have resourceScanner yes?"
**Answer:** Yes, but it does **shallow scanning** (file paths only). TemplateScanner does **deep scanning** (parse JSON, validate, extract metadata).

**Recommended Action:** Let each scanner do what it's best at:
- ResourceScanner: Generic file discovery (fonts, colors, libraries)
- TemplateScanner: Template-specific deep analysis

---

### 3. "What are the roles of ResourceTemplate, ResourceScript, ResourceItem? Only in the GUI?"
**Answer:** **No, not GUI-only.** They are **domain objects** used throughout:
- **Scanners** create them
- **Parsers** populate them
- **Managers** operate on them
- **GUI** displays them

**Current Issue:** ResourceItem used as DTO (data transfer object), not storage container.

**Future Design:** Replace with struct-based `DiscoveredResource` + specialized metadata stores.

---

## Next Steps

**Immediate Actions:**
1. ✅ Review this document
2. Choose a phase to start (recommendation: Phase 1 - connect TemplateTreeModel)
3. Create planning doc for chosen phase
4. Begin implementation

**Priority Recommendation:**
- **Phase 1 first** - Low risk, proves Model/View approach works
- **Phase 2 second** - Reduces scanner confusion
- **Phase 3 third** - Big refactor, do when confident
- **Phase 4 last** - Enables advanced features

---

## References

**Existing Code:**
- [TemplateTreeModel.hpp](d:/repositories/cppsnippets/cppsnippets/src/resourceInventory/templateTreeModel.hpp) - Already implemented Model/View
- [ResourceStore](d:/repositories/cppsnippets/cppsnippets-bad-refactoring/src/resInventory/resourceStore.h) - Signal-based storage
- [DiscoveredResource](d:/repositories/cppsnippets/cppsnippets-bad-refactoring/src/resInventory/resourceScannerDirListing.h) - Lightweight struct

**Qt Documentation:**
- QAbstractItemModel - Custom models
- QDataWidgetMapper - Property-to-widget binding
- QVariant - Type-safe value storage

---

**Decision Point:** You have everything needed for a modern Qt Model/View architecture. The question is: which phase to start with?

My recommendation: **Phase 1** - It's already built, just needs connecting. Low risk, immediate value.
