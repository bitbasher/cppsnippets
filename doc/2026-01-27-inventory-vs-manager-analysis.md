# TemplatesInventory vs TemplateManager: Feature Analysis

**Goal:** Clarify responsibility split between storage/model (TemplatesInventory) and business logic/CRUD (TemplateManager)

## Current State Analysis

### TemplatesInventory (resourceInventory namespace)
**Core Purpose:** Qt Model for template inventory storage  
**Current Location:** `src/resourceInventory/TemplatesInventory.{hpp,cpp}`  
**Inherits:** `QAbstractItemModel`

| Feature | Responsibility | Current Implementation |
|---------|---|---|
| **Storage** | QHash-based inventory with unique keys | ✅ `QHash<QString, ResourceTemplate> m_templates` |
| **Adding** | `addTemplate(DirEntry, ResourceLocation)` | ✅ Reads JSON, validates, generates unique ID via ResourceIndexer |
| **Adding Folders** | `addFolder(DirEntry, ResourceLocation)` | ✅ Scans .json files, calls addTemplate for each |
| **Model Interface** | QAbstractItemModel (rows, columns, data) | ✅ index(), parent(), rowCount(), columnCount(), data(), headerData() |
| **JSON Read** | `parseJsonFile()` - Load from disk | ✅ Uses JsonReader, handles schema validation, legacy conversion |
| **JSON Write** | `writeJsonContent()` - Save to disk | ✅ Uses JsonWriter with atomic writes |
| **Querying** | `get()`, `contains()`, `getAll()`, `count()` | ✅ Dict lookups + model data access |
| **Schema Validation** | Validate against JSON schema | ✅ Loads snippet.schema.json, validates with nlohmann_json_schema |
| **Legacy Format Support** | Convert old formats to modern | ✅ Uses LegacyTemplateConverter |

---

### TemplateManager (scadtemplates namespace)
**Current Purpose:** Generic template collection manager  
**Current Location:** `src/scadtemplates/template_manager.{hpp,cpp}`  
**Status:** Simple QList-based collection, minimal functionality

| Feature | Current Implementation | Issue |
|---------|---|---|
| **Storage** | `QList<ResourceTemplate> m_templates` | ❌ Duplicate with TemplatesInventory |
| **Adding** | `addTemplate(ResourceTemplate)` | ❌ No JSON reading, no location awareness, no unique IDs |
| **Removing** | `removeTemplate(prefix)` | ⚠️ Not in TemplatesInventory (deletion not handled) |
| **Search** | `findByPrefix()`, `findByScope()`, `search()` | ⚠️ Not in TemplatesInventory (queries not provided) |
| **Model Interface** | None (just a list) | ❌ No Model-View support |
| **File I/O** | `loadFromFile()`, `saveToFile()` | ❌ Using simple TemplateParser, not integrated |
| **CRUD** | Basic add/remove | ⚠️ Incomplete, not connected to inventory |

---

## Design Problem: Current Overlap vs Missing

```
OVERLAP/CONFUSION:
├── Storage Strategy:
│   ├── TemplatesInventory: QHash with unique IDs (correct for multi-tier)
│   └── TemplateManager: QList with prefix search (fine for single collection)
│
├── JSON Handling:
│   ├── TemplatesInventory: Full JSON read/write with validation (in resourceInventory)
│   └── TemplateManager: File I/O only via TemplateParser (incomplete)
│
└── Model Interface:
    ├── TemplatesInventory: Provides QAbstractItemModel (correct, one model per data source)
    └── TemplateManager: None (should have had it, or defer to Inventory)

MISSING RESPONSIBILITIES:
├── Deletion: No remove() in TemplatesInventory
├── Search: No query methods in TemplatesInventory (only getAll)
└── Edit-in-place: No update() for modifying template properties
```

---

## Proposed Clean Split

### TemplatesInventory (Final Responsibility)
**Becomes:** The single Qt Model + Storage layer  
**Namespace:** `resourceInventory`  
**Inheritance:** `QAbstractItemModel`

#### Keep These
```cpp
// Storage
void addTemplate(const QDirListing::DirEntry&, const ResourceLocation&);
int addFolder(const QDirListing::DirEntry&, const ResourceLocation&);
bool contains(const QString& key) const;
int count() const { return m_templates.size(); }
void clear();

// Model Interface (existing QtModel methods)
QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
// ... all existing model methods

// Query (keep)
QList<QVariant> getAll() const;
ResourceTemplate get(const QString& key) const;

// JSON I/O
QJsonObject getJsonContent(const QString& key) const;
bool writeJsonContent(const QString& key, const QJsonObject& json, QString& errorMsg);
```

#### Add These (CRUD Extensions)
```cpp
// Delete - MISSING
bool removeTemplate(const QString& key);

// Update - MISSING (edit existing)
bool updateTemplate(const QString& key, const ResourceTemplate& updated);

// Search queries - MISSING (but could be in separate repo service)
QList<ResourceTemplate> findByScope(const QString& scope) const;
QList<ResourceTemplate> search(const QString& keyword) const;

// Batch operations
int addTemplates(const QList<ResourceTemplate>& templates, const ResourceLocation& location);
```

### TemplateManager (Redirect/Deprecate)
**Options:**

#### Option A: Deprecate (Recommended)
- Phase it out after migrating all consumers to use TemplatesInventory
- TemplatesInventory becomes the single source of template management
- Simpler codebase, one model per data source

#### Option B: Repurpose as Service Layer
- Keep TemplateManager as a **stateless utility class** for template operations
- Does NOT own storage
- Acts as mediator between UI and TemplatesInventory
- Provides high-level operations:

```cpp
class TemplateManager {
private:
    TemplatesInventory* m_inventory;  // Owns reference, not data
    
public:
    TemplateManager(TemplatesInventory* inventory) : m_inventory(inventory) {}
    
    // Can't add without location, so would need refactor anyway
    bool insertTemplate(const QString& filePath, const ResourceLocation& location);
    
    // High-level operations
    void deleteTemplates(const QList<QString>& keys);
    bool updateTemplateMetadata(const QString& key, const QJsonObject& newMetadata);
    QList<ResourceTemplate> searchByKeyword(const QString& keyword) const;
    
    // For batch ops
    int importFromDirectory(const QString& dirPath, const ResourceLocation& location);
};
```

---

## Recommendation: 3-Layer Architecture

```
┌─────────────────────────────────────┐
│          UI Layer (QML/Widgets)     │
│  (Shows templates in list/tree)     │
└────────────┬────────────────────────┘
             │
┌────────────▼────────────────────────┐
│    Business Logic Layer             │
│  (TemplateManager as service)       │
│  • High-level CRUD                  │
│  • Search & filtering               │
│  • Import/export workflows          │
└────────────┬────────────────────────┘
             │
┌────────────▼────────────────────────┐
│  Model/Storage Layer                │
│  (TemplatesInventory)               │
│  • QHash storage                    │
│  • QAbstractItemModel for views     │
│  • JSON read/write                  │
│  • Schema validation                │
└─────────────────────────────────────┘
```

---

## Concrete Changes Needed

### 1. Add to TemplatesInventory
```cpp
// Deletion
bool removeTemplate(const QString& key);

// Updating
bool updateTemplate(const QString& key, const ResourceTemplate& updated);

// Search  
QList<ResourceTemplate> findByScope(const QString& scope) const;
QList<ResourceTemplate> search(const QString& keyword) const;
QList<QString> getAllKeys() const { return m_keys; }  // For bulk ops
```

### 2. Refactor TemplateManager (Option B)
```cpp
// Make it a service that works WITH inventory, not replacing it
class TemplateManager {
private:
    TemplatesInventory* m_inventory;
    
public:
    explicit TemplateManager(TemplatesInventory* inv);
    
    // High-level ops that delegate to inventory
    bool insertTemplateFile(const QString& filePath, 
                           const ResourceLocation& location);
    bool deleteTemplate(const QString& key);
    bool deleteTemplates(const QStringList& keys);
    
    // Batch import
    int importFromDirectory(const QString& dir, 
                           const ResourceLocation& location);
};
```

### 3. Delete from TemplateManager
```cpp
// Remove these - use TemplatesInventory methods instead
// - removeTemplate()  ← Use TemplatesInventory::removeTemplate()
// - addTemplate()     ← Use TemplatesInventory methods 
// - loadFromFile()    ← Just add folder via inventory
// - saveToFile()      ← Not needed if inventory handles JSON writes
// - findByPrefix()    ← Use TemplatesInventory::search()
// - findByScope()     ← Use TemplatesInventory::findByScope()
```

---

## Migration Path

### Phase 1: Add Missing Methods to TemplatesInventory
- [ ] `removeTemplate(key) → bool`
- [ ] `updateTemplate(key, ResourceTemplate) → bool`
- [ ] `findByScope(scope) → QList<ResourceTemplate>`
- [ ] `search(keyword) → QList<ResourceTemplate>`

### Phase 2: Refactor TemplateManager as Service
- [ ] Inject TemplatesInventory reference (dependency injection)
- [ ] Remove storage (m_templates)
- [ ] Delegate all operations to inventory

### Phase 3: Update Callers
- [ ] Audit all usages of TemplateManager
- [ ] Replace with direct TemplatesInventory calls or TemplateManager service methods
- [ ] Update unit tests

### Phase 4: Optional Cleanup
- [ ] Deprecate TemplateManager if all ops covered by inventory + services
- [ ] Or keep as service layer for business logic

---

## Summary Table: Who Does What

| Operation | TemplatesInventory | TemplateManager |
|-----------|---|---|
| Store templates | ✅ QHash | ❌ |
| Add from file | ✅ | ❌ |
| Add from folder | ✅ | ❌ |
| Remove template | ⚠️ (add) | — |
| Update template | ⚠️ (add) | — |
| Search/filter | ⚠️ (add) | ❌ |
| Provide Model interface | ✅ | ❌ |
| Read JSON | ✅ | ❌ |
| Write JSON | ✅ | ❌ |
| Validate JSON | ✅ | ❌ |
| High-level workflows | — | ⚠️ (refactor) |
| Import/export | — | ⚠️ (refactor) |

**Legend:** ✅ Keep/Add | ⚠️ Needs work | ❌ Remove | — Not applicable
