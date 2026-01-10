# Phase 2A: ResourceDiscovery - Location Validation (Revised)

**Date:** January 10, 2026  
**Status:** Ready for Implementation  
**Author:** GitHub Copilot (Claude Sonnet 4.5)  
**Phase:** 2A - Resource Location Discovery and Validation

---

## Context: Completed Namespace Reorganization

✅ **Steps 1-7 from 2026-01-09 roadmap COMPLETE:**

1. Namespace structure established:
   - `platformInfo` - OS/platform abstractions
   - `resourceMetadata` - Type definitions and tier enums
   - `pathDiscovery` - Candidate path generation
   - `resourceInventory` - Storage structures

2. Path discovery system fully functional:
   - Environment variable expansion
   - Folder name rules with suffix handling
   - Sibling installation detection
   - QSet-based deduplication
   - QStandardPaths integration (Documents folder)
   - QCoreApplication::applicationDirPath() for exe location
   - User-designated paths from QSettings
   - Produces 12 qualified paths with tier tags

3. Test infrastructure:
   - `test_path_discovery.cpp` validates entire workflow
   - 68 Google tests passing
   - Clean production-ready code (debug traces removed)

**Current State:**
```
pathDiscovery::ResourcePaths → QList<PathElement> with tier tags
                                    ↓
                           [Phase 2A goes here]
                                    ↓
                        QList<ResourceLocation> validated
```

---

## Design Philosophy

**Keep It Simple:** This phase implements ONLY location validation. No file scanning, no inventory population, just path existence checking and resource folder detection.

**Test-Driven:** Tests written first, implementation follows.

**Clear Boundaries:** 
- **Input:** `QList<pathDiscovery::PathElement>` from ResourcePaths
- **Output:** `QList<resourceDiscovery::ResourceLocation>` for valid, populated paths

---

## Scope: What Phase 2A Does

✅ **In Scope:**

- Take 12 qualified paths from pathDiscovery::ResourcePaths
- Check each path exists on filesystem
- Check each path is readable
- Detect if path contains any resource folders (templates/, libraries/, fonts/, etc.)
- Create ResourceLocation for paths that pass all checks
- Return list of validated locations with metadata (tier, displayName, isWritable)

❌ **Out of Scope:**

- Resource file scanning (Phase 2B)
- File content parsing (Phase 2B)
- Inventory population (Phase 2B)
- GUI integration (Phase 5)
- Manual enable/disable of locations (Phase 5)
- LocationScanner per-type scanning (Phase 2B)

---

## Namespace: resourceDiscovery

**New Folder:** `src/resourceDiscovery/`

**Purpose:** Bridge between path generation and file scanning. Validates candidate paths to produce confirmed locations containing resources.

**Responsibility:** Filesystem validation only - no knowledge of file contents.

**Dependencies:**
- ✅ `platformInfo` (for QDir, QFileInfo operations)
- ✅ `resourceMetadata` (for ResourceTier enum, ResourceType definitions)
- ✅ `pathDiscovery` (for PathElement input)
- ❌ Does NOT depend on `resourceScanning` or `resourceInventory`

---

## Input/Output Specification

### Input: PathElement from pathDiscovery

```cpp
namespace pathDiscovery {
    struct PathElement {
        resourceMetadata::ResourceTier tier;
        QString path;
        
        PathElement(resourceMetadata::ResourceTier t, const QString& p)
            : tier(t), path(p) {}
    };
}
```

**Real Example (from test_path_discovery output):**

```cpp
QList<pathDiscovery::PathElement> discoveryPaths = {
    // Installation tier (4 paths)
    {Installation, "C:/Program Files/ScadTemplates"},
    {Installation, "D:/repositories/cppsnippets/cppsnippets/build/bin/Debug"},  // exe location
    {Installation, "D:/repositories/cppsnippets/cppsnippets/build/bin/share/ScadTemplates"},
    {Installation, "D:/repositories/cppsnippets/cppsnippets/build/bin"},
    
    // Machine tier (1 path)
    {Machine, "C:/ProgramData/ScadTemplates"},
    
    // User tier (7 paths)
    {User, "C:/Users/Jeff/AppData/Roaming/ScadTemplates"},
    {User, "C:/Users/Jeff/AppData/Local/ScadTemplates"},
    {User, "D:/repositories/cppsnippets/cppsnippets/build/bin/ScadTemplates"},
    {User, "C:/Users/Jeff/Documents/ScadTemplates"},  // QStandardPaths
    {User, "C:/CustomScad"},                          // User-designated
    {User, "C:/Users/Jeff/Documents/MyTemplates"},    // User-designated
    {User, "D:/ProjectResources/ScadLibs"}            // User-designated
};
```

**Characteristics:**
- All paths are absolute
- All paths have been deduplicated
- Environment variables already expanded
- Folder name rules already applied
- Each path may or may not exist on filesystem
- Each path may or may not contain resource folders

### Output: ResourceLocation from resourceDiscovery

```cpp
namespace resourceDiscovery {
    /**
     * @brief Represents a validated location containing resources
     * 
     * Only created for paths that:
     * 1. Exist on filesystem
     * 2. Are readable
     * 3. Contain at least one resource folder
     */
    struct ResourceLocation {
        QString path;                          // Absolute filesystem path
        resourceMetadata::ResourceTier tier;   // From PathElement
        QString displayName;                   // Human-readable (e.g., "User - Documents")
        bool isWritable;                       // Can user write to this location?
        QStringList resourceFolders;           // Which folders found (templates/, fonts/, etc.)
        
        // Constructor for convenience
        ResourceLocation(const QString& p, 
                        resourceMetadata::ResourceTier t,
                        const QString& dn, 
                        bool w,
                        const QStringList& rf)
            : path(p), tier(t), displayName(dn), isWritable(w), resourceFolders(rf) {}
        
        // Default constructor for Qt containers
        ResourceLocation() 
            : tier(resourceMetadata::ResourceTier::Unknown), isWritable(false) {}
        
        // Comparison for testing
        bool operator==(const ResourceLocation& other) const {
            return path == other.path && tier == other.tier;
        }
    };
}
```

**Example Output:**

Assuming testFileStructure exists at appropriate locations:

```cpp
QList<resourceDiscovery::ResourceLocation> validatedLocations = {
    {
        "D:/repositories/cppsnippets/cppsnippets/testFileStructure",
        resourceMetadata::ResourceTier::Installation,
        "Installation - Build Area",
        true,  // writable
        {"templates", "libraries", "examples", "fonts", "color-schemes"}
    },
    {
        "C:/Users/Jeff/Documents/ScadTemplates",
        resourceMetadata::ResourceTier::User,
        "User - Documents",
        true,
        {"templates"}
    }
    // ... other validated locations
};
```

---

## Display Name Generation Rules

Display names provide compact, human-readable identification for UI elements.

### Installation Tier

**Goal:** Show app name or location identifier

**Rules:**
1. If path contains exe directory → "Installation - [BuildType]" (e.g., "Installation - Debug")
2. If path ends with app name → "Installation - [AppName]"
3. If path in Program Files → "Installation - Program Files"
4. Otherwise → "Installation - [LastComponent]"

**Examples:**
- `D:/repos/cppsnippets/build/bin/Debug` → "Installation - Debug"
- `C:/Program Files/ScadTemplates` → "Installation - Program Files"

### Machine Tier

**Goal:** Show system location

**Rules:**
1. Extract key path component (ProgramData, etc.)
2. Format as "Machine - [Component]"

**Examples:**
- `C:/ProgramData/ScadTemplates` → "Machine - ProgramData"

### User Tier

**Goal:** Show personal location context

**Rules:**
1. If Documents folder → "User - Documents"
2. If AppData/Roaming → "User - AppData Roaming"
3. If AppData/Local → "User - AppData Local"
4. If user-designated path → "User - [LastSignificantComponent]"
5. Otherwise → "User - [LastComponent]"

**Examples:**
- `C:/Users/Jeff/Documents/ScadTemplates` → "User - Documents"
- `C:/Users/Jeff/AppData/Roaming/ScadTemplates` → "User - AppData Roaming"
- `C:/CustomScad` → "User - CustomScad"
- `D:/ProjectResources/ScadLibs` → "User - ScadLibs"

---

## Detailed Design

### File Structure

```
src/
├── resourceDiscovery/              (NEW FOLDER)
│   ├── ResourceLocation.hpp        (struct definition)
│   ├── DiscoveryEngine.hpp         (class declaration)
│   ├── DiscoveryEngine.cpp         (implementation)
│   └── export.hpp                  (RESOURCEDISCOVERY_API macro)
│
tests/
└── resourceDiscovery/              (NEW FOLDER)
    └── test_discovery_engine.cpp   (Google Test suite)
```

### DiscoveryEngine Class

**File:** `src/resourceDiscovery/DiscoveryEngine.hpp`

```cpp
#ifndef DISCOVERYENGINE_HPP
#define DISCOVERYENGINE_HPP

#include <QString>
#include <QStringList>
#include <QList>
#include "ResourceLocation.hpp"
#include "pathDiscovery/PathElement.hpp"

namespace resourceDiscovery {

/**
 * @brief Validates paths and creates ResourceLocation objects
 * 
 * Takes qualified search paths from pathDiscovery::ResourcePaths and filters
 * to only those that exist, are readable, and contain resource folders.
 */
class DiscoveryEngine {
public:
    /**
     * @brief Scan discovery paths and return validated locations
     * 
     * @param discoveryPaths Qualified paths from ResourcePaths::qualifiedSearchPaths()
     * @return List of ResourceLocation objects for valid, populated paths
     * 
     * Validates each path:
     * - Skips empty paths (shouldn't happen with current pathDiscovery)
     * - Checks path exists on filesystem
     * - Checks path is readable
     * - Detects which resource folders are present
     * - Creates ResourceLocation only if at least one resource folder found
     * 
     * Logs info for debugging (normal qDebug, not qWarning for non-existent paths)
     */
    static QList<ResourceLocation> discoverLocations(
        const QList<pathDiscovery::PathElement>& discoveryPaths);
    
private:
    /**
     * @brief Check if path exists and is accessible
     * @param path Absolute filesystem path
     * @return true if path exists and is readable
     */
    static bool isAccessible(const QString& path);
    
    /**
     * @brief Check if path is writable
     * @param path Absolute filesystem path
     * @return true if user can write to path
     */
    static bool isWritable(const QString& path);
    
    /**
     * @brief Detect which resource folders exist in path
     * 
     * Checks for standard resource folder names:
     * - templates/
     * - libraries/
     * - fonts/
     * - color-schemes/
     * - examples/
     * - tests/
     * - locale/
     * - shaders/
     * 
     * @param path Absolute filesystem path
     * @return List of folder names found (empty if none)
     */
    static QStringList detectResourceFolders(const QString& path);
    
    /**
     * @brief Generate display name from path and tier
     * 
     * Uses tier-specific rules to create compact, meaningful names.
     * 
     * @param path Absolute filesystem path
     * @param tier Resource tier
     * @return Human-readable display name
     */
    static QString generateDisplayName(const QString& path, 
                                       resourceMetadata::ResourceTier tier);
    
    /**
     * @brief Get list of expected resource folder names
     * @return List of folder names to check for
     */
    static QStringList expectedResourceFolders();
    
    /**
     * @brief Extract significant path component for display
     * 
     * Helper for generateDisplayName - extracts context-appropriate
     * path component (Documents, AppData, etc.)
     * 
     * @param path Full filesystem path
     * @return Significant component or last directory name
     */
    static QString extractSignificantComponent(const QString& path);
};

} // namespace resourceDiscovery

#endif // DISCOVERYENGINE_HPP
```

---

## Implementation Details

### Resource Folder Detection

```cpp
QStringList DiscoveryEngine::expectedResourceFolders()
{
    return {
        QStringLiteral("templates"),
        QStringLiteral("libraries"),
        QStringLiteral("fonts"),
        QStringLiteral("color-schemes"),
        QStringLiteral("examples"),
        QStringLiteral("tests"),
        QStringLiteral("locale"),
        QStringLiteral("shaders")
    };
}

QStringList DiscoveryEngine::detectResourceFolders(const QString& path)
{
    QStringList found;
    QDir dir(path);
    
    if (!dir.exists()) {
        return found;  // Empty list
    }
    
    const QStringList expected = expectedResourceFolders();
    for (const QString& folderName : expected) {
        QString subPath = dir.filePath(folderName);
        QFileInfo info(subPath);
        if (info.exists() && info.isDir()) {
            found.append(folderName);
        }
    }
    
    return found;
}
```

### Display Name Generation

```cpp
QString DiscoveryEngine::generateDisplayName(
    const QString& path, 
    resourceMetadata::ResourceTier tier)
{
    QString tierName;
    
    // Get tier prefix
    switch (tier) {
        case resourceMetadata::ResourceTier::Installation:
            tierName = QStringLiteral("Installation");
            break;
        case resourceMetadata::ResourceTier::Machine:
            tierName = QStringLiteral("Machine");
            break;
        case resourceMetadata::ResourceTier::User:
            tierName = QStringLiteral("User");
            break;
        default:
            tierName = QStringLiteral("Unknown");
            break;
    }
    
    // Extract significant component
    QString component = extractSignificantComponent(path);
    
    return QString("%1 - %2").arg(tierName, component);
}

QString DiscoveryEngine::extractSignificantComponent(const QString& path)
{
    // Check for Documents
    if (path.contains(QStringLiteral("Documents"), Qt::CaseInsensitive)) {
        return QStringLiteral("Documents");
    }
    
    // Check for AppData variants
    if (path.contains(QStringLiteral("AppData/Roaming"), Qt::CaseInsensitive) ||
        path.contains(QStringLiteral("AppData\\Roaming"), Qt::CaseInsensitive)) {
        return QStringLiteral("AppData Roaming");
    }
    if (path.contains(QStringLiteral("AppData/Local"), Qt::CaseInsensitive) ||
        path.contains(QStringLiteral("AppData\\Local"), Qt::CaseInsensitive)) {
        return QStringLiteral("AppData Local");
    }
    
    // Check for ProgramData
    if (path.contains(QStringLiteral("ProgramData"), Qt::CaseInsensitive)) {
        return QStringLiteral("ProgramData");
    }
    
    // Check for Program Files
    if (path.contains(QStringLiteral("Program Files"), Qt::CaseInsensitive)) {
        return QStringLiteral("Program Files");
    }
    
    // Check for Debug/Release build directories
    QDir dir(path);
    QString dirName = dir.dirName();
    if (dirName.compare(QStringLiteral("Debug"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("Build Area");
    }
    if (dirName.compare(QStringLiteral("Release"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("Release Build");
    }
    
    // Default: use last directory component
    if (!dirName.isEmpty()) {
        return dirName;
    }
    
    // Last resort: use full path
    return path;
}
```

### Main Discovery Method

```cpp
QList<ResourceLocation> DiscoveryEngine::discoverLocations(
    const QList<pathDiscovery::PathElement>& discoveryPaths)
{
    QList<ResourceLocation> validatedLocations;
    
    qDebug() << "DiscoveryEngine: Processing" << discoveryPaths.size() << "candidate paths";
    
    for (const auto& pathElement : discoveryPaths) {
        const QString& path = pathElement.path;
        const resourceMetadata::ResourceTier tier = pathElement.tier;
        
        // Skip empty paths (shouldn't happen)
        if (path.isEmpty()) {
            qDebug() << "DiscoveryEngine: Skipping empty path";
            continue;
        }
        
        // Check if path exists
        if (!isAccessible(path)) {
            qDebug() << "DiscoveryEngine: Path does not exist:" << path;
            continue;
        }
        
        // Detect resource folders
        QStringList resourceFolders = detectResourceFolders(path);
        
        if (resourceFolders.isEmpty()) {
            qDebug() << "DiscoveryEngine: Path exists but contains no resource folders:" << path;
            continue;
        }
        
        // Check write access
        bool writable = isWritable(path);
        
        // Generate display name
        QString displayName = generateDisplayName(path, tier);
        
        // Create and add ResourceLocation
        validatedLocations.append(
            ResourceLocation(path, tier, displayName, writable, resourceFolders)
        );
        
        qDebug() << "DiscoveryEngine: Added location:"
                 << displayName
                 << "- folders:" << resourceFolders
                 << "(" << (writable ? "writable" : "read-only") << ")";
    }
    
    qDebug() << "DiscoveryEngine: Found" << validatedLocations.size()
             << "validated locations";
    
    return validatedLocations;
}
```

---

## Testing Strategy

### Test File Structure

**File:** `tests/resourceDiscovery/test_discovery_engine.cpp`

Uses Google Test with QTemporaryDir for filesystem simulation.

### Test Categories

#### 1. Basic Functionality (5 tests)
- Empty input returns empty output
- Empty path is skipped
- Non-existent path is skipped
- Path with no resource folders is skipped
- Path with random folders (not resource folders) is skipped

#### 2. Resource Folder Detection (8 tests)
- Path with templates/ folder is included
- Path with libraries/ folder is included
- Path with fonts/ folder is included
- Path with color-schemes/ folder is included
- Path with examples/ folder is included
- Path with multiple resource folders is included
- ResourceLocation.resourceFolders contains correct names
- Case sensitivity check (Windows: case-insensitive, Linux: case-sensitive)

#### 3. Multiple Paths (4 tests)
- Multiple valid paths all included
- Mixed valid and invalid paths (only valid included)
- All Installation tier paths processed correctly
- Mixed tiers processed with correct tier tags

#### 4. Write Access Detection (3 tests)
- Writable path marked as writable
- Read-only path marked as read-only (platform-specific)
- Temp directory is writable

#### 5. Display Name Generation (6 tests)
- Installation tier shows correct format
- Machine tier shows correct format
- User tier - Documents shows "User - Documents"
- User tier - AppData Roaming shows "User - AppData Roaming"
- User tier - AppData Local shows "User - AppData Local"
- User tier - custom path shows last component

#### 6. Edge Cases (4 tests)
- Path with only files (no folders) is skipped
- Resource folder as file (not directory) is ignored
- Path with subdirectories named similarly but not matching is skipped
- Symlinks handled correctly (if OS supports)

**Total Estimated Tests:** ~30 tests

---

## Integration Example

### Using DiscoveryEngine with pathDiscovery

```cpp
#include "pathDiscovery/ResourcePaths.hpp"
#include "resourceDiscovery/DiscoveryEngine.hpp"

void performResourceDiscovery() {
    // Phase 1: Generate candidate paths
    pathDiscovery::ResourcePaths pathGenerator;
    QList<pathDiscovery::PathElement> candidatePaths = 
        pathGenerator.qualifiedSearchPaths();
    
    qDebug() << "Phase 1: Generated" << candidatePaths.size() << "candidate paths";
    
    // Phase 2A: Validate paths and detect resource folders
    QList<resourceDiscovery::ResourceLocation> validatedLocations = 
        resourceDiscovery::DiscoveryEngine::discoverLocations(candidatePaths);
    
    qDebug() << "Phase 2A: Found" << validatedLocations.size() << "valid resource locations";
    
    // Display results
    for (const auto& location : validatedLocations) {
        qDebug() << "Location:" << location.displayName;
        qDebug() << "  Path:" << location.path;
        qDebug() << "  Tier:" << (int)location.tier;
        qDebug() << "  Writable:" << location.isWritable;
        qDebug() << "  Resource folders:" << location.resourceFolders;
    }
    
    // Future: Phase 2B will use validatedLocations for file scanning
}
```

---

## Build Integration

### CMakeLists.txt

```cmake
# Create resourceDiscovery library
add_library(resourceDiscovery SHARED
    src/resourceDiscovery/DiscoveryEngine.cpp
    src/resourceDiscovery/DiscoveryEngine.hpp
    src/resourceDiscovery/ResourceLocation.hpp
    src/resourceDiscovery/export.hpp
)

target_include_directories(resourceDiscovery PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(resourceDiscovery PUBLIC
    Qt6::Core
    platformInfo
    resourceMetadata
    pathDiscovery
)

# Tests
add_executable(test_discovery_engine
    tests/resourceDiscovery/test_discovery_engine.cpp
)

target_link_libraries(test_discovery_engine PRIVATE
    resourceDiscovery
    pathDiscovery
    resourceMetadata
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
)

add_test(NAME DiscoveryEngineTests COMMAND test_discovery_engine)
```

---

## Success Criteria

Phase 2A is complete when:

1. ✅ All 30 tests pass
2. ✅ Code compiles without warnings (MSVC /W4, Clang -Wall)
3. ✅ Integration with pathDiscovery confirmed
4. ✅ Real filesystem testing shows expected behavior
5. ✅ Display names are human-readable and meaningful
6. ✅ Documentation complete (this document + code comments)
7. ✅ Code reviewed and committed to repository

---

## Estimated Effort

- **File Creation:** 1 hour
  - DiscoveryEngine.hpp: 0.5 hours
  - DiscoveryEngine.cpp: 0.5 hours
  
- **Implementation:** 4 hours
  - detectResourceFolders: 1 hour
  - generateDisplayName: 1 hour
  - discoverLocations: 1 hour
  - Helper methods: 1 hour
  
- **Testing:** 4 hours
  - Test file creation: 2 hours
  - Test execution & debug: 1.5 hours
  - Manual testing with real paths: 0.5 hours
  
- **Build Integration:** 1 hour
  - CMakeLists.txt updates
  - Export headers
  - Dependency verification

- **Documentation:** 1 hour
  - Update this document with results
  - Code comments
  - README updates

**Total:** ~11 hours for Phase 2A

---

## Next Steps After Phase 2A

### Immediate: Create test_discovery_integration.cpp

Small program similar to test_path_discovery that:
1. Calls pathDiscovery to get candidate paths
2. Calls resourceDiscovery to validate
3. Prints side-by-side comparison showing which paths were kept/rejected and why

### Phase 2B: ResourceScanning Implementation

Break into sub-phases:

1. **2B.1: Template Scanning**
   - Read .json files from templates/ folders
   - Validate JSON structure
   - Populate TemplateStore

2. **2B.2: Font and Color Scheme Scanning**
   - Scan fonts/ for .ttf, .otf files
   - Scan color-schemes/ for theme files

3. **2B.3: Example Scanning with Groups**
   - Handle nested directory structure
   - Category detection

4. **2B.4: Library and Test Scanning**
   - .scad file detection
   - Handle includes/dependencies

---

## Related Documents

- [2026-01-09 Namespace Reorganization Roadmap](2026-01-09-namespace-folder-reorganization-roadmap.md)
- [QDir Documentation](https://doc.qt.io/qt-6/qdir.html)
- [QFileInfo Documentation](https://doc.qt.io/qt-6/qfileinfo.html)
- [QStandardPaths Reference Table](stdpathstable.md)

---

## Summary

**Phase 2A Goal:** Bridge the gap between path generation (pathDiscovery) and file scanning (resourceScanning future).

**Input:** 12 qualified paths with tier tags  
**Process:** Filesystem validation + resource folder detection  
**Output:** Validated ResourceLocation objects ready for scanning

**Why This Matters:**
- Prevents scanning non-existent paths
- Identifies which resource types are available at each location
- Provides UI-friendly display names
- Maintains tier information for access control
- Clear separation of concerns (validation vs. scanning)

**Status:** Ready to implement. All prerequisites complete (namespace reorganization, path discovery system functional and tested).
