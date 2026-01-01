# NewResources Container Integration - Discovery Pipeline Analysis

**Date**: 2025-12-26  
**Purpose**: Document all adaptations needed to handle the `NewResources` container type during resource discovery

---

## 1. Overview

The `NewResources` container is a special drop-zone folder structure that:
- Acts as a container (like `color-schemes/`) with subfolders for each resource type
- **Key difference**: Resources from `newresources/templates` are discovered and flattened as **direct children of the location**, not nested under a container node
- Discovered resources retain their full canonical path in tooltips/metadata so users can distinguish them from standard `location/templates` resources
- Initially supports only `Templates` type; future expansion can add `EditorColors` and `RenderColors` without folder nesting

**Folder Structure Example**:
```
User Location: C:\Users\Jeff\AppData\Local\ScadTemplates
├── templates/              ← Standard location: direct templates
│   ├── aaa.json
│   └── bbb.json
└── newresources/           ← NEW Container
    └── templates/          ← Drop zone for templates
        ├── import1.json
        └── import2.json
```

**Discovery Result**:
- All 4 templates appear as direct children of the User Location in the tree
- `aaa.json` → path: `...AppData\Local\ScadTemplates\templates\aaa.json`
- `bbb.json` → path: `...AppData\Local\ScadTemplates\templates\bbb.json`
- `import1.json` → path: `...AppData\Local\ScadTemplates\newresources\templates\import1.json` ← Tooltip shows full path
- `import2.json` → path: `...AppData\Local\ScadTemplates\newresources\templates\import2.json` ← Tooltip shows full path

---

## 2. Discovery Classes Affected

The resource discovery pipeline consists of these key components:

### 2.1 **resourcePaths.h** (Platform Info Module)
**Status**: ✅ **ALREADY UPDATED BY USER**

- `enum ResourceType` → Added `NewResources` 
- `s_resourceTypes` → Added entry for `NewResources` with subdirectories `{Templates}` (future: `{EditorColors, RenderColors}`)
- `resourceSubdirectory()` → Returns `"newresources"` for `ResourceType::NewResources`

**No further changes needed here.**

---

### 2.2 **resourceScannerDirListing.h / .cpp** (ResInventory Module)

**Purpose**: Scan directories using Qt 6.8+ `QDirListing` and stream discovered resources via callback.

#### Changes Required:

**A. `resourceSubfolder()` static method**
```cpp
// Current behavior: Maps ResourceType to folder name
// Example: ResourceType::Template → "templates"

// After NewResources support:
// - NewResources is NOT directly scanned as a type
// - Instead, handled as a CONTAINER within location scanning
// - NO entry needed here (NewResources is opaque to type-based scanning)
```

**B. `categorizeByPath()` static method**
```cpp
// Current: Analyzes file path to determine ResourceType
// Example: Path contains "/templates/" → ResourceType::Template

// After NewResources support:
// - Add detection for "/newresources/templates/" → Still return ResourceType::Template
// - Path like "/newresources/templates/foo.json" categorizes as Template
// - The full path is preserved in DiscoveredResource.path (canonical absolute path)
```

**C. `scanLocationForType()` method**
```cpp
// Current behavior: 
//   - Takes ResourceType (e.g., Template)
//   - Scans location/templates/
//   - Streams found resources via callback

// After NewResources support:
// - SAME BEHAVIOR for standard scanning
// - For Template type: Scan both:
//     1. location/templates/ (standard location)
//     2. location/newresources/templates/ (new resources)
// - Both sets streamed via callback with full paths preserved
```

**Recommended Implementation**:
```cpp
int ResourceScannerDirListing::scanLocationForType(
    const QString& basePath,
    ResourceType type,
    ResourceTier tier,
    const QString& locationKey,
    const ScanCallback& callback)
{
    int count = 0;
    
    // Standard location scan (unchanged)
    QString subfolder = resourceSubfolder(type);
    if (!subfolder.isEmpty()) {
        count += scanDirectory(basePath, subfolder, type, tier, locationKey, callback);
    }
    
    // NEW: Also scan newresources container if type is supported there
    if (type == ResourceType::Template) {
        count += scanDirectory(basePath, "newresources/templates", type, tier, locationKey, callback);
    }
    // Future: Add EditorColors and RenderColors here when needed
    
    return count;
}

// Helper method to scan a specific directory path
private:
int scanDirectory(const QString& basePath, const QString& relPath,
                  ResourceType type, ResourceTier tier, 
                  const QString& locationKey, const ScanCallback& callback)
{
    QString scanPath = QDir::cleanPath(basePath + QLatin1Char('/') + relPath);
    QDir scanDir(scanPath);
    
    if (!scanDir.exists()) {
        return 0;
    }
    
    // Existing scan logic (QDirListing, recursive, filters, etc.)
    // Resources discovered with full canonical path preserved
}
```

---

### 2.3 **resourceStore.h / .cpp** (ResInventory Module)

**Purpose**: Store discovered resources indexed by type and tier.

#### Changes Required:

**No structural changes needed.**

- `addResource()` — Already handles `DiscoveredResource` with full path
- `scanTypeAndStore()` — Already calls `scanLocationForType()` which will now discover both standard and newresources templates
- The full path is already preserved in `DiscoveredResource::path`

**Result**: When scanner feeds resources with newresources paths, they are automatically stored with correct paths.

---

### 2.4 **templateTreeModel.h / .cpp** (ResInventory Module)

**Purpose**: Build hierarchical tree model for display in QTreeView (Tier → Location → Template).

#### Changes Required:

**Current behavior**:
```
Installation (Tier)
├── examples/
│   └── [template resources]
├── templates/
│   └── [template resources]
└── libraries/
    └── [library resources]

User (Tier)
└── AppData/Local (Location)
    └── [template resources]
```

**After NewResources**:
- **NO CHANGE to tree structure** — NewResources templates still appear as direct children of Location
- The difference is visibility in **tooltips and metadata**

**Tooltip Enhancement**:
```cpp
// In data() method where tooltips are built
case Qt::ToolTipRole:
    if (node->hasResource()) {
        const DiscoveredResource& res = node->resource();
        // Existing: return res.displayName + " (" + res.locationKey + ")"
        // Enhanced: also include canonical path
        return res.displayName + "\n" + QDir::fromNativeSeparators(res.path);
    }
    break;
```

**Recommendation**: Enhance tooltip to show full path for clarity:
- Standard template: `C:\...\AppData\Local\ScadTemplates\templates\foo.json`
- NewResources template: `C:\...\AppData\Local\ScadTemplates\newresources\templates\foo.json`

---

### 2.5 **mainwindow.cpp** (Template Editor App)

**Purpose**: Drag-and-drop handling for template files.

#### Current Issue:
Dragging a template into the User tier folder results in duplicates:
- File copied to standard location → discovered as `aaa.json`
- File also discovered from newresources → `aaa_1.json`

#### Root Cause Analysis:

Current `processDroppedTemplates()` flow:
1. User selects User tier location or template
2. Find writable templates folder via `findNewResourcesTemplatesFolder()`
   - This method searches **only** for existing `location/templates` or `location/newresources/templates`
   - Returns first found (likely from another location)
3. Copy file to found folder
4. Call `refreshInventory()` which scans ALL user locations
5. Both `location/templates` AND `location/newresources/templates` now contain the file → Duplicate

#### Fix Already Implemented (You described):
- Drop destination should target the **selected location's** `newresources/templates` folder
- Not just any writable folder

#### No Model Changes Needed:
- `templateTreeModel` already handles this correctly
- Just need to drop into correct folder at source

---

### 2.6 **DiscoveredResource struct** (resourceScannerDirListing.h)

**Status**: ✅ **No changes needed**

Already contains:
```cpp
struct DiscoveredResource {
    QString path;              // Canonical absolute path ← Used as-is
    QString name;              // File name
    QString displayName;       // Display in UI
    QString locationKey;       // Which location this came from
    ResourceType type;         // Resource type
    ResourceTier tier;         // Tier (Installation/Machine/User)
    // ... other metadata
};
```

The full canonical path is already preserved, allowing tooltips to show distinction.

---

### 2.7 **ResourceScanner (older class)** (resourceScanner.h / .cpp)

**Status**: ⚠️ **VERIFY IF STILL IN USE**

This older class (non-`DirListing` variant) may be:
- Deprecated in favor of `ResourceScannerDirListing`
- Still used in other parts of codebase

**Action**: Search codebase for usage of `ResourceScanner::scanLocation()` (non-`DirListing` variant). If found and still used:
- Apply same changes as `ResourceScannerDirListing`
- Otherwise, mark as deprecated and update callers to use `ResourceScannerDirListing`

---

## 3. Summary of Changes

### ✅ Already Done (User)
- [ x ] `enum ResourceType` — Added `NewResources`
- [ x ] `s_resourceTypes` — Added entry with `newresources` subfolder

### ⚠️ To Do (Code Changes)

| File | Method | Change | Priority |
|------|--------|--------|----------|
| `resourceScannerDirListing.cpp` | `categorizeByPath()` | Detect `/newresources/templates/` paths as `ResourceType::Template` | High |
| `resourceScannerDirListing.cpp` | `scanLocationForType()` | Also scan `location/newresources/templates/` when type is Template | High |
| `templateTreeModel.cpp` | `data()` (ToolTipRole) | Enhance tooltip to show canonical path | Medium |
| `mainwindow.cpp` | `processDroppedTemplates()` | Already fixed to use selected location's newresources folder | ✅ Done |
| `resourceScanner.cpp` (if used) | `scanLocation()` / `scanTemplates()` | Mirror changes from `DirListing` variant | Medium |

### 🔍 To Verify

| Item | Action |
|------|--------|
| Usage of old `ResourceScanner` | Search for `ResourceScanner::scan*()` calls (non-`DirListing`) |
| Test coverage | Add tests for discovering templates from `newresources/templates` |
| Duplicate prevention | Confirm no file is discovered twice (even from two locations) |

---

## 4. Implementation Sequence

1. **Update `resourceScannerDirListing.cpp`**:
   - Modify `categorizeByPath()` to recognize `newresources/templates/` paths
   - Modify `scanLocationForType()` to also scan `newresources/templates/` container

2. **Test Resource Discovery**:
   - Verify templates from both `location/templates` and `location/newresources/templates` are discovered
   - Verify paths are full canonical paths
   - Verify no duplicates

3. **Enhance UI (Optional but Recommended)**:
   - Update tooltip in `templateTreeModel.cpp` to show full path
   - Users can distinguish between standard and imported templates

4. **Verify No Old Scanner Usage**:
   - Search codebase for non-`DirListing` `ResourceScanner` usage
   - Update if necessary

5. **Update Tests**:
   - Add test case: scanning location with both standard and newresources templates
   - Verify correct count and paths

---

## 5. Future Enhancements (Not Now)

When adding EditorColors and RenderColors to newresources:

```cpp
// In scanLocationForType():
if (type == ResourceType::Template) {
    count += scanDirectory(basePath, "newresources/templates", ...);
}
else if (type == ResourceType::EditorColors) {
    count += scanDirectory(basePath, "newresources/editorColors", ...);  // NEW
}
else if (type == ResourceType::RenderColors) {
    count += scanDirectory(basePath, "newresources/renderColors", ...);  // NEW
}
```

The tree model already handles flat discovery, so no UI changes needed.

---

## 6. Notes

- **No nesting**: NewResources templates appear as direct children of Location, not under a "NewResources" node
- **Tooltips provide clarity**: Full path in tooltip shows which templates were imported vs. standard
- **Drag-and-drop**: Already directs files to selected location's `newresources/templates` folder
- **Scalability**: Architecture supports adding EditorColors and RenderColors without tree restructuring
