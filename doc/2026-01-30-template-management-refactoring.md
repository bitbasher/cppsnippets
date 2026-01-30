# Template Management Architecture Refactoring

**Date:** 2026-01-30  
**Status:** Planning  
**Objective:** Refactor template/snippet management into clean, separated concerns with TemplateManager as orchestrator, TemplateParser as format handler, and TemplatesInventory as exclusive storage.

The goal is to have all template handling code bundled into a library that can be built into the OpenSCAD app to add sippets to its script editor.

Note that template handling is going to be a sub-section of the complete resource system that is being prototyped by this project. At some point in the future the template library will in turn require resource management facilities to be provided by a libary, but for the moment the scatemplate app will provide resource knowledge to the template classes

---

## User Requirements

Templates (snippets) must be available in the OpenSCAD editor. The management system should:

1. **Discovery Integration:** TemplateManager receives locations from discovery process, delegates to TemplatesInventory to add template items to the inventory.
2. **File Operations:** Manager should not need to interacte directly with template_parser, but will be responsible for deleting unwanted template .json files in the file system
3. **Inventory Operations** will be handled by TemplatesInventgory methods as it wll contain the resrouce storage be provide the model to the views that will be implemented in the GUI.
3. **GUI Editor Integration:** Manager handles new, copy, save, delete operations from GUI template editor, but will not be aware of the signals nor state of editing operations in it.
4. **Inventory Consistency:** TemplatesInventory is the single authoritative storage; all access goes through it
5. **File Lifecycle:** a resource deletion signaled from the GUI shall cause the Manager to delegate to TemplatesInventory to remove it from memory storage, and then will use the file system to delete the .json

---

## Problem Statement

Current architecture has responsibility overlap:

- TemplatesInventory directly calls JsonReader/JsonWriter and handles file I/O
- TemplateManager duplicates storage (m_templates) instead of using inventory as source of truth
- No clear separation between format handling, storage, and business logic
- GUI has multiple code paths to modify templates without consistent manager oversight

**Result:** Code duplication, multiple storage sources, unclear data flow, difficult to test

---

## Proposed Architecture

Keeping focuses on this project, the scadtemplates prototype app.

``` text
┌─────────────────────────────────────────────────────────────────┐
│                    scadtemplates Editor                         │
│            (Uses templates in a Scintilla Editor )              │
└──────────────────────────┬──────────────────────────────────────┘
                           │ GUI template editor signals
                           │ (new, copy, save, delete.
                           |  File Menu: import, export )
 ┌───────────────────────┐ │
 │   Discovery Process   │ │
 │  at startup: outputs  │ │
 │  resources locations  │ │
 └──────────┬────────────┘ │
            │              │
            ▼              ▼
        ┌──────────────────────────────────┐
        │     TemplateManager (NEW ROLE)   │
        │  - Orchestrates all operations   │
        │  - Delegates template import and │
        │      export operations           │
        │  - handles .json file deletion   │
        └────────┬─────────────────────────┘
                 │   
         ┌───────┘
         │  Delegates to
         ▼  
  ┌──────────────────────┐
  │ TemplatesInventory   │
  │ (Storage/Model)      │
  ├──────────────────────┤
  │ • addTemplate()      │
  │ • addFolder()        │
  │ • removeTemplate()   │
  │ • count()            │
  │ • clear()            │
  │ • search()           │
  │ • Model interface    │
  │                      │
  │ Uses TemplateParser  │
  │   for json file r/w  │
  └──────────────────────┘
        ┌───┘
        │  Delegates to
        ▼ 
  ┌─────────────────┐
  │ TemplateParser  │ 
  │ (Format Layer)  │ 
  ├─────────────────┤
  │ • readJsonFile()│  
  │ • readJsonText()│ 
  │ • writeJsonFile │
  │ • validate()    │
  │ • converts legacy│ 
  │                 │ 
  │ No storage      │
  │ No file delete  │
  └─────────────────┘ 


```

---

## Detailed Responsibility Breakdown

### TemplateManager (Orchestrator/Service Layer)

**New Primary Responsibilities:**

- Single entry point for all template operations from GUI and discovery
- Orchestrates workflows combining multiple system components
- Owns only delete file operation, all other file handline is delegated
- Manages the relationship between inventory and file system

**Key Methods:**

```cpp
class TemplateManager {
public:
    // Discovery integration
    void onDiscoveryLocationsFound(const QList<platformInfo::ResourceLocation>& locations);
    
    // Batch operations (discovery process)
    void importTemplatesFromLocations(const QList<platformInfo::ResourceLocation>& locations);
    
    // GUI: File-based import/export
    void importFromJsonFile(const QString& filePath);
    void exportToJsonFile(const QString& templateKey, const QString& outputPath);
    void exportAllToJsonFile(const QString& outputPath);
    
    // GUI: Template editor - create new
    void createTemplateFromEditor(const ResourceTemplate& newTemplate, 
                                  const platformInfo::ResourceLocation& location);
    
    // GUI: Template editor - copy operation (NOT a storage operation, just data transfer)
    ResourceTemplate getTemplateForCopy(const QString& templateKey);
    
    // GUI: Template editor - save modified copy
    void saveModifiedTemplateCopy(const ResourceTemplate& modified,
                                  const platformInfo::ResourceLocation& location);
    
    // GUI: Template editor - delete
    void deleteTemplate(const QString& templateKey);
    
    // Queries (delegate to inventory)
    int templateCount() const { return m_inventory->count(); }
    QList<QVariant> getAllTemplates() const { return m_inventory->getAll(); }
    ResourceTemplate getTemplate(const QString& key) const;
    QList<ResourceTemplate> searchTemplates(const QString& query) const;

private:
    TemplatesInventory* m_inventory;  // Not owned (application singleton)
    TemplateParser m_parser;
};
```

### TemplateParser (Format & I/O Handler)

**Responsibilities:**

- Read JSON files (with QJsonParseError reporting)
- Parse JSON strings with format detection and legacy conversion
- Write JSON files to disk
- Schema validation
- Format conversion (legacy → modern)

**Current Methods (Keep As-Is):**

```cpp
class TemplateParser {
public:
    // File I/O
    ParseResult parseFile(const QString& filePath);
    bool writeToFile(const QString& outputPath, const ResourceTemplate& tmpl);
    bool writeMultipleToFile(const QString& outputPath, 
                            const QList<ResourceTemplate>& templates);
    
    // String parsing
    ParseResult parseJson(const QString& jsonContent);
    ParseResult parseJsonText(const QString& jsonText);
    
    // Format handling
    QJsonObject templateToJson(const ResourceTemplate& tmpl, 
                              const QString& source = "cppsnippet-made");
    
    // Schema validation
    bool validateAgainstSchema(const QJsonObject& json, 
                              QString& validationError);
    
    // Utilities
    static bool isLegacyFormat(const QJsonObject& json);
    static bool isModernFormat(const QJsonObject& json);

private:
    static QList<ResourceTemplate> parseModernTemplate(const QJsonObject& root);
    static ResourceTemplate parseLegacyTemplate(const QJsonObject& json);
};
```

### TemplatesInventory (Storage/Model - THE Authority)

**New Responsibilities:**

- Single source of truth for all templates
- Use TemplateParser for all file reading and writing operations
- Use TemplateParser for all format conversions  
- Add CRUD methods that are currently missing
- Model interface for UI (provides data() for view display - already exists)
- **Manage UniqueID lifecycle** (critical responsibility - see UniqueID Lifecycle section below)

**UniqueID Lifecycle Management:**

| Scenario | Entry State | Action | Exit State |
|----------|---|---|---|
| **New Template** | No UniqueID | User creates in editor, saves without ID | ID generated by addTemplateObject() on save |
| **Imported Template** | No UniqueID | File parsed by TemplateParser, added to inventory | ID generated by addTemplateObject() after successful read |
| **Edited Template** | Has UniqueID | User modifies existing (ID not visible to user), saves | updateTemplate() keeps original ID, replaces content |
| **Copied Template** | Original has ID | User copies original (copy has no ID temporarily) | addTemplateObject() generates NEW ID for copy |
| **Deleted Template** | Has UniqueID | removeTemplate() called | ID removed, hole remains in sequence (rebuilt at next startup) |

**New Methods to Add:**

```cpp
class TemplatesInventory : public QAbstractItemModel {
public:
    // Existing CRUD (keep)
    bool addTemplate(const QDirEntry& entry, 
                     const platformInfo::ResourceLocation& location);
    bool addFolder(const QString& folderPath, 
                  const platformInfo::ResourceLocation& location);
    QJsonObject getJsonContent(const QString& templateKey) const;
    int count() const;
    void clear();
    QList<QVariant> getAll() const;

    // NEW CRUD Methods
    
    // Add new template object: ALWAYS generates a NEW UniqueID
    // Used when:
    //   - Saving new templates created in editor (tmpl.uniqueID() is always empty)
    //   - Saving copies of templates (copy.uniqueID() will be empty before calling this)
    // - Validates template has required fields (prefix, body not empty)
    // - Generates NEW ID via ResourceIndexer
    // - Returns false if validation fails, sets errorMsg
    bool addTemplateObject(const ResourceTemplate& tmpl,
                          const platformInfo::ResourceLocation& location,
                          QString& errorMsg);
    
    // Get template for testing purposes only
    // In production, use the QAbstractItemModel interface (data() method)
    // which provides display properties without exposing internal structure
    ResourceTemplate getTemplate(const QString& templateKey) const;
    
    // Update existing template: replaces content while preserving UniqueID
    // Used when:
    //   - User modifies existing template (ID not visible to user, so cannot change)
    // - Simply replaces template in storage with updated version
    // - Original templateKey and UniqueID remain unchanged
    // - Returns false if templateKey doesn't exist
    bool updateTemplate(const QString& templateKey, 
                       const ResourceTemplate& updated);
    
    // Search capabilities
    QList<ResourceTemplate> search(const QString& query) const;
    QList<ResourceTemplate> filterByTier(ResourceTier tier) const;
    
    // Removal from inventory (NOT file deletion - that's manager's job)
    // - Removes template from m_templates
    // - Leaves hole in UniqueID sequence (will be rebuilt at startup)
    bool removeTemplate(const QString& templateKey);
    
    // Import/populate from locations
    // Delegates file I/O to TemplateParser
    void importFromLocation(const platformInfo::ResourceLocation& location);
    void importFromJsonFile(const QString& filePath);
    void importFromJsonString(const QString& jsonText);

private:
    // Generate a NEW UniqueID for a template being added
    QString m_generateNewUniqueID(const QString& templatePrefix);
};
```

**GUI Data Access:**

The GUI accesses template data through the model interface:
- `data(index, Qt::DisplayRole)` - Template display name for list/tree views
- `data(index, Qt::UserRole)` - Full ResourceTemplate object for editing
- `data(index, Qt::ToolTipRole)` - Template metadata (path, tier, ID)

Example: GUI wants to edit a template selected in a view:
```cpp
// In GUI, user clicks "Edit" on selected row
QModelIndex selected = view->currentIndex();
QVariant variantData = templateInventory->data(selected, Qt::UserRole);
ResourceTemplate templateToEdit = variantData.value<ResourceTemplate>();

// Now load into editor
editor->setTemplate(templateToEdit);
// User modifies prefix, body, description
// When user clicks "Save":
ResourceTemplate modified = editor->getTemplate();  // No ID in editor
// Call inventory with modified (ID still intact internally from original)
inventory->updateTemplate(selected.data(Qt::DisplayRole).toString(), modified);
```

---

## UniqueID Lifecycle - CRITICAL Architecture Detail

The UniqueID is the unique identifier for each template in the inventory. Its generation and management are critical to data consistency.

### Generation Strategy

**UniqueID Format:** `####-basename` (incrementing counter + template prefix)

**Generation Timing:**
- ❌ NOT generated when ResourceTemplate is created
- ❌ NOT generated by TemplateParser
- ✅ Generated by TemplatesInventory ONLY, at the moment template is committed to inventory
- ✅ Specifically, generated in `addTemplateObject()` AFTER validation succeeds

### Lifecycle by Scenario

**1. NEW Template (Created in Editor)**

```
User Creates in GUI Editor
        ↓
ResourceTemplate.uniqueID() = ""  (empty)
        ↓
User clicks "Save"
        ↓
GUI calls: templateManager->createTemplateFromEditor(template, location)
        ↓
Manager calls: inventory->addTemplateObject(template, location, error)
        ↓
Validation succeeds
        ↓
Inventory generates ID: "1001-my_new_template"
        ↓
Template stored in memory with ID
        ↓
Function returns SUCCESS
        ↓
GUI receives: "Template saved as 1001-my_new_template"
```

**2. IMPORTED Template (From Discovery or File)**

```
File on disk: "cube_basic.json"
        ↓
TemplateParser::parseFile() reads and validates
        ↓
ParseResult.templates[0].uniqueID() = ""  (empty, not from file)
        ↓
Inventory calls: addTemplateObject(parsed_template, location)
        ↓
Validation succeeds
        ↓
Inventory generates ID: "1002-cube_basic"
        ↓
Template stored in memory with ID
        ↓
Database rebuild at next startup recreates IDs from in-memory templates
```

**3. EDITED Template (Modify Existing)**

```
User selects "1000-my_template" and clicks "Edit"
        ↓
GUI gets: original = inventory->getTemplate("1000-my_template")
        ↓
original.uniqueID() = "1000-my_template"  (preserved)
        ↓
User modifies and clicks "Save"
        ↓
GUI calls: inventory->updateTemplate("1000-my_template", modified)
        ↓
Validation succeeds
        ↓
modified.uniqueID() = "1000-my_template"  (UNCHANGED)
        ↓
Template REPLACED in memory, ID preserved
        ↓
Function returns SUCCESS
```

**4. COPIED Template (Copy then Edit and Save as New)**

```
User selects "1000-my_template" and clicks "Copy"
        ↓
GUI gets: copy = inventory->getTemplate("1000-my_template")
        ↓
copy.uniqueID() = "1000-my_template"  (inherited from original)
        ↓
User modifies in editor, changes prefix to "my_template_v2"
        ↓
User clicks "Save As New"
        ↓
GUI MUST CLEAR ID before saving:
      copy.setUniqueID("");  // <- CRITICAL
        ↓
GUI calls: templateManager->saveModifiedTemplateCopy(copy, location)
        ↓
Manager calls: inventory->addTemplateObject(copy, location)
        ↓
Validation succeeds
        ↓
copy.uniqueID() was empty, so Inventory generates NEW ID: "1003-my_template_v2"
        ↓
BOTH templates now in inventory:
  - "1000-my_template" (original)
  - "1003-my_template_v2" (copy)
        ↓
Function returns SUCCESS
```

**5. DELETED Template (Remove from Inventory)**

```
User selects "1001-my_template" and clicks "Delete"
        ↓
Manager calls: inventory->removeTemplate("1001-my_template")
        ↓
Inventory removed template from m_templates QHash
        ↓
Manager calls: QFile::remove(template_file_path)
        ↓
File deleted from disk
        ↓
At next startup, discovery runs again
        ↓
"1001-..." is never recreated (ResourceIndexer starts from counter + 1)
        ↓
Hole in sequence is acceptable
```

### Key Implementation Details

**In ResourceIndexer:**
```cpp
// Static counter persists across calls
static int s_nextId = 1000;  // Or load from max existing ID at startup

QString ResourceIndexer::getUniqueIDString(const QString& baseName) {
    QString id = QString("%1-%2").arg(s_nextId++).arg(baseName);
    // s_nextId is now 1001, 1002, 1003, etc.
    return id;
}
```

**In TemplatesInventory:**
```cpp
bool TemplatesInventory::addTemplateObject(
    const ResourceTemplate& tmpl,
    const platformInfo::ResourceLocation& location,
    QString& errorMsg)
{
    // Validate BEFORE generating ID
    if (tmpl.prefix().isEmpty()) {
        errorMsg = "Template must have prefix";
        return false;
    }
    
    // Make a copy for storage
    ResourceTemplate stored = tmpl;
    
    // CRITICAL: Only generate ID if not already set
    if (stored.uniqueID().isEmpty()) {
        stored.setUniqueID(ResourceIndexer::getUniqueIDString(stored.prefix()));
    }
    
    // Check for duplicates AFTER ID assignment
    if (m_templates.contains(stored.uniqueID())) {
        errorMsg = QString("Duplicate ID: %1").arg(stored.uniqueID());
        return false;
    }
    
    // Insert into inventory
    m_templates.insert(stored.uniqueID(), stored);
    m_keys.append(stored.uniqueID());
    
    return true;
}
```

**In TemplateManager (for GUI copy operation):**
```cpp
void TemplateManager::copyTemplate(const QString& templateKey) {
    // Get the template from inventory
    ResourceTemplate original = m_inventory->getTemplate(templateKey);
    
    // Create working copy - preserve ID initially for user context
    ResourceTemplate workingCopy = original;
    
    // When user is done editing and wants to save as NEW:
    // GUI MUST clear the ID before passing to manager
    workingCopy.setUniqueID("");  // <- Caller responsibility
    
    // Then save as new
    QString errorMsg;
    if (m_inventory->addTemplateObject(workingCopy, location, errorMsg)) {
        // workingCopy now has NEW ID
        qDebug() << "New template created with ID:" << workingCopy.uniqueID();
    }
}
```

### Summary: When IDs Are Generated

| Operation | Generated? | When | By Whom |
|-----------|---|---|---|
| New template created in editor | ❌ No | - | - |
| New template saved | ✅ Yes | After validation | TemplatesInventory::addTemplateObject() |
| Template imported from file | ❌ No (from file) | - | - |
| Imported template added to inventory | ✅ Yes | After validation | TemplatesInventory::addTemplateObject() |
| Template edited and saved | ❌ No (keeps existing) | - | - |
| Template copied | ✅ Yes (NEW) | After validation | TemplatesInventory::addTemplateObject() |
| Template deleted | ❌ (removed) | - | TemplatesInventory::removeTemplate() |

---

## Data Flow Examples

### Example 1: Discovery Process (Imports Get ID After Validation)

**Flow:** Discovery → Manager → Inventory → Parser → File (ID assigned after validation)

```cpp
// In main application, when discovery completes:
QList<platformInfo::ResourceLocation> locations = discoveryProcess.getLocations();
templateManager->importTemplatesFromLocations(locations);

// In TemplateManager::importTemplatesFromLocations():
void TemplateManager::importTemplatesFromLocations(
    const QList<platformInfo::ResourceLocation>& locations)
{
    for (const auto& location : locations) {
        m_inventory->importFromLocation(location);
    }
}

// In TemplatesInventory::importFromLocation():
void TemplatesInventory::importFromLocation(
    const platformInfo::ResourceLocation& location)
{
    QString templatePath = location.path + "/templates";
    
    // Parse files using TemplateParser
    QDir dir(templatePath);
    for (const auto& entry : QDirListing(templatePath, {"*.json"})) {
        if (entry.isFile()) {
            ParseResult result = m_parser.parseFile(entry.filePath());
            if (result.success) {
                // Each parsed template has NO UniqueID yet
                for (auto tmpl : result.templates) {
                    // tmpl.uniqueID() is EMPTY at this point
                    QString errorMsg;
                    // addTemplateObject will generate ID after validation
                    if (!addTemplateObject(tmpl, location, errorMsg)) {
                        qWarning() << "Failed to add template from" << entry.filePath() 
                                   << ":" << errorMsg;
                    }
                    // Now tmpl.uniqueID() is populated
                }
            } else {
                qWarning() << "Failed to parse template file:" << entry.filePath() 
                           << "Error:" << result.errorMessage;
            }
        }
    }
}
```

### Example 2: GUI Import New Template (No UniqueID Until Save)

**Flow:** GUI Editor → Manager → Inventory (generates ID on save)

```cpp
// In GUI template editor, user clicks "New Template" and fills form
ResourceTemplate newTemplate;
newTemplate.setPrefix("my_new_template");
newTemplate.setBody("cube([${1:10}]);");
newTemplate.setDescription("My custom template");
// NOTE: newTemplate.uniqueID() is EMPTY at this point

// GUI signals manager to save - NO ID YET
templateManager->createTemplateFromEditor(
    newTemplate, 
    platformInfo::ResourceLocation(userTemplatesPath, ResourceTier::User)
);

// In TemplateManager::createTemplateFromEditor():
void TemplateManager::createTemplateFromEditor(
    const ResourceTemplate& newTemplate,
    const platformInfo::ResourceLocation& location)
{
    QString errorMsg;
    // Inventory generates ID on successful add
    if (m_inventory->addTemplateObject(newTemplate, location, errorMsg)) {
        // Success - template NOW has a UniqueID
        emit templateAdded(newTemplate.uniqueID());  // NOW populated
    } else {
        emit templateSaveFailed(errorMsg);
    }
}

// In TemplatesInventory::addTemplateObject():
bool TemplatesInventory::addTemplateObject(
    const ResourceTemplate& tmpl,
    const platformInfo::ResourceLocation& location,
    QString& errorMsg)
{
    errorMsg.clear();
    
    // Validate template (can be done before ID assignment)
    if (tmpl.prefix().isEmpty()) {
        errorMsg = "Template must have a prefix";
        return false;
    }
    
    // Create a copy for storage
    ResourceTemplate stored = tmpl;
    
    // CRITICAL: If no UniqueID, generate one NOW
    if (stored.uniqueID().isEmpty()) {
        QString newID = m_generateNewUniqueID(stored.prefix());
        stored.setUniqueID(newID);
    }
    
    // Insert into inventory
    int row = m_keys.size();
    beginInsertRows(QModelIndex(), row, row);
    m_templates.insert(stored.uniqueID(), stored);
    m_keys.append(stored.uniqueID());
    endInsertRows();
    
    return true;
}
```

### Example 3: GUI Copy Template + Modify + Save (Copy Gets NEW ID)

**Flow:** GUI requests copy (ID preserved) → GUI edits copy → Manager saves with NEW ID

```cpp
// In GUI, user selects template "1000-cube_basic" and clicks "Copy"
ResourceTemplate original = templateManager->getTemplateForCopy("1000-cube_basic");
// original.uniqueID() == "1000-cube_basic"

// GUI copies to editor - copies the object, but destination should NOT use the ID
// for saving as new (will get NEW ID on save)
ResourceTemplate workingCopy = original;
workingCopy.setPrefix("cube_basic_custom");  // Modify prefix
// workingCopy.uniqueID() still == "1000-cube_basic" at this point (in editor working copy)

// User modifies in GUI editor and clicks "Save As Copy"
ResourceTemplate modified = guiEditor->getModifiedTemplate();

// IMPORTANT: Clear the ID so it gets a NEW one on save
modified.setUniqueID("");  // Clear ID for this to be a NEW template

// GUI signals manager to save as new template
templateManager->saveModifiedTemplateCopy(
    modified,
    platformInfo::ResourceLocation(userPath, ResourceTier::User)
);

// In TemplateManager::saveModifiedTemplateCopy():
void TemplateManager::saveModifiedTemplateCopy(
    const ResourceTemplate& modified,
    const platformInfo::ResourceLocation& location)
{
    // modified.uniqueID() is empty (or will be cleared by inventory)
    QString errorMsg;
    if (m_inventory->addTemplateObject(modified, location, errorMsg)) {
        // modified now has a NEW UniqueID (different from original)
        emit templateSaved(modified.uniqueID());
    } else {
        emit templateSaveFailed(errorMsg);
    }
}

// In TemplatesInventory::addTemplateObject():
// [Same as Example 2 above - generates NEW ID because modified.uniqueID() is empty]
```

**Contrast with Editing an Existing Template:**

```cpp
// If user EDITS an existing template (not copy):
ResourceTemplate original = templateManager->getTemplateForCopy("1000-cube_basic");
// Modify in editor
original.setDescription("Modified description");

// Save back WITH the SAME ID
QString errorMsg;
if (m_inventory->updateTemplate("1000-cube_basic", original, errorMsg)) {
    // original.uniqueID() still == "1000-cube_basic"
    emit templateUpdated(original.uniqueID());
}
```

### Example 4: GUI File Export

**Flow:** GUI menu → Manager → Parser → Disk

```cpp
// In GUI menu, user selects "Export to JSON"
void MainWindow::onExportTemplate() {
    QString filePath = QFileDialog::getSaveFileName(this, "Export Template", "", "JSON (*.json)");
    if (!filePath.isEmpty()) {
        templateManager->exportToJsonFile(selectedTemplate, filePath);
    }
}

// In TemplateManager::exportToJsonFile():
void TemplateManager::exportToJsonFile(
    const QString& templateKey, 
    const QString& outputPath)
{
    // Get template from inventory (single source of truth)
    ResourceTemplate tmpl = m_inventory->getTemplate(templateKey);
    
    // Delegate file writing to parser
    if (!m_parser.writeToFile(outputPath, tmpl)) {
        emit exportFailed("Could not write to " + outputPath);
    } else {
        emit exportSucceeded(templateKey);
    }
}

// In TemplateParser::writeToFile():
bool TemplateParser::writeToFile(const QString& outputPath, 
                                 const ResourceTemplate& tmpl)
{
    QJsonObject jsonObj = templateToJson(tmpl);
    QJsonDocument doc(jsonObj);
    
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}
```

### Example 5: GUI Delete Template

**Flow:** GUI → Manager → Inventory (remove from memory) + Manager (delete file)

```cpp
// In GUI, user selects template and clicks "Delete"
templateManager->deleteTemplate("1000-cube_basic");

// In TemplateManager::deleteTemplate():
void TemplateManager::deleteTemplate(const QString& templateKey)
{
    // Get template info before removal
    ResourceTemplate tmpl = m_inventory->getTemplate(templateKey);
    QString filePath = getTemplateFilePath(tmpl);  // Need method to track file location
    
    // Remove from inventory (memory)
    if (m_inventory->removeTemplate(templateKey)) {
        // Delete file from disk
        QFile file(filePath);
        if (!file.remove()) {
            qWarning() << "Could not delete file:" << filePath;
            // Note: template still removed from inventory, file remains orphaned
            // Could emit warning signal
        }
    }
}
```

---

## Implementation Plan

## Phase 1: Extend TemplatesInventory API (Low Risk)

**Goal:** Give TemplatesInventory the CRUD methods it needs without breaking existing code

**Current Code Issue:**

The current `addTemplate(QDirEntry, location)` generates UniqueID immediately:

```cpp
// CURRENT CODE - WRONG TIMING for new templates
bool TemplatesInventory::addTemplate(const QDirEntry& entry, ...) {
    // ... validation ...
    QString uniqueID = ResourceIndexer::getUniqueIDString(baseName);  // <- Too early
    tmpl.setUniqueID(uniqueID);
    // ... insert ...
}
```

This is correct for IMPORTS (ID after validation) but wrong for NEW templates (should get ID only on save).

**Changes:**

- Keep `addTemplate(QDirEntry, location)` for IMPORTS - it already does the right thing (ID after validation)
- Add NEW `addTemplateObject(ResourceTemplate, location, errorMsg)` - generates ID for new/copied templates
- Add `getTemplate(key)` - return ResourceTemplate instead of QVariant
- Add `removeTemplate(key)` - remove from m_templates QHash (leave hole in ID sequence)
- Add `updateTemplate(key, updated, errorMsg)` - modify existing, keep ID
- Add `search(query)` - filter templates by prefix/description
- Add `importFromLocation()`, `importFromJsonFile()` - delegate to parser

**Files Modified:**

- `src/resourceInventory/TemplatesInventory.hpp` - Add method declarations
- `src/resourceInventory/TemplatesInventory.cpp` - Implement new methods
- `src/resourceInventory/ResourceIndexer.hpp` - May add `loadMaxIDFromInventory()` helper if needed

**Risk:** Low - all additions, no removals. Existing behavior unchanged.

**Critical Implementation: addTemplateObject()**

```cpp
bool TemplatesInventory::addTemplateObject(
    const ResourceTemplate& tmpl,
    const platformInfo::ResourceLocation& location,
    QString& errorMsg)
{
    errorMsg.clear();
    
    // Step 1: Validate template has required fields
    if (tmpl.prefix().isEmpty()) {
        errorMsg = "Template must have a non-empty prefix";
        return false;
    }
    
    if (tmpl.body().isEmpty()) {
        errorMsg = "Template must have content (body)";
        return false;
    }
    
    // Step 2: Create storage copy with tier
    ResourceTemplate stored = tmpl;
    stored.setTier(location.tier());
    
    // Step 3: ALWAYS generate NEW UniqueID
    //         (ID will be empty for new templates and copies - this is normal)
    QString newID = ResourceIndexer::getUniqueIDString(stored.prefix());
    stored.setUniqueID(newID);
    
    // Step 4: Insert into inventory
    int row = m_keys.size();
    beginInsertRows(QModelIndex(), row, row);
    m_templates.insert(newID, stored);
    m_keys.append(newID);
    endInsertRows();
    
    return true;
}

bool TemplatesInventory::updateTemplate(const QString& templateKey, 
                                       const ResourceTemplate& updated)
{
    // Template must exist
    if (!m_templates.contains(templateKey)) {
        return false;
    }
    
    // Create storage copy preserving the original ID
    ResourceTemplate stored = updated;
    stored.setUniqueID(templateKey);  // Keep original ID - never changes on update
    
    // Find row for signals
    int row = m_keys.indexOf(templateKey);
    
    // Replace entry
    m_templates[templateKey] = stored;
    
    // Signal model changed
    QModelIndex idx = index(row, 0);
    emit dataChanged(idx, index(row, columnCount() - 1));
    
    return true;
}

bool TemplatesInventory::removeTemplate(const QString& templateKey)
{
    if (!m_templates.contains(templateKey)) {
        return false;
    }
    
    int row = m_keys.indexOf(templateKey);
    beginRemoveRows(QModelIndex(), row, row);
    
    m_templates.remove(templateKey);
    m_keys.removeAt(row);
    
    endRemoveRows();
    return true;
}

ResourceTemplate TemplatesInventory::getTemplate(const QString& templateKey) const
{
    return m_templates.value(templateKey, ResourceTemplate());
}

QList<ResourceTemplate> TemplatesInventory::search(const QString& query) const
{
    QList<ResourceTemplate> results;
    for (const auto& key : m_keys) {
        const ResourceTemplate& tmpl = m_templates.value(key);
        if (tmpl.prefix().contains(query, Qt::CaseInsensitive) ||
            tmpl.description().contains(query, Qt::CaseInsensitive) ||
            tmpl.displayName().contains(query, Qt::CaseInsensitive)) {
            results.append(tmpl);
        }
    }
    return results;
}

QList<ResourceTemplate> TemplatesInventory::filterByTier(ResourceTier tier) const
{
    QList<ResourceTemplate> results;
    for (const auto& key : m_keys) {
        const ResourceTemplate& tmpl = m_templates.value(key);
        if (tmpl.tier() == tier) {
            results.append(tmpl);
        }
    }
    return results;
}

void TemplatesInventory::importFromLocation(
    const platformInfo::ResourceLocation& location)
{
    QString templatePath = location.path() + "/templates";
    QDir dir(templatePath);
    
    if (!dir.exists()) {
        qWarning() << "TemplatesInventory: Template directory not found:" << templatePath;
        return;
    }
    
    // Use TemplateParser to read files
    TemplateParser parser;
    QDirListing listing(templatePath, {"*.json"}, QDirListing::IteratorFlag::FilesOnly);
    
    for (const auto& entry : listing) {
        ParseResult result = parser.parseFile(entry.filePath());
        if (result.success) {
            // Each parsed template has NO UniqueID yet (will be assigned below)
            for (const auto& tmpl : result.templates) {
                QString errorMsg;
                if (!addTemplateObject(tmpl, location, errorMsg)) {
                    qWarning() << "Failed to add template from" << entry.filePath() 
                               << "Error:" << errorMsg;
                }
            }
        } else {
            qWarning() << "Failed to parse template file:" << entry.filePath() 
                       << "Error:" << result.errorMessage;
        }
    }
}
```

**Tests Added:** 

- `TEST_F(TemplatesInventoryTest, addTemplateObjectAlwaysGeneratesNewID)` - Verify addTemplateObject always generates a new ID regardless of input
- `TEST_F(TemplatesInventoryTest, addTemplateObjectRejectsEmptyPrefix)` - Verify validation rejects templates with empty prefix
- `TEST_F(TemplatesInventoryTest, addTemplateObjectRejectsEmptyBody)` - Verify validation rejects templates with empty body
- `TEST_F(TemplatesInventoryTest, removeTemplateCreatesHoleInSequence)` - Verify deleted template ID is not reused
- `TEST_F(TemplatesInventoryTest, updateTemplatePreservesIDAndDoesNotRevalidate)` - Verify update keeps original ID and replaces content
- `TEST_F(TemplatesInventoryTest, searchFiltersResults)` - Verify search filters by prefix, description, and name
- `TEST_F(TemplatesInventoryTest, filterByTierReturnsOnlyRequestedTier)` - Verify tier filtering works

---

### Phase 2: Enhance TemplateParser for Manager Use

**Goal:** Ensure TemplateParser has all methods manager needs

**Changes:**

- Already has `parseFile()` and `parseJson()` ✅
- Add `writeToFile(path, template)` - serialize single template
- Add `writeMultipleToFile(path, templates)` - serialize list
- Ensure all error reporting uses QJsonParseError ✅ (already done)
- No file deletion (manager's job)

**Files Modified:**

- `src/scadtemplates/template_parser.hpp` - Add write methods
- `src/scadtemplates/template_parser.cpp` - Implement write methods

**Risk:** Low - additive changes
**Tests Added:**

- Unit tests for JSON write operations
- Validate round-trip (write then read)

---

### Phase 3: Create TemplateManager (Orchestration)

**Goal:** New service layer that owns all higher-level workflows

**Changes:**

- New file: `src/scadtemplates/template_manager.hpp`
- New file: `src/scadtemplates/template_manager.cpp`
- Constructor takes reference to TemplatesInventory (not owned)
- Implement all orchestration methods from section above
- Signals for GUI notification (added, saved, deleted, errors)
- No inheritance - just a service class

**Files Created:**

- `src/scadtemplates/template_manager.hpp` - Interface and signals
- `src/scadtemplates/template_manager.cpp` - Implementation

**Files Modified:**

- `CMakeLists.txt` - Add manager to scadtemplates_lib target

**Risk:** Low - new code, doesn't affect existing systems
**Tests Added:**

- Integration tests for each workflow (import, export, create, copy, save, delete)
- Mock TemplatesInventory to verify delegation

---

### Phase 4: Update Application Integration

**Goal:** Integrate TemplateManager into main application

**Changes:**

- Create TemplateManager as application singleton (alongside TemplatesInventory)
- Update discovery process to call `templateManager->importTemplatesFromLocations()`
- Update GUI template editor to call manager methods instead of direct inventory/file operations
- Update existing code that calls inventory directly for file operations

**Files Modified:**

- `src/scadtemplates/scad_template_session.cpp` - May need to use manager for template operations
- `src/mainwindow.cpp` - Query manager, not inventory directly for templates
- GUI template editor files - Use manager for all mutations
- Discovery process integration point (TBD based on current code)

**Risk:** Medium - touches multiple subsystems
**Tests Added:**

- Integration tests with GUI components
- End-to-end test scenarios

---

### Phase 5: Clean Up Old Code

**Goal:** Remove duplication and simplified code

**Changes:**

- In TemplateManager, remove m_templates QHash (use inventory)
- Remove direct file operations from GUI (delegate to manager)
- Remove TemplatesInventory's direct usage in GUI event handlers
- Clean up TemplateSession if it was doing manager-level work

**Files Modified:**

- Review all usages of TemplatesInventory and TemplateManager
- Remove deprecated methods or helper functions

**Risk:** Medium-High - requires careful refactoring
**Tests Added:**

- Regression tests for all affected workflows

---

## File Structure After Refactoring

```
src/
├── resourceInventory/
│   ├── TemplatesInventory.hpp       (Enhanced CRUD)
│   ├── TemplatesInventory.cpp       (New inventory methods)
│   └── resourceItem.hpp
│
├── scadtemplates/
│   ├── template_manager.hpp         (NEW - Orchestrator)
│   ├── template_manager.cpp         (NEW - Implementation)
│   ├── template_parser.hpp          (Enhanced I/O)
│   ├── template_parser.cpp          (Write methods)
│   ├── scad_template_session.hpp    (Unchanged)
│   ├── scad_template_session.cpp    (Unchanged)
│   └── resource_item.hpp
│
└── ui/
    └── template_editor/             (Uses templateManager)
```

---

## Decision Log

| Decision | Rationale |
|----------|-----------|
| TemplateManager as separate class, not in TemplatesInventory | Single Responsibility: inventory ≠ orchestration; easier to test |
| TemplateParser stateless, no storage | Format handling is different from storage; easier to unit test |
| Manager owns file deletion, not inventory | Inventory is "what templates exist in memory"; Manager owns "what files exist on disk" |
| Inventory uses Parser internally | Avoid code duplication; Parser knows ALL format handling rules |
| Signals from Manager, not Inventory | Inventory is model (data changes); Manager is service (operation results) |
| Key generation in Inventory, not Parser | Parser is format-focused; key generation is business logic |

---

## Testing Strategy

### Unit Tests (Phase 1-2)

```cpp
// TemplatesInventory new methods
TEST_F(TemplatesInventoryTest, addTemplateObjectSucceeds) { ... }
TEST_F(TemplatesInventoryTest, removeTemplateSucceeds) { ... }
TEST_F(TemplatesInventoryTest, searchFiltersResults) { ... }

// TemplateParser write methods
TEST_F(TemplateParserTest, writeToFileCreatesValidJson) { ... }
TEST_F(TemplateParserTest, writeMultiplePreservesAllTemplates) { ... }
```

### Integration Tests (Phase 3-4)

```cpp
// Manager workflows
TEST_F(TemplateManagerTest, importFromLocationPopulatesInventory) { ... }
TEST_F(TemplateManagerTest, exportToJsonFileCreatesValidFile) { ... }
TEST_F(TemplateManagerTest, deleteRemovedFromInventoryAndDisk) { ... }
TEST_F(TemplateManagerTest, copyThenSaveCreatesNewTemplate) { ... }
```

### GUI Integration Tests (Phase 4)

```cpp
// End-to-end with GUI components
TEST_F(TemplateEditorTest, newTemplateSignalReachesManager) { ... }
TEST_F(TemplateEditorTest, deleteConfirmationCleansUpProperly) { ... }
```

---

## Success Criteria

- ✅ All three components (Manager, Parser, Inventory) have clear, non-overlapping responsibilities
- ✅ TemplatesInventory is the single source of truth for template storage
- ✅ Manager orchestrates all external operations (file I/O, discovery integration)
- ✅ Parser handled all format concerns (read, write, validation, conversion)
- ✅ No code duplication between Manager's old m_templates and Inventory's storage
- ✅ All existing tests pass
- ✅ New test coverage for all new methods
- ✅ GUI can query templates only through Manager
- ✅ File deletion is deterministic (Manager always responsible)

---

## Risk Mitigation

| Risk | Mitigation |
|------|-----------|
| Breaking existing code | Phase 1-2 are additive only; existing methods unchanged until Phase 5 |
| Integration complexity | Phase 3 creates Manager in isolation; Phase 4 integrates systematically |
| Test coverage gaps | Each phase has required test additions before moving to next |
| Multiple storage sources | Clear ownership: Inventory = memory, Manager = coordinated operations |
| Circular dependencies | Manager depends on Inventory and Parser; neither depends back (acyclic) |

---

## Next Steps

1. **Review this plan** - Confirm responsibilities and data flows align with your vision
2. **Approval gates** - Approve/adjust Phase 1 design before starting implementation
3. **Incremental commits** - Each phase is a complete, testable increment
4. **Continuous validation** - Tests before moving to next phase
