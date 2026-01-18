# Location-Based Template Indexing Implementation

**Date:** January 18, 2026  
**Branch:** resScannerOption1  
**Status:** ✅ Complete and Tested

---

## Problem Statement

Templates with identical filenames in different locations were being rejected as duplicates:
- `C:/Program Files/OpenSCAD/templates/difference.json`
- `C:/Program Files/OpenSCAD (Nightly)/templates/difference.json`
- `C:/Users/Jeff/AppData/Local/OpenSCAD/templates/difference.json`

**Before:** 55 templates loaded (33 rejected as duplicates)  
**After:** 88 templates loaded (all accepted)

---

## Solution Design

### Location Indexing System

Implemented centralized location index manager in `ResourceScanner` with:

1. **Dynamic Location Indexing**
   - Each unique templates folder gets a short index: "aaa", "aab", "aac", etc.
   - Base-26 encoding supports 17,576 unique locations (26³)
   - Bidirectional mapping: path ↔ index

2. **Unique Template IDs**
   - Format: `locationIndex-filename`
   - Examples: "aaa-cube", "aab-pyramid", "aah-difference"
   - Stored in ResourceTemplate.uniqueID field

3. **Path Normalization**
   - Convert backslashes to forward slashes before storage
   - Avoids escape character issues in C++ strings
   - Windows handles both `/` and `\` natively

---

## Implementation Details

### Files Modified

#### 1. ResourceScanner (Centralized Index Manager)
**Header:** `src/resourceScanning/ResourceScanner.hpp`
- Added static members: `s_pathToLocationIndex`, `s_locationIndexToPath`, `s_nextLocationIndex`
- Public methods:
  - `getOrCreateLocationIndex(path)` - Get/create index for folder
  - `getLocationPath(index)` - Reverse lookup
  - `locationIndexCount()` - Total indexed locations
  - `normalizePath(path)` - Convert backslashes to forward slashes

**Implementation:** `src/resourceScanning/ResourceScanner.cpp`
- `numberToLocationIndex(int)` - Converts 1→"aaa", 2→"aab", 27→"aba", etc.
- Thread-safe index generation
- Debug logging for new location registrations

#### 2. ResourceItem (Base Class Enhancement)
**File:** `src/resourceInventory/resourceItem.hpp`
- Added `m_uniqueID` member variable
- Added `uniqueID()` and `setUniqueID()` methods
- Available to all resource types (templates, examples, etc.)

#### 3. TemplatesInventory (Updated Key Generation)
**File:** `src/resourceInventory/TemplatesInventory.cpp`
- Changed from `tier-name` keys to `locationIndex-filename` keys
- Calls `ResourceScanner::getOrCreateLocationIndex(folderPath)`
- Stores uniqueID in ResourceTemplate object
- Added include: `../resourceScanning/ResourceScanner.hpp`

#### 4. Test Infrastructure
**File:** `tests/test_sa_scanner_output.cpp`
- Fixed to compile sources directly (not link DLL)
- Runtime app name override now works correctly
- Shows "OpenSCAD" paths instead of "ScadTemplates"

**CMakeLists.txt Updates:**
- test_sa_scanner_output compiles all required sources directly
- Added: template_parser.cpp, legacy_template_converter.cpp, JsonWriter.cc, JsonReader.cpp
- Links: nlohmann_json and json-schema-validator libraries

---

## Test Results

### Location Index Generation
```
ResourceScanner: Indexed location "aaa" = "C:/Program Files/OpenSCAD/templates"
ResourceScanner: Indexed location "aab" = "C:/Program Files/OpenSCAD (Nightly)/templates"
ResourceScanner: Indexed location "aac" = "D:/repositories/cppsnippets/cppsnippets/templates"
ResourceScanner: Indexed location "aad" = "C:/ProgramData/OpenSCAD/templates"
ResourceScanner: Indexed location "aae" = "C:/Users/Jeff/AppData/Local/OpenSCAD/templates"
ResourceScanner: Indexed location "aaf" = "D:/repositories/.../testFileStructure/inst/OpenSCAD/templates"
ResourceScanner: Indexed location "aag" = "D:/repositories/.../testFileStructure/inst/OpenSCAD (Nightly)/templates"
ResourceScanner: Indexed location "aah" = "D:/repositories/.../testFileStructure/pers/.../local/openscad/templates"
ResourceScanner: Indexed location "aai" = "D:/repositories/.../testFileStructure/pers/.../Documents/OpenSCAD/templates"
```

### Inventory Summary
- **Examples:** 209
- **Templates:** 88 (✅ +33 from before)
- **Fonts:** 29
- **Shaders:** 5
- **Translations:** 0
- **Tests:** 0

### No Duplicate Warnings
Previous behavior:
```
TemplatesInventory: Duplicate template key: Installation-difference
TemplatesInventory: Duplicate template key: User-color
...
```

New behavior:
```
✅ All templates accepted with unique location-based IDs
```

---

## Architecture Benefits

### 1. Correct Semantics
Templates with same filename in different locations ARE different resources:
- Installation vs Nightly build variations
- User customizations vs system defaults
- Per-location template libraries

### 2. Human-Readable Keys
- "aaa-cube" is clear and concise
- Easy to debug in logs
- Deterministic (same path = same index always)

### 3. No Collision Risk
- True uniqueness via path differentiation
- No hash collision concerns
- Scales to 17,576 locations

### 4. No Escape Issues
- Forward slash normalization
- Clean C++ string literals
- Works on Windows, Linux, macOS

### 5. GUI-Ready
Template picker can show:
- "difference (OpenSCAD)"
- "difference (Nightly)"
- "difference (My Templates)"

User can distinguish between variants.

---

## Related Changes

### App Name Override Fix
**Problem:** Test apps showed "ScadTemplates" in paths instead of configurable app name

**Solution:** Compile ResourcePaths.cpp directly into test executables
- DLL has compile-time app name baked in
- Test executables compile sources with USE_TEST_APP_INFO defined
- appInfo::setTestAppName() hook works when sources compiled directly

**Pattern:** See test_sa_location_discovery for reference implementation

---

## Future Considerations

### Nested Template Folders
Current design supports nested structures:
```
examples/
  category1/
    templates/  → Gets unique location index
      color.json
  templates/    → Gets different location index
    color.json
```

Each templates folder gets its own index automatically.

### Index Persistence
Location indices are session-specific (regenerated on each run). This is correct because:
- Indices are internal implementation detail
- Templates store full paths, not indices
- Consistent across restarts (same paths → same indices)

### GUI Display
ResourceTemplate.uniqueID enables:
- Fast inventory lookups by unique ID
- Location disambiguation in UI
- Conflict-free template management

---

## Build & Test Commands

```powershell
# Build test
cmake --build "d:\repositories\cppsnippets\cppsnippets\build" --config Debug --target test_sa_scanner_output --parallel 4

# Run test
.\build\bin\Debug\test_sa_scanner_output.exe --appname OpenSCAD

# Verify no duplicate warnings in output
.\build\bin\Debug\test_sa_scanner_output.exe --appname OpenSCAD | Select-String "Duplicate"
# (Should return nothing)
```

---

## Code Quality

### Static Members (ResourceScanner)
```cpp
static QHash<QString, QString> s_pathToLocationIndex;   // path → "aaa"
static QHash<QString, QString> s_locationIndexToPath;   // "aaa" → path
static int s_nextLocationIndex;                         // Counter
```

Initialized in ResourceScanner.cpp with proper static storage.

### Base-26 Encoding Algorithm
```cpp
QString ResourceScanner::numberToLocationIndex(int index)
{
    // 1 → "aaa", 2 → "aab", ..., 26 → "aaz", 27 → "aba"
    index--;  // Convert to 0-based
    int third = index % 26;
    int second = (index / 26) % 26;
    int first = (index / 676) % 26;  // 676 = 26²
    
    return QString() + QChar('a' + first) 
                     + QChar('a' + second) 
                     + QChar('a' + third);
}
```

### Path Normalization
```cpp
QString ResourceScanner::normalizePath(const QString& path)
{
    QString normalized = path;
    normalized.replace('\\', '/');
    return normalized;
}
```

Simple, effective, cross-platform.

---

## Testing Coverage

✅ Multiple templates with same filename accepted  
✅ Location indices generated correctly (aaa → aai)  
✅ Path normalization works (backslash → forward slash)  
✅ No duplicate warnings in logs  
✅ Template count increased from 55 to 88  
✅ Bidirectional path ↔ index lookup functional  
✅ App name override working in tests  

---

## Known Good Commit

This implementation is stable and tested. Safe to merge or build upon.

**Branch:** resScannerOption1  
**Commit:** [To be filled after commit]  
**Build Status:** ✅ Successful  
**Tests:** ✅ Passing (88 templates discovered)

---

## References

- **Planning Doc:** `doc/2025-12-28-resource-location-enablement-refactoring.md`
- **Test App:** `tests/test_sa_scanner_output.cpp`
- **Core Implementation:** `src/resourceScanning/ResourceScanner.cpp`
- **Inventory Integration:** `src/resourceInventory/TemplatesInventory.cpp`
