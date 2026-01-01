# Resource Discovery Specifications
**Date**: December 22, 2025  
**Status**: Design Clarifications from December 21 Work

---

## Tier-Level Path Suffix Handling

### Installation Tier (Tier 1)
- ✅ **Suffix applied**: Paths ending with `/` get `openscad<suffix>` appended
- **Purpose**: Discover sibling installations (e.g., stable vs. nightly builds)
- **Scope**: Only checked in primary installation location
- **Developer exception**: Dev snapshots elsewhere require manual configuration

### Machine Tier (Tier 2)
- ❌ **No suffix**: Resources are version-agnostic
- **Rationale**: Admin-managed resources shared across all OpenSCAD versions
- **Implementation**: machineSearchPaths() method IS required (similar to appSearchPaths)
- **Status**: New functionality, no legacy behavior to maintain

### User Tier (Tier 3)
- ❌ **No suffix**: Resources shared between all OpenSCAD installations
- **Discovery**: Only by user intervention
- **Rationale**: Users manage one set of resources for all versions

---

## Resource Folder Structures

### Color Schemes (Special Hierarchy)

**Location**: Optional `color-schemes/` folder in any tier location

**Structure**:
```
<location>/
  └── color-schemes/          ← Optional container
      ├── editor/             ← ONLY "editor" allowed
      │   ├── scheme1.json
      │   └── scheme2.json
      └── render/             ← ONLY "render" allowed
          ├── metallic.json
          └── high-contrast.json
```

**Rules**:
- Only `editor/` or `render/` subfolders allowed inside `color-schemes/`
- JSON files in `editor/` → ResourceType::EditorColors
- JSON files in `render/` → ResourceType::RenderColors
- **Filename ≠ Name**: JSON filename does NOT need to match "name" field in file content
- **Discovery**: Separate scanning for editor vs. render (they have different JSON structures)
- **Display Name**: Use "name" field from JSON, not filename

**Why Separate**:
- Different JSON structures for editor vs. render schemes
- OpenSCAD editor discovers editor schemes internally
- Main window discovers render schemes for 3D view
- This project mirrors OpenSCAD's discovery pattern

---

### Templates (Flat Structure)

**Location**: `templates/` subfolder of tier location

**Structure**:
```
<location>/
  └── templates/              ← ONLY here, no deeper
      ├── template1.json      ← ONLY .json allowed
      ├── template2.json
      └── legacy.json
```

**Rules**:
- **No subfolders**: Templates are flat, no category nesting
- **Only .json files**: Ignore ALL other extensions (especially .scad)
- ❌ **Never .scad files**: Templates are JSON metadata, not OpenSCAD scripts

---

### Examples (One-Level Categories)

**Location**: `examples/` subfolder of tier location

**Structure**:
```
<location>/
  └── examples/
      ├── basic.scad          ← Direct examples (no category)
      ├── cube.scad
      ├── Basics/             ← Category subfolder (ONE level only)
      │   ├── example1.scad   ← Primary file
      │   ├── example1.png    ← Attachment
      │   └── example1.json   ← Attachment
      └── Advanced/           ← Another category
          ├── mesh.scad
          ├── mesh.stl
          └── mesh.dat
```

**Rules**:
- **Primary files**: .scad scripts
- **Attachments**: .json, .txt, .dat, .png, .jpg, .jpeg, .svg, .gif, .csv, .stl, .off, .dxf
- **Categories**: ONE level of subfolders allowed (ResourceType::Group)
- **Group capture**: Folder name becomes display name (via `groupNameCapture` signal)

---

### Tests (Flat with Templates)

**Location**: `tests/` subfolder of tier location

**Structure**:
```
<location>/
  └── tests/
      ├── test1.scad          ← Test script
      ├── test1.png           ← Attachment
      ├── test2.scad
      └── templates/          ← Optional templates subfolder
          ├── test_template1.json
          └── test_template2.json
```

**Rules**:
- **Primary files**: .scad scripts (same as Examples)
- **Attachments**: Same list as Examples
- **No categories**: Tests are flat (no Group subfolders)
- **May contain templates**: Optional `templates/` subfolder

---

## Resource Type Definitions

### Attachment Extensions (Shared)
```cpp
static const QStringList attachmentsList = {
    QStringLiteral(".json"), QStringLiteral(".txt"), QStringLiteral(".dat"),
    QStringLiteral(".png"),  QStringLiteral(".jpg"), QStringLiteral(".jpeg"),
    QStringLiteral(".svg"),  QStringLiteral(".gif"), QStringLiteral(".csv"),
    QStringLiteral(".stl"),  QStringLiteral(".off"), QStringLiteral(".dxf")
};
```

### Sub-Resource Types
```cpp
// Examples can contain Groups (categories) and Templates
static const QVector<ResourceType> exampleSubResTypes = {
    ResourceType::Group,
    ResourceType::Templates
};

// Tests can contain Templates
static const QVector<ResourceType> testSubResTypes = {
    ResourceType::Templates
};
```

### ResourceType Enum
```cpp
enum class ResourceType {
    Unknown,
    ColorSchemes,   ///< Container folder
    RenderColors,   ///< Render color scheme (.json)
    EditorColors,   ///< Editor color scheme (.json)
    Font,           ///< Font file (.ttf, .otf)
    Library,        ///< OpenSCAD library (folder with .scad files)
    Example,        ///< Example script with attachments
    Test,           ///< Test script with attachments
    Template,       ///< Template (.json only)
    Group,          ///< Category folder (folder name captured as display name)
    Shader,         ///< Shader file
    Translation     ///< Translation file (.ts, .qm)
};
```

---

## Special Signals

### Group Name Capture
```cpp
// Defined in resourcePaths.h
static const QString groupNameCapture = QStringLiteral("__capture__");
```

**Purpose**: Signals to scanning code that the folder name should be captured and stored in the ResourceItem's name field.

**Usage**: When `ResourceTypeInfo::subdirectory == groupNameCapture`, scanner captures the actual folder name as the group's display name.

---

## Implementation Notes

### String Handling for Unicode
- ✅ **Use**: `QStringLiteral()`  
- ❌ **Avoid**: `QLatin1String()` (won't work with tr() for internationalization)
- **Future**: May need wide character support for full Unicode handling

### Column Header Fix Status
- **TemplateTreeModel column 2**: Correctly labeled as "Name" (not "Path")
- **Status**: No change needed

### Machine Tier Method
- **Required**: `machineSearchPaths()` method similar to `appSearchPaths()`
- **Behavior**: Returns machine tier paths WITHOUT suffix appending

---

## Reference Code Locations

### Resource Type Definitions
- **File**: `src/platformInfo/resourcePaths.cpp`
- **Starting Line**: 33
- **Contains**: attachmentsList, subResTypes, s_resourceTypes vector

### Group Name Capture Constant
- **File**: `src/platformInfo/resourcePaths.h`
- **Line**: 43
- **Definition**: `static const QString groupNameCapture = QStringLiteral("__capture__");`

### Resource Type Enum
- **File**: `src/resInventory/resourceItem.h`
- **Contains**: ResourceType, ResourceTier, ResourceAccess enums

---

## Summary Table

| Resource | Container | Primary Extension | Attachments | Subfolders | Notes |
|----------|-----------|-------------------|-------------|------------|-------|
| **ColorSchemes** | Yes | N/A | N/A | editor/, render/ only | Container only, no files directly |
| **EditorColors** | No | .json | None | N/A | Inside color-schemes/editor/ |
| **RenderColors** | No | .json | None | N/A | Inside color-schemes/render/ |
| **Templates** | No | .json | None | ❌ None | Flat, no .scad files |
| **Examples** | Yes | .scad | 12 types | ✅ 1 level (Groups) | Categories allowed |
| **Tests** | Yes | .scad | 12 types | ❌ None | May have templates/ subfolder |
| **Group** | No | .scad | 12 types | N/A | Folder name captured |
| **Fonts** | No | .ttf, .otf | None | N/A | System font supplements |
| **Libraries** | Yes | .scad | Various | Nested | Complex hierarchy |

---

## Design Decisions Summary

1. ✅ **Suffix only at Installation tier** - User/Machine resources are version-agnostic
2. ✅ **machineSearchPaths() required** - New functionality, no legacy constraints
3. ✅ **Color schemes separated** - Different structures, different discovery paths
4. ✅ **Templates are JSON only** - No .scad files, flat structure
5. ✅ **Examples have categories** - One level of Group subfolders
6. ✅ **Tests are flat** - No category subfolders (except optional templates/)
7. ✅ **Group name capture** - Folder name becomes display name
8. ✅ **QStringLiteral everywhere** - Prepare for tr() internationalization
