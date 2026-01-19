# Location-Based Template Indexing - Corrected Implementation Plan

**Date:** January 18, 2026  
**Branch:** resScannerOption1  
**Status:** ✅ COMPLETE - Phases 1-8 Complete  
**Commits:** 
- Phase 1-7: `0bf424f` - ResourceIndexer + TemplatesInventory (7/7 tests)
- Phase 8: `f82f614` - ExamplesInventory (8/8 tests)

**Previous Attempt:** Reverted (ff937f9) - Had architectural issues  
**Recovery Steps:** Git reset, documentation recovery, cleanup restoration

---

## Final Architecture: ResourceIndexer Solution

### ✅ What Was Actually Implemented

Instead of putting index generation in ResourceLocation, we created a **unified ResourceIndexer** class that manages indices for ALL resource types across ALL tiers.

**Key Innovation:** Compound key system prevents collisions:
```
Key Format: "{locationIndex}|{resourceType}|{baseName}"
Example: "1000|Templates|cube" → Index "1000"
Example: "1000|Examples|cube" → Index "1001"  
Example: "1001|Templates|cube" → Index "1002"
```

**Why This Works:**
- Same file in different tiers gets different location indices
- Same filename in different resource types gets different indices  
- Same filename in different locations gets different indices
- Thread-safe with QMutex
- Single static counter for ALL resources (starts at 1000)

---

## Implementation Summary

### Phase 1-2: ResourceLocation + ResourceItem ✅ COMPLETE

**Commit:** c037598

- ResourceLocation tracks base paths with unique indices
- ResourceItem has uniqueID member
- Foundation for indexing system

### Phase 3: TemplatesInventory API Redesign ✅ COMPLETE

**Commit:** 0bf424f (part of unified commit)

- Accept `ResourceLocation&` instead of tier string
- Friend pattern with private location-based constructor
- Uses `tryInsert()` for atomic duplicate detection
- Simplified `addFolder()` to take QString path

**Before:**
```cpp
bool addTemplate(const QDirListing::DirEntry& entry, 
                 const QString& tier,
                 const QString& locationIndex);
```

**After:**
```cpp
bool addTemplate(const QDirListing::DirEntry& entry, 
                 const platformInfo::ResourceLocation& location);
```

### Phase 4-6: CMakeLists + ResourceIndexer Creation ✅ COMPLETE

**Commit:** 0bf424f

Created `src/resourceInventory/ResourceIndexer.{hpp,cpp}`:

**API:**
```cpp
class ResourceIndexer {
public:
    static QString getOrCreateIndex(
        const ResourceLocation& location,
        ResourceType type,
        const QString& baseName);
    
    static QString getKey(const QString& index);
    static QString extractBaseName(const QString& key);
    static int count();
    static void clear();
};
```

**Implementation:**
- Static counter: `s_nextIndex = 1000`
- Bidirectional mappings: `s_keyToIndex`, `s_indexToKey`
- Thread-safe: `QMutex s_mutex`
- Compound key uses `ResourceTypeInfo::getResTypeString()`

### Phase 6: ResourceTypeInfo Enhancement ✅ COMPLETE

**Commit:** 0bf424f

Added `getResTypeString()` method:
```cpp
static QString getResTypeString(ResourceType type);
// Returns: "Templates", "Examples", "Fonts", etc.
```

### Phase 7: JSON Snippet Unwrapping ✅ COMPLETE

**Commit:** 0bf424f

Fixed modern template format parsing:
```cpp
// In TemplatesInventory::parseJsonFile()
// Unwrap: {"cube_basic": {"prefix": ..., "body": ...}} 
// Extract inner object for schema validation
```

**Format Details:**
- Wrapper object with snippet name as key
- Body is single string (not array - simplified from VS Code)
- One template per JSON file

### Phase 8: ExamplesInventory Application ✅ COMPLETE

**Commit:** f82f614

Applied same pattern to ExamplesInventory:

**ResourceScript Changes:**
- Added friend declaration for ExamplesInventory
- New private constructor: `ResourceScript(filePath, ResourceLocation&)`
- Uses ResourceIndexer for uniqueID generation

**ExamplesInventory Changes:**
- Accept `ResourceLocation&` instead of tier string
- `addFolder()` returns `int` (count) instead of `bool`
- Uses `tryInsert()` for atomic duplicate detection
- Simplified implementation (no manual key generation)

**ResourceScanner Updates:**
- scanExamplesAt() uses ResourceLocation directly
- Removed tierToString() conversion
- Pass location object to inventory methods

---

## Current State: Applied to 2 of 6 Resource Types

### ✅ Using ResourceIndexer (Fully Migrated)

1. **TemplatesInventory** - 7/7 tests passing
   - Uses ResourceType::Templates
   - Location-based constructor in ResourceTemplate
   - Accepts ResourceLocation& in API

2. **ExamplesInventory** - 8/8 tests passing  
   - Uses ResourceType::Examples
   - Location-based constructor in ResourceScript
   - Accepts ResourceLocation& in API

### ⏳ Still Using Old API (Not Yet Migrated)

3. **FontsInventory** - Uses path as key (no ResourceLocation)
   - API: `addFont(QString path, ResourceTier tier)`
   - Storage: `m_fonts: QHash<QString, QVariant>`
   - **Needs:** ResourceFont class + ResourceIndexer integration

4. **ShadersInventory** - Not examined yet
   - Likely similar to FontsInventory
   - **Needs:** Investigation + migration

5. **TestsInventory** - Uses tier as string (old API)
   - API: `addTest(QString scriptPath, QString tier)`
   - Uses ResourceScript but without location constructor
   - **Needs:** ResourceLocation& parameter + ResourceIndexer

6. **TranslationsInventory** - Not examined yet
   - **Needs:** Investigation + migration

---

## Key Design Principles (As Implemented)

### 1. Unified Index Counter

✅ Single static counter starting at 1000 for ALL resources  
✅ Shared across Templates, Examples, Fonts, Shaders, Tests, Translations  
✅ Thread-safe with QMutex  
✅ Compound key ensures uniqueness across types

### 2. Location-Based Constructors

✅ Private constructors taking `ResourceLocation&`  
✅ Only accessible via friend class (inventory)  
✅ Automatic uniqueID generation via ResourceIndexer  
✅ No manual index tracking in inventories

### 3. Compound Key System

✅ Format: `"{locationIndex}|{resourceType}|{baseName}"`  
✅ Example: `"1000|Templates|cube"` → Index "1000"  
✅ Prevents collisions across:
   - Different tiers (different location indices)
   - Different resource types (part of key)
   - Different basenames (part of key)

### 4. Friend Pattern + tryInsert()

✅ Inventory classes are friends of resource classes  
✅ Private location-based constructors enforce controlled creation  
✅ `tryInsert()` provides atomic duplicate detection  
✅ No separate contains() check needed

---

## Example Flow (As Implemented)

```
1. ResourceScanner creates ResourceLocation("/opt/OpenSCAD/templates", Installation)
   → Location gets index "1000" internally

2. ResourceScanner::scanTemplatesAt(location) called
   → Finds /opt/OpenSCAD/templates/cube.json

3. TemplatesInventory::addTemplate(entry, location) called
   → Creates ResourceTemplate(path, location)
   
4. ResourceTemplate constructor (private, friend-only)
   → Calls ResourceIndexer::getOrCreateIndex(location, Templates, "cube")
   → Builds key: "1000|Templates|cube"
   → Not in registry → assigns index "1000", stores mapping
   → Sets m_uniqueID = "1000-cube"

5. TemplatesInventory stores via tryInsert()
   → Key: "1000-cube"
   → Atomic duplicate check
   → Insert successful

Later, same file from User tier:

1. ResourceLocation("/home/user/.local/OpenSCAD/templates", User)
   → Gets index "1001"

2. ResourceTemplate constructor for cube.json
   → Calls ResourceIndexer::getOrCreateIndex(userLocation, Templates, "cube")
   → Builds key: "1001|Templates|cube"  
   → Not in registry (different from "1000|Templates|cube")
   → Assigns index "1002"
   → Sets m_uniqueID = "1002-cube"

Result: Same filename, different tiers → different unique IDs ✅
```

---

## Test Results

### TemplatesInventory (7/7 tests passing)
- ✅ AddTemplateWithHierarchicalKey
- ✅ DifferentTiersSameFile (INDEX COLLISION FIXED!)
- ✅ DuplicateKeyRejected
- ✅ GetAll
- ✅ AddFolderWithTemplates
- ✅ Clear
- ✅ JsonContentStructure

### ExamplesInventory (8/8 tests passing)
- ✅ AddExampleWithHierarchicalKey
- ✅ DifferentTiersSameFile (INDEX COLLISION FIXED!)
- ✅ DuplicateKeyRejected
- ✅ GetAll
- ✅ GetByCategory
- ✅ GetCategories
- ✅ AddFolderWithScripts
- ✅ Clear

**Total:** 15/15 tests passing

---

## Migration TODO for Remaining Inventories

### FontsInventory
- [ ] Create ResourceFont class (like ResourceTemplate/ResourceScript)
- [ ] Add friend declaration for FontsInventory
- [ ] Add private location-based constructor
- [ ] Update addFont() signature to accept ResourceLocation&
- [ ] Use ResourceIndexer::getOrCreateIndex() with ResourceType::Fonts
- [ ] Update ResourceScanner::scanFontsAt()
- [ ] Write/update tests with ResourceLocation objects

### ShadersInventory
- [ ] Investigate current implementation
- [ ] Similar pattern to Fonts if simple files
- [ ] Use ResourceType::Shaders with ResourceIndexer

### TestsInventory
- [ ] Already uses ResourceScript (good!)
- [ ] Add ResourceLocation& parameter to addTest()
- [ ] Use ResourceScript's location-based constructor
- [ ] ResourceIndexer with ResourceType::Tests
- [ ] Update ResourceScanner::scanTestsAt()
- [ ] Update tests

### TranslationsInventory
- [ ] Investigate current implementation
- [ ] Create appropriate resource class if needed
- [ ] Use ResourceType::Translations with ResourceIndexer

---

## Success Criteria ✅ ACHIEVED (Partial)

✅ Templates and Examples fully migrated  
✅ 15/15 tests passing (7 templates + 8 examples)  
✅ Zero duplicate warnings  
✅ Indices are "1000", "1001", "1002", ...  
✅ Template keys are "1000-cube", "1002-sphere", etc.  
✅ Examples keys are "1003-cube", "1004-customizer", etc.  
✅ Same file in different tiers gets unique indices  
✅ Build successful  
✅ DifferentTiersSameFile tests pass for both inventories  

⏳ Remaining: Apply pattern to Fonts, Shaders, Tests, Translations

---

## References

- **Commits:**
  - c037598 - Phase 1-2: ResourceLocation + ResourceItem
  - 0bf424f - Phases 3-7: ResourceIndexer + TemplatesInventory
  - f82f614 - Phase 8: ExamplesInventory
  
- **Code Files:**
  - `src/resourceInventory/ResourceIndexer.{hpp,cpp}` - Unified indexer
  - `src/resourceInventory/resourceItem.{hpp,cpp}` - ResourceTemplate/ResourceScript
  - `src/resourceInventory/TemplatesInventory.{hpp,cpp}` - Templates
  - `src/resourceInventory/ExamplesInventory.{hpp,cpp}` - Examples
  - `src/resourceMetadata/ResourceTypeInfo.{hpp,cpp}` - getResTypeString()

- **Original Doc:** `doc/2026-01-18-location-based-template-indexing.md`
- **Reverted Commit:** ff937f9 (had issues, lessons learned)
- **Current Branch:** resScannerOption1

---

## Recovery Actions Completed

### 1. ✅ Git Reset to Clean State

```bash
git reset --hard 81df442
# "some cleaning and tidying of code in mainwindow"
```

**Why:** Commit ff937f9 had architectural flaws that required complete rewrite.

### 2. ✅ Documentation Recovery

```bash
git show ff937f9:doc/2026-01-18-location-based-template-indexing.md > recovered.md
```

**Recovered:** Original planning document showing 88 templates discovered.  
**Purpose:** Preserve lessons learned and test expectations.

### 3. ✅ Cleanup Work Restoration

Restored all code quality improvements from ff937f9:

**resourceItem.hpp/cpp:**

- Removed `isValid()` methods (moved validation to callers)
- Removed state tracking members: `m_exists`, `m_isEnabled`, `m_isModified`, `m_lastModified`
- Removed string conversion functions: `resourceTypeToString()`, `stringToResourceType()`
- Added `loadFromJson()` methods to ResourceTemplate
- Added `m_lastError` member for error tracking
- Simplified constructors (no file existence checks)

**Why Important:** These cleanups improve code maintainability and were unrelated to the indexing bugs.

### 4. ✅ Architecture Decision: User Preference

**Original plan:** QHash<QString, int> for path→index mapping  
**User feedback:** "do this index as static QHash<QString, QString>"  
**Implemented:** Bidirectional string-based mapping

- `s_pathToIndex: QHash<QString, QString>` (path → "1000")
- `s_indexToPath: QHash<QString, QString>` ("1000" → path)

**Benefits:** Easier debugging, no int↔string conversion, simpler lookups.

### 5. ✅ Initialization Order Safety

**User concern:** "how can we be sure that the default actions on the index will happen in the correct order"  
**Solution:** Moved initialization from member initializer list to constructor body

```cpp
// Explicit sequential initialization in body
m_index = s_nextIndex++;
m_indexString = QString::number(m_index).rightJustified(4, '0');
m_uniqueID = QString("loc-%1").arg(m_indexString);
```

**Why:** Makes dependencies obvious, immune to member declaration order changes.

### Current Git State

- **Branch:** resScannerOption1
- **Local HEAD:** 81df442 (clean)
- **Origin:** ff937f9 (will diverge after commit)
- **Working Tree:** Phase 1 & 2 changes (uncommitted)
- **Files Modified:** 
  - `src/platformInfo/ResourceLocation.hpp`
  - `src/platformInfo/ResourceLocation.cpp`
  - `src/resourceInventory/resourceItem.hpp`
  - `src/resourceInventory/resourceItem.cpp`

---

## What Went Wrong in First Attempt

### ❌ Problem 1: Wrong Place for Index Generation

- **Mistake:** Put index manager in `ResourceScanner` as static members
- **Why Wrong:** Scanner is too late in the pipeline
- **Issue:** Templates from same folder got different indices

### ❌ Problem 2: Complex Base-26 Encoding

- **Mistake:** Used "aaa", "aab", "aac" encoding (78 lines of code)
- **User Feedback:** "this is a simpler way" (provided integer example)
- **Better:** Simple integers starting at 1000 ("1000", "1001", "1002")

### ❌ Problem 3: Changed Wrong Function Signature

- **Mistake:** Changed `TemplatesInventory::addTemplate()` to take `ResourceLocation&`
- **Why Wrong:** Creates new ResourceLocation per template file (different indices!)
- **Correct:** ResourceLocation represents base directory, not individual files

---

## Correct Architecture

### ✅ Index Assignment in ResourceLocation Constructor

**When:** During `ResourceLocation` object creation  
**Where:** `src/platformInfo/ResourceLocation.cpp` constructor  
**How:** Static registry checks if path already indexed

```cpp
// In ResourceLocation constructor:
QString normalizedPath = QString(p).replace('\\', '/');

if (s_pathToIndex.contains(normalizedPath)) {
    m_index = s_pathToIndex.value(normalizedPath);  // Reuse existing
} else {
    m_index = s_nextIndex++;                         // Create new
    s_pathToIndex.insert(normalizedPath, m_index);
}

m_indexString = QString::number(m_index).rightJustified(4, '0');
m_uniqueID = QString("loc-%1").arg(m_indexString);
```

### ✅ Simple Integer Indexing

**Format:** 1000, 1001, 1002, ...  
**String:** "1000", "1001", "1002", ... (4-digit zero-padded)  
**Capacity:** 9000 locations (1000-9999)

---

## Implementation Checklist

### Phase 1: Add Indexing to ResourceLocation ✅ COMPLETE

- ✅ **ResourceLocation.hpp** - Add members (int m_index, QString m_indexString, QString m_uniqueID)
- ✅ **ResourceLocation.hpp** - Add static members (s_nextIndex, s_pathToIndex, s_indexToPath)
- ✅ **ResourceLocation.hpp** - Add getters (getIndex(), getIndexString(), uniqueID())
- ✅ **ResourceLocation.hpp** - Add QHash include
- ✅ **ResourceLocation.cpp** - Initialize statics (s_nextIndex=1000, QHash instances)
- ✅ **ResourceLocation.cpp** - Update default constructor (explicit body initialization)
- ✅ **ResourceLocation.cpp** - Update path constructor (registry check, bidirectional mapping)
- ✅ **ResourceLocation.cpp** - Update copy constructor (copy all index members)

**Note:** Used bidirectional QHash<QString,QString> for string-based mapping per user preference.

### Phase 2: Add uniqueID to ResourceItem ✅ COMPLETE

- ✅ **resourceItem.hpp** - Add m_uniqueID member (QString)
- ✅ **resourceItem.hpp** - Add getter/setter (uniqueID(), setUniqueID())
- ✅ **resourceItem.hpp** - Cleanup: Removed isValid() methods
- ✅ **resourceItem.hpp** - Cleanup: Removed state tracking (m_exists, m_isEnabled, m_isModified, m_lastModified)
- ✅ **resourceItem.hpp** - Add loadFromJson() methods and m_lastError to ResourceTemplate
- ✅ **resourceItem.cpp** - Simplified constructors (no state init)
- ✅ **resourceItem.cpp** - Added loadFromJson() implementation
- ✅ **resourceItem.cpp** - Cleanup: Removed string conversion functions (resourceTypeToString, stringToResourceType)

**Note:** Restored all cleanup work from reverted commit ff937f9.

### Phase 3: Update TemplatesInventory ⏳ NEXT

- [ ] **Change signature:**

  ```cpp
  bool addTemplate(const QDirListing::DirEntry& entry, 
                   const QString& tier,
                   const QString& locationIndex);  // NEW parameter
  ```

- [ ] **Use passed locationIndex** in uniqueID generation:

  ```cpp
  QString uniqueID = QString("%1-%2").arg(locationIndex, baseName);
  template.setUniqueID(uniqueID);
  ```

### Phase 4: Update ResourceScanner

- [ ] **scanTemplatesAt():** Get locationIndex from location parameter:

  ```cpp
  QString locationIndex = location.getIndexString();
  ```

- [ ] **scanTemplatesAt():** Pass locationIndex to addTemplate():

  ```cpp
  m_templatesInventory.addTemplate(entry, tierStr, locationIndex)
  ```

### Phase 5: Test & Verify

- [ ] **Build:** Successful compilation
- [ ] **Test Run:** Execute `test_sa_scanner_output.exe`
- [ ] **Verify:** 88 templates loaded (not 55)
- [ ] **Check Logs:** Location indices are "1000", "1001", etc.
- [ ] **Verify:** No duplicate warnings
- [ ] **Check Keys:** Template keys are "1000-cube", "1001-pyramid", etc.

---

## Key Design Principles

### 1. One Index Per Base Path

✅ All templates from `C:/Program Files/OpenSCAD/templates/` share index "1000"  
✅ Templates from `C:/Program Files/OpenSCAD (Nightly)/templates/` get "1001"  
✅ User templates from `C:/Users/Bob/.../templates/` get "1002"

### 2. Index Assigned During Location Creation

✅ ResourceLocation constructor checks registry  
✅ Same normalized path → same index (deterministic)  
✅ New path → new index (auto-increment)

### 3. Templates Use Location's Index

✅ Template file determines its folder path  
✅ Creates temporary ResourceLocation to get index  
✅ Uses index string in unique ID: "1000-filename"

### 4. Immutable After Creation

✅ Index assigned in constructor initializer list  
✅ No setter for index (const after creation)  
✅ Copy constructor preserves index

---

## Example Flow

```
1. main.cpp creates ResourceLocation("/opt/OpenSCAD", Installation)
   → Constructor checks registry (not found)
   → Assigns index 1000
   → Stores mapping: "/opt/OpenSCAD" → 1000
   → Sets m_indexString = "1000"
   → Sets m_uniqueID = "loc-1000"

2. ResourceScanner::scanTemplatesAt(location) called
   → Finds /opt/OpenSCAD/templates/cube.json

3. TemplatesInventory::addTemplate(entry, "installation")
   → Gets folder: "/opt/OpenSCAD/templates"
   → Creates temp ResourceLocation("/opt/OpenSCAD/templates", User)
   → Constructor checks registry (not found)
   → Assigns NEW index 1001 ⚠️

PROBLEM: Template folder gets different index than base location!
```

### 🔧 Fix: Use Base Location's Index

Actually, we need to think about this more carefully. The question is:

**Should templates folder have its own index, or use the parent location's index?**

**Option A:** Templates folder "/opt/OpenSCAD/templates" gets its own index (1001)

- Pro: Technically correct - it's a different path
- Con: Not what we want - all templates from same installation should share index

**Option B:** Use parent location's index somehow

- Pro: Correct semantics - all templates from same installation = same index
- Con: How do we pass it?

**Solution:** ResourceScanner should pass the location's index to inventory!

---

## Revised Flow (Correct)

```
1. main.cpp creates ResourceLocation("/opt/OpenSCAD", Installation)
   → Index 1000 assigned

2. ResourceScanner::scanTemplatesAt(location) called
   → Has access to location object
   → location.getIndexString() = "1000"

3. TemplatesInventory needs the location's index
   → Pass location object? OR pass index string directly?
```

### Decision: Pass Index String

**Change TemplatesInventory signature:**

```cpp
bool addTemplate(const QDirListing::DirEntry& entry, 
                 const QString& tier,
                 const QString& locationIndex)  // Add this!
```

**In ResourceScanner:**

```cpp
QString locationIndex = location.getIndexString();
for (const auto& entry : QDirListing(templatesPath, {"*.json"})) {
    if (entry.isFile()) {
        if (m_templatesInventory.addTemplate(entry, tierStr, locationIndex)) {
            addedCount++;
        }
    }
}
```

**In TemplatesInventory:**

```cpp
bool TemplatesInventory::addTemplate(const QDirListing::DirEntry& entry,
                                      const QString& tier,
                                      const QString& locationIndex)
{
    QString baseName = fi.baseName();
    QString uniqueID = QString("%1-%2").arg(locationIndex, baseName);
    // ... rest unchanged
}
```

---

## Updated Implementation Checklist

### Phase 1: ResourceLocation (Same as before)

- [ ] Add index members and static registry
- [ ] Update constructors
- [ ] Add getters

### Phase 2: ResourceItem (Same as before)

- [ ] Add uniqueID member
- [ ] Add getter/setter

### Phase 3: Update TemplatesInventory (REVISED)

- [ ] **Change signature:**

  ```cpp
  bool addTemplate(const QDirListing::DirEntry& entry, 
                   const QString& tier,
                   const QString& locationIndex);  // NEW parameter
  ```

- [ ] **Remove:** ResourceLocation include (not needed)
- [ ] **Remove:** Temp ResourceLocation creation
- [ ] **Use:** Passed locationIndex directly in uniqueID

### Phase 4: Update ResourceScanner (REVISED)

- [ ] **scanTemplatesAt():** Get locationIndex from location parameter
- [ ] **scanTemplatesAt():** Pass locationIndex to addTemplate()
- [ ] **Remove:** Old location index static members and functions

### Phase 5: Test & Verify (Same as before)

---

## Success Criteria

✅ 88 templates loaded (all unique)  
✅ Zero duplicate warnings  
✅ Indices are "1000", "1001", "1002", ...  
✅ Template keys are "1000-cube", "1001-pyramid", etc.  
✅ Templates from same base location share same index  
✅ Build successful  
✅ Tests passing  

---

## References

- **Original Doc:** `doc/2026-01-18-location-based-template-indexing.md`
- **Reverted Commit:** ff937f9 (had issues, lessons learned)
- **Current Branch:** resScannerOption1 at 81df442 (clean state)
