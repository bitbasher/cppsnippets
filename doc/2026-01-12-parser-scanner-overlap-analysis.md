# Parser vs Scanner Overlap Analysis

**Date:** 2026-01-12  
**Author:** User + Agent  
**Status:** Investigation for Consolidation  

---

## Problem Statement

We have MULTIPLE components handling template resources at different architectural layers:

**Scanning/Discovery Layer:**

1. **resourceScanning/resourceScanner** - Generic file discovery (all resource types)
2. **resourceScanning/templateScanner** - Template-specific scanning + parsing

**Parsing Layer:**
3. **scadtemplates/template_parser** - JSON format parsing (legacy ↔ modern conversion)

**Management Layer:**
4. **scadtemplates/template_manager** - In-memory template collection (add/remove/search/load/save)
5. **resourceInventory/ResourceInventoryManager** - High-level inventory coordinator (all types)

**Questions:**

1. Should templateScanner be renamed? (It's not just scanning, it's parsing)
2. Does template_parser overlap with templateScanner?
3. Where should resourceScanner live?
4. How do the Manager layers fit? Should TemplateManager be generalized for all resource types?

---

## Current State Analysis

### 1. ResourceScanner (Generic File Discovery)

**Location:** `src/resourceScanning/` *(Successfully moved from resourceInventory/ - no code changes needed)*

**Purpose:** Find files by type, create basic metadata

**What it does:**

```cpp
QVector<ResourceItem> scanColorSchemes(path, tier, key) {
    QStringList filters = {"*.json"};
    QFileInfoList files = dir.entryInfoList(filters);
    for (file : files) {
        ResourceItem item(path, type, tier);
        item.setName(baseName);
        results.append(item);
    }
}
```

**Characteristics:**

- ✅ Finds files by extension
- ✅ Creates basic ResourceItem (path, name, tier)
- ❌ Does NOT read file contents
- ❌ Does NOT parse JSON
- ❌ Does NOT validate structure

**Role:** **File Discovery** (filesystem operations only)

---

### 2. TemplateScanner (Template Discovery + Parsing)

**Location:** `src/resourceScanning/templateScanner.cpp`

**Purpose:** Scan folders for templates, parse JSON, extract metadata

**What it does:**

```cpp
QList<ResourceTemplate> scanLocation(location) {
    QDirIterator it(templatesPath, {"*.json"}, QDir::Files);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        
        // 1. READ FILE
        QFile file(filePath);
        file.open(QIODevice::ReadOnly);
        QByteArray fileData = file.readAll();
        
        // 2. PARSE JSON
        QJsonDocument json = QJsonDocument::fromJson(fileData);
        
        // 3. VALIDATE (simple check)
        if (!validateTemplateJson(json)) continue;
        
        // 4. EXTRACT METADATA
        ResourceTemplate tmpl = extractMetadata(json, filePath, location);
        templates.append(tmpl);
    }
    return templates;
}

bool validateTemplateJson(json) {
    // Check if object has "name" field (REQUIRED)
    // Check if "category", "parameters", "body" are correct types (OPTIONAL)
    return obj.contains("name") && obj["name"].isString();
}

ResourceTemplate extractMetadata(json, path, location) {
    // Extract fields from JSON:
    QString name = obj["name"].toString();
    QString category = obj["category"].toString();
    QString body = obj["body"].toString();
    QJsonArray params = obj["parameters"].toArray();
    
    // Set ResourceTemplate fields
    tmpl.setName(name);
    tmpl.setCategory(category);
    tmpl.setBody(body);
    // ... etc
}
```

**Characteristics:**

- ✅ Finds .json files in templates/ folder
- ✅ Reads file contents
- ✅ Parses JSON
- ✅ Validates structure (simple: checks "name" exists)
- ✅ Extracts metadata into ResourceTemplate
- ❌ Does NOT handle multiple format types (legacy vs modern)
- ❌ Does NOT convert between formats

**Role:** **Template Discovery + Basic Parsing** (filesystem + JSON operations)

**JSON Format Expected:**

```json
{
  "name": "Template Name",
  "category": "Category",
  "parameters": [...],
  "body": "OpenSCAD code"
}
```

---

### 3. TemplateParser (Format Conversion + Parsing)

**Location:** `src/scadtemplates/template_parser.cpp`

**Purpose:** Parse template JSON in multiple formats, convert between formats

**What it does:**

```cpp
ParseResult parseJson(jsonContent) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonContent.toUtf8());
    QJsonObject root = doc.object();
    
    // 1. DETECT FORMAT
    if (root.contains("_format")) {
        QString format = root["_format"].toString();
        if (format == "vscode-snippet") {
            return parseModernTemplate(root);  // VS Code format
        }
    }
    
    // 2. CHECK LEGACY FORMAT
    if (isLegacyFormat(root)) {  // Has "key" + "content"
        return parseLegacyTemplate(root);
    }
    
    // 3. CHECK MODERN FORMAT
    if (isModernFormat(root)) {  // Has nested objects with "prefix" + "body"
        return parseModernTemplate(root);
    }
}

bool isLegacyFormat(json) {
    // Legacy: { "key": "name", "content": "code" }
    return json.contains("key") && json.contains("content");
}

bool isModernFormat(json) {
    // Modern VS Code: { "template_name": { "prefix": "...", "body": [...] } }
    for (key : json) {
        if (json[key].isObject()) {
            if (json[key].contains("prefix") && json[key].contains("body")) {
                return true;
            }
        }
    }
}

ResourceTemplate parseLegacyTemplate(json) {
    QString key = json["key"].toString();
    QString content = json["content"].toString();
    
    // Convert escaped newlines
    content.replace("\\n", "\n");
    
    // Convert cursor marker ^~^ to $0
    content.replace("^~^", "$0");
    
    tmpl.setPrefix(key);
    tmpl.setBody(content);
    tmpl.setFormat("text/scad.template");
    tmpl.setSource("legacy-converted");
}

QList<ResourceTemplate> parseModernTemplate(json) {
    // VS Code format: { "name": { "prefix": "x", "body": ["line1", "line2"] } }
    for (templateName : json.keys()) {
        QString prefix = json[templateName]["prefix"].toString();
        QJsonArray bodyArray = json[templateName]["body"].toArray();
        QString description = json[templateName]["description"].toString();
        
        // Join body array into single string
        QString body = joinBodyArray(bodyArray);
        
        tmpl.setPrefix(prefix);
        tmpl.setBody(body);
        tmpl.setName(templateName);
    }
}

// ALSO: Convert TO JSON
QJsonObject templateToJson(ResourceTemplate tmpl, source) {
    // Convert ResourceTemplate back to JSON format
    QJsonObject json;
    json["_format"] = "vscode-snippet";
    json["_source"] = source;
    json[tmpl.name()] = {
        "prefix": tmpl.prefix(),
        "body": splitBodyToArray(tmpl.body()),
        "description": tmpl.description()
    };
}
```

**Characteristics:**

- ✅ Parses JSON strings (not files - caller must read)
- ✅ Handles MULTIPLE formats (legacy, modern VS Code)
- ✅ Validates structure (checks format markers)
- ✅ Converts between formats
- ✅ Bidirectional (parse JSON → ResourceTemplate, ResourceTemplate → JSON)
- ❌ Does NOT scan folders
- ❌ Does NOT read files directly

**Role:** **Format Conversion + Deep Parsing** (JSON operations only)

**JSON Formats Supported:**

**Legacy Format:**

```json
{
  "key": "module",
  "content": "module name() {\\n  ^~^\\n}"
}
```

**Modern VS Code Format:**

```json
{
  "_format": "vscode-snippet",
  "_source": "builtin",
  "Create Module": {
    "prefix": "module",
    "body": [
      "module ${1:name}() {",
      "  $0",
      "}"
    ],
    "description": "Create an OpenSCAD module"
  }
}
```

---

## 4. TemplateManager (Collection Management)

**Location:** `src/scadtemplates/template_manager.cpp`

**Purpose:** Manage in-memory collection of templates (CRUD operations)

**What it does:**

```cpp
class TemplateManager {
    QList<ResourceTemplate> m_templates;  // In-memory storage

public:
    // CRUD operations
    bool addTemplate(const ResourceTemplate& tmpl);
    bool removeTemplate(const QString& prefix);
    
    // Query operations
    std::optional<ResourceTemplate> findByPrefix(const QString& prefix) const;
    QList<ResourceTemplate> findByScope(const QString& scope) const;
    QList<ResourceTemplate> search(const QString& keyword) const;
    QList<ResourceTemplate> getAllTemplates() const;
    size_t count() const;
    void clear();
    
    // Persistence operations
    bool loadFromFile(const QString& filePath);  // Uses TemplateParser internally
    bool saveToFile(const QString& filePath) const;  // Uses TemplateParser internally
};

// Example usage:
TemplateManager manager;

// Load templates from disk
manager.loadFromFile("templates.json");  // Uses TemplateParser under the hood

// Add new template
ResourceTemplate tmpl;
tmpl.setPrefix("module");
tmpl.setBody("module name() { $0 }");
manager.addTemplate(tmpl);

// Search
auto results = manager.findByScope("source.scad");
auto moduleTemplate = manager.findByPrefix("module");

// Save back to disk
manager.saveToFile("templates.json");
```

**Characteristics:**

- ✅ In-memory collection storage (QList<ResourceTemplate>)
- ✅ CRUD operations (add, remove, update)
- ✅ Query operations (find by prefix, scope, keyword search)
- ✅ Persistence (load/save using TemplateParser internally)
- ❌ Does NOT scan folders (delegates to TemplateScanner)
- ❌ Does NOT parse JSON directly (delegates to TemplateParser)

**Role:** **Collection Management** (business logic for template operations)

**Dependencies:**

- Uses `TemplateParser` for load/save operations
- Stores `ResourceTemplate` objects
- No GUI dependencies (pure business logic)

---

## 5. ResourceInventoryManager (High-Level Coordinator)

**Location:** `src/resourceScanning/resourceScanner.cpp` (embedded in same file as ResourceScanner)

**Purpose:** Coordinate scanning across all tiers (Installation/Machine/User) for all resource types

**What it does:**

```cpp
class ResourceInventoryManager : public QObject {
    Q_OBJECT
    
    ResourceScanner m_scanner;
    QVector<platformInfo::ResourceLocation> m_installLocs;
    QVector<platformInfo::ResourceLocation> m_machineLocs;
    QVector<platformInfo::ResourceLocation> m_userLocs;
    
    // One tree widget per resource type
    QMap<ResourceType, ResourceTreeWidget*> m_inventories;

public:
    // Set locations for each tier
    void setInstallLocations(const QVector<platformInfo::ResourceLocation>& locs);
    void setMachineLocations(const QVector<platformInfo::ResourceLocation>& locs);
    void setUserLocations(const QVector<platformInfo::ResourceLocation>& locs);
    
    // Build inventories from ResourceLocationManager
    void buildInventory(const platformInfo::ResourceLocationManager& manager);
    
    // Build inventory for specific type
    ResourceTreeWidget* buildInventory(ResourceType type);
    
    // Get existing inventory
    ResourceTreeWidget* inventory(ResourceType type);
    
    // Refresh inventory
    void refreshInventory(ResourceType type);
};

// Example usage:
ResourceInventoryManager invManager;

// Initialize with discovered locations
platformInfo::ResourceLocationManager locationMgr;
invManager.buildInventory(locationMgr);  // Scans all tiers for all types

// Get inventory for specific type
ResourceTreeWidget* templateTree = invManager.inventory(ResourceType::Template);
ResourceTreeWidget* fontTree = invManager.inventory(ResourceType::Font);

// Refresh after changes
invManager.refreshInventory(ResourceType::Template);
```

**Characteristics:**

- ✅ Coordinates scanning across tiers (Installation/Machine/User)
- ✅ Manages inventories for ALL resource types (not just templates)
- ✅ Delegates actual scanning to ResourceScanner
- ✅ Stores results in ResourceTreeWidget (GUI component)
- ✅ Qt signals/slots for async operations
- ❌ Does NOT parse files (delegates to ResourceScanner)
- ❌ Does NOT manage in-memory collections (different from TemplateManager)

**Role:** **Inventory Coordinator** (orchestrates scanning, manages GUI trees)

**Key Difference from TemplateManager:**

- **ResourceInventoryManager:** GUI-focused, coordinates scanning → populates tree widgets
- **TemplateManager:** Business logic, manages in-memory collection, no GUI

---

## Overlap Analysis

### What's Redundant?

| Feature | ResourceScanner | TemplateScanner | TemplateParser | TemplateManager | ResourceInventoryManager |
|---------|----------------|-----------------|----------------|-----------------|--------------------------|
| **Find files** | ✅ All types | ✅ Templates only | ❌ No | ❌ No | ❌ Delegates to Scanner |
| **Read file contents** | ❌ No | ✅ Yes | ❌ Caller does | ✅ Via Parser | ❌ Delegates |
| **Parse JSON** | ❌ No | ✅ Basic | ✅ Advanced | ✅ Via Parser | ❌ Delegates |
| **Validate structure** | ❌ No | ✅ Simple (name exists) | ✅ Format detection | ❌ Delegates | ❌ Delegates |
| **Handle multiple formats** | ❌ No | ❌ One format | ✅ Legacy + Modern | ✅ Via Parser | ❌ No |
| **Convert formats** | ❌ No | ❌ No | ✅ Yes | ✅ Via Parser | ❌ No |
| **Bidirectional (to JSON)** | ❌ No | ❌ No | ✅ Yes | ✅ Via Parser | ❌ No |
| **Folder scanning** | ✅ All types | ✅ Templates | ❌ No | ❌ No | ✅ Orchestrates |
| **In-memory collection** | ❌ No | ❌ No | ❌ No | ✅ QList storage | ❌ No |
| **CRUD operations** | ❌ No | ❌ No | ❌ No | ✅ Add/Remove/Find | ❌ No |
| **GUI tree management** | ❌ No | ❌ No | ❌ No | ❌ No | ✅ ResourceTreeWidget |
| **Multi-tier coordination** | ❌ No | ❌ No | ❌ No | ❌ No | ✅ Install/Machine/User |

### Where's the Overlap?

**TemplateScanner vs TemplateParser:**

Both parse JSON templates, but:

- **TemplateScanner:** File discovery + basic parsing (one format)
- **TemplateParser:** Advanced parsing + format conversion (multiple formats)

**Redundancy:** Both extract metadata from JSON into ResourceTemplate

---

## Naming Analysis

### Current Problem: "TemplateScanner" is Misleading

**What "Scanning" Usually Means:**

- Finding files (filesystem operations)
- Directory traversal
- Pattern matching

**What TemplateScanner Actually Does:**

- Finds files ✅ (this is scanning)
- Reads files ✅ (not scanning)
- Parses JSON ✅ (not scanning)
- Validates structure ✅ (not scanning)
- Extracts metadata ✅ (not scanning)

**Conclusion:** "TemplateScanner" does WAY more than scanning

### Better Names

#### Option 1: Emphasize Discovery Role

- `TemplateDiscovery` - Discovers templates (find + parse)
- `TemplateCollector` - Collects templates from folders
- `TemplateFinder` - Finds and loads templates

#### Option 2: Emphasize Loader Role

- `TemplateLoader` - Loads templates from disk
- `TemplateReader` - Reads template files
- `TemplateImporter` - Imports templates from folders

#### Option 3: Be Specific About Combined Role

- `TemplateFileScanner` - Scans template files specifically
- `TemplateFolderScanner` - Scans folders for templates
- `TemplateDirectoryReader` - Reads templates from directories

#### Option 4: Use "Provider" Pattern

- `TemplateFileProvider` - Provides templates from files
- `TemplateFolderProvider` - Provides templates from folders

**Recommendation:** `TemplateLoader` or `TemplateFileProvider`

- Clear: Loads templates from disk
- Accurate: Does more than just scanning
- Common: Matcal Layers Clarification

### The Five-Layer Architecture

```
Layer 1: DISCOVERY (Filesystem)
  ├── ResourceScanner → Finds files (all types)
  └── [Moving to resourceScanning/ - code unchanged, just relocating]

Layer 2: LOADING (Filesystem + Parsing)
  └── TemplateScanner → Finds + reads + parses templates
      [Misleading name - does more than scanning]

Layer 3: PARSING (Format Handling)
  └── TemplateParser → Parses JSON strings, handles multiple formats
   Manager Layer Design Question

### Two Different "Managers" with Different Purposes

**TemplateManager (Business Logic):**
- In-memory collection management
- CRUD operations (add, remove, find, search)
- Persistence (load/save to files)
- No GUI dependencies
- Template-specific

**ResourceInventoryManager (GUI Coordinator):**
- Orchestrates scanning across tiers
- Populates GUI tree widgets
- Manages ResourceTreeWidget instances
- GUI-coupled
- All resource types

### Key Question: How Should Managers Be Organized?

#### Option 1: Generalize TemplateManager → ResourceManager<T>

```cpp
// Generic resource collection manager (business logic)
template<typename T>
class ResourceManager {
    QList<T> m_resources;
    
public:
    bool add(const T& resource);
    bool remove(const QString& key);
    std::optional<T> findByKey(const QString& key) const;
    QList<T> search(const QString& keyword) const;
    QList<T> getAll() const;
    void clear();
};

// Specializations for each type
using TemplateManager = ResourceManager<ResourceTemplate>;
using FontManager = ResourceManager<ResourceFont>;
using ColorSchemeManager = ResourceManager<ResourceColorScheme>;
```

**Pros:**

- DRY - shared code for all resource types
- Type-safe collections
- Easy to add new resource types

**Cons:**

- Templates add complexity
- Load/save needs type-specific parsers
- May be overkill for simple types

---

#### Option 2: Subclass TemplateManager → ResourceManagerBase

```cpp
// Abstract base class
class ResourceManagerBase : public QObject {
    Q_OBJECT
public:
    virtual bool add(const ResourceItem& item) = 0;
    virtual bool remove(const QString& key) = 0;
    virtual std::optional<ResourceItem> find(const QString& key) const = 0;
    virtual QList<ResourceItem> search(const QString& keyword) const = 0;
    virtual void clear() = 0;
    
    // Common operations
    virtual bool loadFromFile(const QString& path) = 0;
    virtual bool saveToFile(const QString& path) const = 0;
};

// Template-specific subclass
class TemplateManager : public ResourceManagerBase {
    QList<ResourceTemplate> m_templates;
    TemplateParser m_parser;
    
public:
    // Override base methods with template-specific logic
    bool add(const ResourceTemplate& tmpl);
    std::optional<ResourceTemplate> findByPrefix(const QString& prefix);
    bool loadFromFile(const QString& path) override;  // Uses TemplateParser
};

// Font-specific subclass
class FontManager : public ResourceManagerBase {
    QList<ResourceFont> m_fonts;
    
    bool loadFromFile(const QString& path) override;  // Font discovery logic
};
```

**Pros:**

- Polymorphism allows treating all managers uniformly
- Each subclass handles type-specific logic
- Clear inheritance hierarchy

**Cons:**

- Polymorphism overhead
- Base class needs to know about all common operations
- Return types must use base ResourceItem

---

#### Option 3: Keep Separate Managers, Extract Common Patterns

```cpp
// Shared utilities (no base class)
namespace ResourceManagerUtils {
    template<typename T>
    bool addUnique(QList<T>& list, const T& item, 
                   std::function<QString(const T&)> keyFunc);
    
    template<typename T>
    QList<T> searchByKeyword(const QList<T>& list, const QString& keyword,
                            std::function<bool(const T&, const QString&)> matcher);
}

// Independent managers using shared utilities
class TemplateManager {
    QList<ResourceTemplate> m_templates;
    
    bool addTemplate(const ResourceTemplate& tmpl) {
        return ResourceManagerUtils::addUnique(m_templates, tmpl, 
            [](const auto& t) { return t.prefix(); });
    }
    
    QList<ResourceTemplate> search(const QString& keyword) {
        return ResourceManagerUtils::searchByKeyword(m_templates, keyword,
            [](const auto& t, const QString& kw) {
                return t.prefix().contains(kw) || t.description().contains(kw);
            });
    }
};

class FontManager {
    QList<ResourceFont> m_fonts;
    // Uses same utilities, different types
};
```

**Pros:**

- No inheritance complexity
- Each manager is independent
- Shared code via utility functions
- Type-safe (no base class casting)

**Cons:**

- More code duplication in manager classes
- No polymorphism (can't treat managers uniformly)

---

#### Option 4: Two-Tier Manager Design (Recommended)

**Separate GUI coordination from business logic:**

```cpp
// Layer 4A: Business Logic (In-Memory Collections)
class TemplateCollectionManager {
    QList<ResourceTemplate> m_templates;
    TemplateParser m_parser;
    
    bool add(const ResourceTemplate& tmpl);
    bool remove(const QString& prefix);
    std::optional<ResourceTemplate> findByPrefix(const QString& prefix);
    bool loadFromFile(const QString& path);
    bool saveToFile(const QString& path);
};

class FontCollectionManager {
    QList<ResourceFont> m_fonts;
    // Font-specific operations
};

// Layer 4B: GUI Coordination (Multi-Tier Discovery)
class ResourceInventoryCoordinator {
    ResourceScanner m_scanner;
    QVector<platformInfo::ResourceLocation> m_installLocs;
    QVector<platformInfo::ResourceLocation> m_machineLocs;
    QVector<platformInfo::ResourceLocation> m_userLocs;
    
    // Coordinates discovery, NO direct GUI coupling
    QVector<ResourceItem> scanAllTiers(ResourceType type);
    
signals:
    void resourcesDiscovered(ResourceType type, const QVector<ResourceItem>& items);
};

// Layer 5: GUI Adapters (Separate from business logic)
class ResourceTreeWidgetAdapter {
    ResourceTreeWidget* m_widget;
    ResourceInventoryCoordinator* m_coordinator;
    
    void populateFromCoordinator(ResourceType type);
};
```

**Pros:**

- Clear separation: business logic ↔ GUI coordination
- TemplateCollectionManager is pure business logic (testable)
- ResourceInventoryCoordinator emits signals (no GUI coupling)
- GUI adapters connect coordinators to widgets
- Easy to swap GUI (TreeWidget → Model/View)

**Cons:**

- More classes
- Need coordination between layers

---

### Proposed Consolidation

#### Option A: Clean Architecture with Separatedsiness Logic)
  ├── TemplateManager → In-memory collection, CRUD, search
  │   [Template-specific, needs generalization]
  └── ResourceInventoryManager → GUI coordinator, multi-tier scanning
      [All resource types, but GUI-coupled]

Layer 5: GUI (Presentation)
  ├── ResourceTreeWidget → Display resources in tree
  └── TemplateTreeModel → Model/View (already exists but unused)

```

### Current State Problems

**1. Mixed Responsibilities:**
```
resourceInventory/
  └── ResourceInventoryManager      [Layer 4: GUI Coordinator]
  
resourceScanning/
  ├── resourceScanner.cpp           [Layer 1: Discovery - ✅ MOVED HERE]
  └── templateScanner.cpp           [Layer 2: Loading - MISLEADING NAME]
  
scadtemplates/
  ├── template_parser.cpp           [Layer 3: Parsing - OVERLAPS Layer 2]
  └── template_manager.cpp          [Layer 4: Business Logic - Template-only]
```

**2. Redundancies:**
- TemplateScanner + TemplateParser both parse JSON
- TemplateManager is template-specific (need generic ResourceManager)
- ResourceInventoryManager is GUI-coupled (should be separate from display)── templateScanner.cpp       [MISLEADING NAME - does more than scanning]
  
scadtemplates/
  └── template_parser.cpp       [OVERLAPS with templateScanner]
```

### Proposed Consolidation

#### Option A: Separate Concerns (Recommended)

```
resourceScanning/
  ├── resourceScanner.cpp       [✅ COMPLETED - file discovery only]
  ├── templateFileProvider.cpp  [RENAMED - loads templates from disk]
  └── templateFormatParser.cpp  [RENAMED - parses multiple JSON formats]
```

**Responsibilities:**

1. **ResourceScanner** - File discovery only (no parsing)

   ```cpp
   // Finds files, returns paths + basic metadata
   QVector<DiscoveredResource> discoverFiles(path, type, tier);
   ```

2. **TemplateFileProvider** (renamed from TemplateScanner)

   ```cpp
   // High-level: Scan folders, load templates
   QList<ResourceTemplate> loadFromLocation(location) {
       auto files = ResourceScanner::discoverFiles(path, "*.json");
       QList<ResourceTemplate> templates;
       for (file : files) {
           auto content = readFile(file.path);
           templates.append(TemplateFormatParser::parse(content));
       }
       return templates;
   }
   ```

3. **TemplateFormatParser** (renamed from TemplateParser)

   ```cpp
   // Low-level: Parse JSON strings, handle multiple formats
   ResourceTemplate parse(jsonString);
   QString toJson(ResourceTemplate);
   ```

**Benefits:**

- Clear separation: Discovery → Loading → Parsing
- TemplateFileProvider uses ResourceScanner + TemplateFormatParser
- Each component does ONE thing

---

#### Option B: Merge Scanner + Parser (Alternative)

```
resourceScanning/
  ├── resourceScanner.cpp       [✅ COMPLETED - generic file discovery]
  └── templateLoader.cpp        [MERGED - combines scanner + parser]
  
scadtemplates/
  └── [DELETE template_parser]  [No longer needed]
```

**Responsibilities:**

1. **ResourceScanner** - Generic file discovery

   ```cpp
   QVector<DiscoveredResource> discoverFiles(path, type);
   ```

2. **TemplateLoader** (merged templateScanner + templateParser)

   ```cpp
   // Handles all template operations: scan + parse + format conversion
   QList<ResourceTemplate> loadFromLocation(location);
   ResourceTemplate parseJson(jsonString);
   QString templateToJson(ResourceTemplate);
   ```

**Benefits:**

- Single component for all template operations
- No redundancy between scanner and parser
- Simpler mental model

**Drawbacks:**

- Larger class with multiple responsibilities
- Harder to test individual pieces

---

#### Option C: Keep All Three, Clarify Roles (Minimal Change)

```
resourceScanning/
  ├── resourceScanner.cpp       [✅ COMPLETED]
  └── templateLoader.cpp        [RENAMED - was templateScanner]
  
scadtemplates/
  └── templateFormatParser.cpp  [RENAMED - was template_parser]
```

**Keep existing structure, just:**

- ✅ Move resourceScanner to correct folder
- Rename for clarity
- Document responsibilities

---

## Recommended Implementation Plan

### Phase 1: Move + Rename (Low Risk)

**Actions:**

1. ✅ **COMPLETED:** Move `resourceInventory/resourceScanner.*` → `resourceScanning/resourceScanner.*`
2. Rename `templateScanner.*` → `templateLoader.*` (or `templateFileProvider.*`)
3. Rename `template_parser.*` → `templateFormatParser.*`
4. Update includes/CMakeLists.txt
5. Update documentation

**Estimated Effort:** 2-3 hours  
**Risk:** Low - mostly mechanical changes  
**Benefit:** Clear naming, correct folder structure

---

### Phase 2: Clarify Responsibilities (Medium Risk)

**Actions:**

1. **ResourceScanner:** Remove any JSON parsing, return only file paths
2. **TemplateLoader:** Use ResourceScanner for discovery, TemplateFormatParser for parsing
3. **TemplateFormatParser:** Keep format conversion logic, make it stateless utility

**Code Example:**

```cpp
// templateLoader.cpp
QList<ResourceTemplate> TemplateLoader::loadFromLocation(location) {
    // 1. Discover files (delegate to ResourceScanner)
    QString templatesPath = location.path() + "/templates";
    auto files = ResourceScanner::discoverFiles(templatesPath, "*.json");
    
    // 2. Load and parse each file
    QList<ResourceTemplate> templates;
    for (auto& file : files) {
        // Read file
        QFile f(file.path);
        f.open(QIODevice::ReadOnly);
        QString content = f.readAll();
        
        // Parse (delegate to TemplateFormatParser)
        auto result = TemplateFormatParser::parse(content);
        if (result.success) {
            templates.append(result.templates);
        }
    }
    
    return templates;
}
```

**Estimated Effort:** 4-6 hours  
**Risk:** Medium - changes logic, needs testing  
**Benefit:** Clear separation of concerns

---

### Phase 3: Consolidate or Keep Separate (Your Choice)

**Decision Point:** Do you want to merge TemplateLoader + TemplateFormatParser?

**Option A:** Keep separate (recommended for large projects)

- TemplateLoader = high-level orchestration
- TemplateFormatParser = low-level format handling

**Option B:** Merge into single TemplateLoader

- Simpler for small projects
- All template operations in one place

---

## Summary

### Answers to Your Questions

**1. Should we rename templateScanner?**

**YES.** It does more than scanning (reads files, parses JSON, extracts metadata).

**Recommended Names:**

- `TemplateLoader` (loads templates from disk)
- `TemplateFileProvider` (provides templates from files)
- `TemplateImporter` (imports templates from folders)

My preference: **TemplateLoader** - clear and common in Qt

---

**2. Does template_parser overlap with templateScanner?**

**YES.** Both parse JSON templates into ResourceTemplate.

**Key Differences:**

- **templateScanner:** Basic parsing (one format: {"name", "category", "body"})
- **template_parser:** Advanced parsing (multiple formats: legacy + VS Code)

**Overlap:** Both extract metadata from JSON

**Recommendation:** 

- Keep both BUT clarify roles:
  - TemplateLoader (high-level: scan folders → load files)
  - TemplateFormatParser (low-level: parse JSON → handle formats)
- TemplateLoader should USE TemplateFormatParser internally

---

**3. Should resourceScanner move to resourceScanning/?**

**YES.** It's a scanning operation (filesystem discovery), not inventory management.

**Important:** ResourceScanner code works well - no functional changes needed, just relocate to proper folder.

**Correct Folder:**
- `resourceScanning/` = Finding resources (discovery, parsing)
- `resourceInventory/` = Managing resources (storage, display, tree models)

**Action:** Move `resourceInventory/resourceScanner.*` → `resourceScanning/resourceScanner.*`

---

## Next Steps

**Recommended Sequence:**

1. ✅ Review this analysis
2. Choose Option A, B, or C from Architecture Recommendation
3. Create planning doc for Phase 1 (Move + Rename)
4. Implement Phase 1
5. Test and commit
6. Repeat for Phase 2

**My Recommendation:** Start with **Option A - Separate Concerns**

- Clearest architecture
- Best for future Qt Model/View migration
- Follows Single Responsibility Principle

**Do you want me to create the planning doc for Phase 1?**
