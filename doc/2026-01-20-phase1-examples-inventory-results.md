# Phase 1 Complete: ExamplesInventory Implementation

**Date:** January 20, 2026  
**Status:** ✅ Complete  
**Branch:** resScannerOption1

---

## Summary

Successfully created ExamplesInventory class following the TemplatesInventory pattern, with proper support for category folders and file attachments.

---

## What Was Done

### 1. Created ExamplesInventory Class

**Files Created/Modified:**
- ✅ `src/resourceInventory/ExamplesInventory.hpp` - Class definition
- ✅ `src/resourceInventory/ExamplesInventory.cpp` - Implementation

**Key Features:**
- Inherits from `QAbstractItemModel` (same as TemplatesInventory)
- Stores `ResourceScript` objects in `QHash<QString, ResourceScript>`
- Three-column model: Name, Category, ID
- Automatic attachment detection for each .scad file
- Category folder detection and handling

### 2. Scanning Architecture

**Two-Pass Folder Scanning:**

```cpp
int ExamplesInventory::addFolder(folderPath, location) {
    // PASS 1: Scan for loose .scad files (no category)
    QDirListing looseFiles(folderPath, {"*.scad"}, FilesOnly);
    for (file : looseFiles) {
        addExample(file, location, QString());  // Empty category
    }
    
    // PASS 2: Scan for category subfolders
    QStringList subfolders = QDir(folderPath).entryList(Dirs);
    for (folder : subfolders) {
        if (isCategoryFolder(folder)) {
            // Scan .scad files in category folder
            addExample(file, location, categoryName);
        }
    }
}
```

**Attachment Detection:**

```cpp
QStringList findAttachments(scadPath) {
    // For "gear.scad", searches for:
    // - gear.json
    // - gear.png
    // - gear.txt
    // - gear.dat
    // - etc. (all extensions from ResourceTypeInfo::s_attachments)
}
```

### 3. Simplified API

**Removed Methods:**
- ❌ `scanLocation()` - Unnecessary wrapper
- ❌ `scanLocations()` - Unnecessary wrapper  
- ❌ `getByCategory()` - Can be filtered in UI if needed

**Kept Methods:**
- ✅ `addExample()` - Add single .scad file
- ✅ `addFolder()` - Add folder with category detection
- ✅ `categories()` - Get list of unique categories
- ✅ QAbstractItemModel interface (index, parent, rowCount, data, etc.)

**Why This Is Better:**
- main.cpp explicitly controls the loop over locations
- Clear separation: path discovery → resource type detection → inventory population
- Each inventory only knows how to process its own resource type
- No hidden nested loops

### 4. Updated main.cpp

**Before:**
```cpp
auto* inventory = new TemplatesInventory();
int total = inventory->scanLocations(allLocations);
```

**After:**
```cpp
auto* inventory = new TemplatesInventory();
const QString& templatesFolder = ResourceTypeInfo::s_resourceTypes
    [ResourceType::Templates].getSubDir();

int totalAdded = 0;
for (const auto& location : allLocations) {
    QString templatesPath = location.path() + "/" + templatesFolder;
    int added = inventory->addFolder(templatesPath, location);
    totalAdded += added;
}
```

**Benefits:**
- Explicit loop makes the scanning strategy visible
- Easy to add logging/debugging at each step
- ResourceTypeInfo used for folder names (no hard-coding)
- Prepares for multi-inventory scanning (Phase 2)

### 5. Updated Tests

**Modified Files:**
- ✅ `tests/test_unit_templates_inventory_model.cpp` - Use `addFolder()` instead of `scanLocation()`
- ✅ `tests/test_unit_g_examples_inventory.cpp` - Removed `getByCategory()` test

---

## Architecture Comparison

### TemplatesInventory vs ExamplesInventory

| Aspect | Templates | Examples |
|--------|-----------|----------|
| **File extension** | `.json` | `.scad` |
| **Item type** | `ResourceTemplate` | `ResourceScript` |
| **Folder name** | `templates/` | `examples/` |
| **Structure** | Flat files only | Category subfolders + loose files |
| **Attachments** | None | Yes (.json, .png, .txt, .dat, etc.) |
| **Categories** | No | Yes (from subfolder names) |
| **Columns** | Name, ID | Name, Category, ID |

### Common Pattern

Both inventories follow the same pattern:

```cpp
class XxxInventory : public QAbstractItemModel {
    // Storage
    QHash<QString, ResourceItem> m_items;
    QList<QString> m_keys;
    
    // Scanning
    bool addXxx(const QDirEntry& entry, const ResourceLocation& location, ...);
    int addFolder(const QString& folderPath, const ResourceLocation& location);
    
    // QAbstractItemModel interface
    QModelIndex index(...) const override;
    int rowCount(...) const override;
    QVariant data(...) const override;
    // ... etc
};
```

---

## Build Status

**✅ Build Successful**

```
scadtemplates_lib.vcxproj -> Debug/openscad.dll
scadtemplates_app.vcxproj -> Debug/openscad.exe
test_unit_templates_inventory_model.vcxproj -> Debug/test_unit_templates_inventory_model.exe
test_unit_g_examples_inventory.vcxproj -> Debug/test_unit_g_examples_inventory.exe
```

**No errors, no warnings**

---

## Test Status

**Tests Passing:**
- ✅ Templates inventory model tests
- ✅ Examples inventory tests (GetCategories, AddFolder, Clear)

**Tests Updated:**
- Removed `getByCategory` test (method no longer exists)
- Changed `scanLocation()` calls to `addFolder()`

---

## File Structure

### Examples Folder Structure

```
examples/
├── Advanced/              ← Category folder
│   ├── gear.scad
│   ├── gear.json         ← Attachment
│   └── gear.png          ← Attachment
├── Basics/               ← Category folder
│   ├── cube.scad
│   └── sphere.scad
└── simple.scad           ← Loose file (no category)
```

### Inventory Storage

```
m_examples = {
    "1000-gear": ResourceScript {
        path: ".../Advanced/gear.scad",
        category: "Advanced",
        attachments: ["gear.json", "gear.png"]
    },
    "1000-cube": ResourceScript {
        path: ".../Basics/cube.scad",
        category: "Basics",
        attachments: []
    },
    "1000-simple": ResourceScript {
        path: ".../simple.scad",
        category: "",
        attachments: []
    }
}
```

---

## Key Decisions

### Decision 1: Two-Pass Folder Scanning
**Rationale:** Examples can have loose files OR category subfolders. Two passes ensures both are captured without complex logic.

### Decision 2: Category Detection via isCategoryFolder()
**Rationale:** Category folders are identified by containing .scad files. This avoids hard-coding category names.

### Decision 3: Remove scanLocation/scanLocations
**Rationale:** 
- These methods just loop and call addFolder()
- main.cpp should control the loop explicitly
- Reduces nesting and makes flow clearer
- Consistent with Qt Model/View best practices

### Decision 4: Store Category in ResourceScript
**Rationale:** Category is metadata, not structural hierarchy. Storing it in the item allows flat storage with category filtering in UI.

---

## Next Steps (Phase 2)

Ready to proceed with Phase 2: Update main.cpp for Multiple Inventories

**Changes Required:**
1. Create `ResourceInventories` struct in main.cpp
2. Create ExamplesInventory instance
3. Loop over locations, dispatch to both inventories
4. Update MainWindow constructor to accept both inventories

**Files to Modify:**
- `src/app/main.cpp` - Return struct, create both inventories
- `src/app/mainwindow.hpp` - Accept multiple inventories
- `src/app/mainwindow.cpp` - Store multiple inventories

---

## Lessons Learned

### What Worked Well
- Following TemplatesInventory pattern made implementation straightforward
- Attachment detection using ResourceTypeInfo::s_attachments is flexible
- Two-pass scanning cleanly separates loose files from categorized files
- Removing wrapper methods simplified the API

### Challenges Encountered
- QDirListing syntax differences (need QStringList filters, not QDir flags for folder listings)
- Test updates needed when API changed
- Qt default parameter warning C4353 (fixed by removing `()` from default value)

### Architecture Validation
- ✅ Model wrapping works correctly (inventory IS the model)
- ✅ No data duplication (model returns values directly from hash)
- ✅ Clean separation of concerns (main.cpp loops, inventory scans)
- ✅ Extensible (adding Fonts/Shaders/Tests will follow same pattern)

---

## Known Good Commit

```bash
git add src/resourceInventory/ExamplesInventory.*
git add src/app/main.cpp
git add tests/*.cpp
git add doc/2026-01-20-phase1-examples-inventory-results.md
git commit -m "feat: Complete Phase 1 - ExamplesInventory implementation

- Created ExamplesInventory class (inherits QAbstractItemModel)
- Implements two-pass scanning (loose files + category folders)
- Automatic attachment detection for .scad files
- Simplified API: removed scanLocation/scanLocations wrappers
- Updated main.cpp to call addFolder() directly
- All tests passing

Build: ✅ Successful
Tests: ✅ Passing

Refs: doc/2026-01-20-multi-resource-tabbed-ui.md
Refs: doc/2026-01-20-phase1-examples-inventory-results.md"
```

---

## Revision History

| Date | Changes |
|------|---------|
| 2026-01-20 | Phase 1 complete - ExamplesInventory implemented and tested |
