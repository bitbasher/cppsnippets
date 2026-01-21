# Phase 1 Complete: ExamplesInventory Tree Model

**Date:** January 21, 2026  
**Status:** ✅ Complete  
**Branch:** resScannerOption1

---

## What Was Done

Refactored `ExamplesInventory` to inherit from `QAbstractItemModel` with two-level tree structure for QTreeView display.

### Files Modified

1. **src/resourceInventory/ExamplesInventory.hpp**
   - Added inheritance: `class ExamplesInventory : public QAbstractItemModel`
   - Added Q_OBJECT macro
   - Added constructor: `explicit ExamplesInventory(QObject* parent = nullptr)`
   - Added QAbstractItemModel method declarations:
     - `index()`, `parent()`, `rowCount()`, `columnCount()`
     - `data()`, `headerData()`, `flags()`
   - Changed storage from `QHash<QString, QVariant>` to `QHash<QString, ResourceScript>`
   - Added category-to-IDs mapping: `QHash<QString, QList<QString>> m_categoryToIds`
   - Added sorted category keys: `QStringList m_categoryKeys`
   - Added helper methods: `categoryRow()`, `scriptRowInCategory()`

2. **src/resourceInventory/ExamplesInventory.cpp**
   - Added constructor implementation with QObject parent
   - Updated `addExample()` to populate category mappings and emit model signals
   - Implemented all QAbstractItemModel methods for two-level tree
   - Added `clear()` method using `beginResetModel()`/`endResetModel()`
   - Updated `getAll()`, `get()`, `getByCategory()` to work with new storage
   - Added `#include <QFont>` for bold category display

---

## Build Status

✅ **Successful**

Build command:
```powershell
cmake --build "d:\repositories\cppsnippets\cppsnippets\build" --config Debug --parallel 4
```

All targets compiled without errors.

---

## Test Results

✅ **All 11 tests passing**

Test executable: `test_unit_g_examples_attachments_categories.exe`

```
[==========] Running 11 tests from 1 test suite.
[----------] 11 tests from ExamplesAttachmentsAndCategoriesTest
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.AttachmentDetectionInRootFolder
[       OK ] ExamplesAttachmentsAndCategoriesTest.AttachmentDetectionInRootFolder (1 ms)
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.AttachmentDetectionInCategoryFolder
[       OK ] ExamplesAttachmentsAndCategoriesTest.AttachmentDetectionInCategoryFolder (0 ms)
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.MultipleAttachmentFormats
[       OK ] ExamplesAttachmentsAndCategoriesTest.MultipleAttachmentFormats (2 ms)
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.NoAttachmentsWhenMissing
[       OK ] ExamplesAttachmentsAndCategoriesTest.NoAttachmentsWhenMissing (0 ms)
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.CategoryExtractionFromFolderName
[       OK ] ExamplesAttachmentsAndCategoriesTest.CategoryExtractionFromFolderName (0 ms)
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.EmptyCategoryForLooseFiles
[       OK ] ExamplesAttachmentsAndCategoriesTest.EmptyCategoryForLooseFiles (0 ms)
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.MultipleCategories
[       OK ] ExamplesAttachmentsAndCategoriesTest.MultipleCategories (0 ms)
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.GetByCategory
[       OK ] ExamplesAttachmentsAndCategoriesTest.GetByCategory (0 ms)
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.GetByNonexistentCategory
[       OK ] ExamplesAttachmentsAndCategoriesTest.GetByNonexistentCategory (0 ms)
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.AttachmentsInCategoryFolders
[       OK ] ExamplesAttachmentsAndCategoriesTest.AttachmentsInCategoryFolders (0 ms)
[ RUN      ] ExamplesAttachmentsAndCategoriesTest.GlobalUniqueIDsAcrossTiersAndCategories
[       OK ] ExamplesAttachmentsAndCategoriesTest.GlobalUniqueIDsAcrossTiersAndCategories (0 ms)
[----------] 11 tests from ExamplesAttachmentsAndCategoriesTest (9 ms total)
[==========] 11 tests from 1 test suite ran. (9 ms total)
[  PASSED  ] 11 tests.
```

---

## Architecture Changes

### Storage Architecture

**Before:**
```cpp
// Simple flat hash with file paths as keys
QHash<QString, QVariant> m_scripts;  // Key: file path
```

**After:**
```cpp
// Flat storage with uniqueID keys from ResourceIndexer
QHash<QString, ResourceScript> m_scripts;  // Key: uniqueID (O(1) lookup)

// Category grouping for tree presentation
QHash<QString, QList<QString>> m_categoryToIds;  // Category → Script IDs
QStringList m_categoryKeys;  // Sorted categories (empty first)
```

### Tree Structure (Virtual via Model Indexes)

```
Root (invisible)
  ├── (Loose Files) - empty category QString() for uncategorized examples
  │     ├── goodIndex.scad (uniqueID: "0")
  │     └── example.scad (uniqueID: "1")
  ├── indexCat - category folder
  │     ├── goodIndex.scad (uniqueID: "2")
  │     └── anotherFile.scad (uniqueID: "3")
  └── Installation - another category
        └── example.scad (uniqueID: "4")
```

The tree exists through `QModelIndex` encoding:
- **Parent nodes (categories):** `createIndex(row, column, quintptr(row))`
- **Child nodes (scripts):** `createIndex(row, column, (parentRow << 32) | childRow)`

### Model Methods Implementation

**index():** Creates indexes for categories (top-level) and scripts (children)
**parent():** Extracts parent row from internalId() high bits
**rowCount():** Returns category count at top level, script count for children
**data():** Returns category names (bold font) or script display names
**flags():** Categories are non-editable, scripts are editable

---

## Issues Encountered

### Missing QFont Include
**Problem:** Compilation error: `error C2079: 'font' uses undefined class 'QFont'`  
**Cause:** Used `QFont` in `data()` method without including header  
**Solution:** Added `#include <QFont>` to ExamplesInventory.cpp  

---

## Design Decisions

### Why Flat Storage + Category Mapping?

**Considered alternatives:**
1. Compute categories on-the-fly in model methods (rejected - too slow)
2. QSortFilterProxyModel for grouping (rejected - adds complexity, only need one view)
3. Current approach: Pre-computed category mappings (chosen)

**Rationale:**
- O(1) lookups by uniqueID (efficient)
- O(1) category insertion with maintained sort order
- Flexible for future features (attachment display, tier badges, inline editing)
- Consistent with TemplatesInventory pattern

### Category Sorting

Empty category (loose files) always appears first, followed by alphabetically sorted categories. This matches user requirement that loose examples are easily accessible.

---

## What Works Now

✅ ExamplesInventory inherits from QAbstractItemModel  
✅ Two-level tree structure (categories + scripts)  
✅ Flat hash storage with uniqueID keys  
✅ Category-to-IDs mapping for efficient grouping  
✅ All existing tests pass  
✅ Attachment detection still works  
✅ Category extraction still works  
✅ Global unique IDs across tiers  

---

## What's Next (Phase 2)

Update `main.cpp::resourceManager()` to:
1. Create both `TemplatesInventory` and `ExamplesInventory`
2. Scan filesystem for both resource types
3. Return struct containing both inventories
4. Pass both to MainWindow constructor

See: doc/2026-01-20-multi-resource-tabbed-ui.md Phase 2
