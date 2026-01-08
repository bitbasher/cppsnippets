# Phase 2: Discovery Pipeline Implementation Plan

**Date:** January 5, 2026  
**Status:** Planning  
**Author:** GitHub Copilot (Claude Sonnet 4.5)  
**Primary Reference:** `2026-01-05-resource-discovery-architecture.md`

---

## Overview

This document outlines the implementation plan for Phase 2 of the resource discovery system. Phase 1 (ResourcePaths with qualified search path generation) is complete. Phase 2 focuses on building the discovery pipeline components that validate paths, enrich locations with metadata, and coordinate the scanning process.

**Phase 2 Goals:**
- Create DiscoveryScanner to validate filesystem paths
- Create LocationScanner to detect resource types at locations
- Refactor ResourceScanner to use `QDirListing` (Qt 6.8+)
- Create ResourceDiscoveryCoordinator to orchestrate the pipeline
- Establish testing strategy for the pipeline

**Documents Reviewed:**
1. ✅ `2026-01-05-resource-discovery-architecture.md` - Primary architecture (5-stage pipeline)
2. ✅ `Resource-Discovery-Specifications.md` - Folder structure rules, tier behavior
3. ✅ `ResourceRefactoring-QDirListing-Design.md` - QDirListing usage patterns
4. ✅ Current `resourceScanner.hpp/.cpp` - Existing implementation using `QDirIterator`

---

## Architecture Context

**5-Stage Pipeline:**
```
1. ResourcePaths::qualifiedSearchPaths()
   └─> QList<PathElement> (s_discoveryPaths)
   
2. DiscoveryScanner(s_discoveryPaths)        ← Phase 2A
   └─> QList<ResourceLocation> (validatedLocations)
   
3. LocationScanner(validatedLocations)        ← Phase 2B
   └─> QList<ResourceLocation> (enrichedLocations with metadata)
   
4. ResourceScanner(s) (enrichedLocations)     ← Phase 2C (refactor existing)
   └─> Resource objects per type
   
5. ResourceInventory                          ← Phase 2D (new storage)
   └─> Stores all discovered resources with metadata
```

**Phase 1 Status:** ✅ **COMPLETE**
- Environment variable expansion
- Folder name appending rules
- Sibling installation detection (LTS ↔ Nightly)
- User-designated paths from QSettings
- 28 unit tests passing

---

## Phase 2A: Create DiscoveryScanner

### Purpose
Validate which qualified search paths from ResourcePaths actually exist on the filesystem.

### File Location
- **Header:** `src/resourceMgmt/discoveryScanner.hpp`
- **Implementation:** `src/resourceMgmt/discoveryScanner.cpp`

### Class Design

```cpp
namespace resourceMgmt {

/**
 * @brief Validates filesystem paths and creates initial ResourceLocation objects
 * 
 * Takes qualified search paths from ResourcePaths and filters to only those
 * that exist and are accessible on the filesystem.
 */
class DiscoveryScanner {
public:
    /**
     * @brief Scan discovery paths and return valid locations
     * @param discoveryPaths Output from ResourcePaths::qualifiedSearchPaths()
     * @return List of ResourceLocation objects for paths that exist
     */
    static QList<ResourceLocation> scanDiscoveryPaths(
        const QList<PathElement>& discoveryPaths);
        
private:
    /**
     * @brief Check if path exists and is readable
     * @param path Absolute filesystem path
     * @return true if path exists and user has read access
     */
    static bool isAccessible(const QString& path);
    
    /**
     * @brief Generate display name from path and tier
     * @param path Absolute filesystem path
     * @param tier Resource tier this location belongs to
     * @return Human-readable display name
     */
    static QString generateDisplayName(const QString& path, ResourceTier tier);
    
    /**
     * @brief Remove duplicate paths from location list
     * @param locations List to deduplicate (modified in place)
     * 
     * Handles cases where same physical path appears from different sources
     * (e.g., environment variable and sibling discovery both point to same dir)
     */
    static void deduplicatePaths(QList<ResourceLocation>& locations);
};

} // namespace resourceMgmt
```

### Responsibilities

- ✅ Check `QFileInfo(path).exists()` for each PathElement
- ✅ Check `QFileInfo(path).isReadable()` for accessibility
- ✅ Check `QFileInfo(path).isWritable()` for write permissions
- ✅ Create `ResourceLocation` objects with:
  - `path` - absolute filesystem path
  - `tier` - from PathElement
  - `exists` - true (filtered to only existing)
  - `isWritable` - from filesystem check
  - `isEnabled` - default true
  - `displayName` - generated from path/tier
  - `hasResourceFolders` - false (not yet checked - LocationScanner does this)
- ✅ Deduplicate identical paths from different sources
- ✅ Preserve tier information from PathElement

### Open Questions

**Q1: Namespace for new classes?**
- Existing: `platformInfo` (ResourcePaths), `resInventory` (ResourceScanner)
- Proposal: `resourceMgmt` for DiscoveryScanner, LocationScanner, Coordinator
- Alternative: Move everything to unified namespace?
- **Decision:** [FILL IN]

---

## Phase 2B: Create LocationScanner

### Purpose
Enrich validated locations by detecting which resource types exist at each location.

### File Location
- **Header:** `src/resourceMgmt/locationScanner.hpp`
- **Implementation:** `src/resourceMgmt/locationScanner.cpp`

### Class Design

```cpp
namespace resourceMgmt {

/**
 * @brief Enriches ResourceLocation objects with resource type metadata
 * 
 * Scans each validated location to determine which resource subdirectories
 * exist (examples/, fonts/, color-schemes/, etc.)
 */
class LocationScanner {
public:
    /**
     * @brief Enrich locations with resource metadata
     * @param candidateLocations Validated locations from DiscoveryScanner
     * @return Same locations with hasResourceFolders and optionally detectedTypes updated
     */
    static QList<ResourceLocation> enrichLocations(
        const QList<ResourceLocation>& candidateLocations);
        
private:
    /**
     * @brief Check if location has ANY resource subdirectories
     * @param basePath Absolute path to location
     * @return true if ANY resource subfolder found
     */
    static bool hasAnyResourceFolders(const QString& basePath);
    
    /**
     * @brief Detect which specific resource types exist at location
     * @param basePath Absolute path to location
     * @return List of ResourceType values for detected subdirectories
     */
    static QList<ResourceType> detectResourceTypes(const QString& basePath);
    
    /**
     * @brief Check if specific resource subdirectory exists
     * @param basePath Absolute path to location
     * @param type Resource type to check for
     * @return true if subdirectory for this type exists
     */
    static bool hasResourceType(const QString& basePath, ResourceType type);
};

} // namespace resourceMgmt
```

### Responsibilities

- ✅ Scan each location for resource type subdirectories:
  - `examples/`, `fonts/`, `color-schemes/`, `templates/`, `tests/`, `libraries/`, `locale/`, `shaders/`
- ✅ Use `ResourceTypeInfo::subdirFor()` for expected subfolder names
- ✅ Update `ResourceLocation::hasResourceFolders` flag:
  - `true` if ANY resource subdirectory found
  - `false` if location has no recognizable resources
- ✅ Optionally store list of discovered types (see Q3 below)
- ✅ Preserve all other location metadata from DiscoveryScanner

### Subfolder Detection Strategy

Use existing `ResourceScanner::resourceSubfolder()` mapping:
```cpp
// From resourceScanner.cpp
ColorSchemes    -> "color-schemes"
RenderColors    -> "color-schemes/render"
EditorColors    -> "color-schemes/editor"
Font            -> "fonts"
Library         -> "libraries"
Example         -> "examples"
Test            -> "tests"
Template        -> "templates"
Shader          -> "shaders"
Translation     -> "locale"
```

Check for top-level folders first (examples, fonts, etc.), then check nested ones (color-schemes/render).

### Open Questions

**Q3: ResourceLocation enhancement - detected types storage?**
- **Option A:** Add `QList<ResourceType> detectedTypes` field to ResourceLocation
  - Pro: Avoids re-scanning when building inventory
  - Pro: Can show "This location has: Examples, Fonts, Templates" in UI
  - Con: Adds complexity to ResourceLocation
- **Option B:** Keep only boolean `hasResourceFolders` flag
  - Pro: Simpler, stays true to "enrichment" concept
  - Pro: ResourceScanner checks for subdirectories anyway
  - Con: Re-checks existence during scanning phase
- **Decision:** [FILL IN]

---

## Phase 2C: Refactor ResourceScanner to use QDirListing

### Purpose
Update existing ResourceScanner to use Qt 6.8+ `QDirListing` API instead of older `QDirIterator`.

### File Location
- **Header:** `src/resInventory/resourceScanner.hpp` (existing)
- **Implementation:** `src/resInventory/resourceScanner.cpp` (existing)

### Changes Required

#### 1. Replace QDirIterator with QDirListing

**Current Pattern (QDirIterator):**
```cpp
QDirIterator it(basePath, filters, QDir::Files, QDirIterator::Subdirectories);
while (it.hasNext()) {
    QString filePath = it.next();
    QFileInfo info(filePath);  // Constructs QFileInfo for metadata
    // ... process file
}
```

**New Pattern (QDirListing):**
```cpp
using F = QDirListing::IteratorFlag;
for (const auto& entry : QDirListing(basePath, F::Recursive | F::FilesOnly)) {
    QString filePath = entry.filePath();
    QString fileName = entry.fileName();
    QDateTime modified = entry.lastModified();  // No QFileInfo needed!
    qint64 size = entry.size();
    // ... process file
}
```

**Benefits:**
- ✅ More efficient - `DirEntry` provides metadata without constructing `QFileInfo`
- ✅ Cleaner syntax - C++20 range-based for loop
- ✅ Scales better for large directories (forward-only iteration)
- ✅ Built-in recursive traversal

#### 2. Scanning Patterns by Resource Type

**Recursive Scanning (Examples, Tests, Libraries):**
```cpp
QVector<ResourceItem> ResourceScanner::scanExamples(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey)
{
    QVector<ResourceItem> items;
    
    using F = QDirListing::IteratorFlag;
    const auto flags = F::Recursive | F::FilesOnly;
    
    for (const auto& entry : QDirListing(basePath, flags)) {
        // Check if .scad file
        if (!entry.fileName().endsWith(QLatin1String(".scad"))) {
            continue;
        }
        
        // Extract category from path depth
        QString category = extractCategory(basePath, entry.filePath());
        
        // Create ResourceScript with attachments
        ResourceScript item = scanScriptWithAttachments(
            entry.filePath(), 
            ResourceType::Example,
            tier, 
            locationKey
        );
        item.setCategory(category);
        
        items.append(item);
    }
    
    return items;
}
```

**Flat Scanning (Templates, Fonts, Colors):**
```cpp
QVector<ResourceItem> ResourceScanner::scanTemplates(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey)
{
    QVector<ResourceItem> items;
    
    using F = QDirListing::IteratorFlag;
    const auto flags = F::FilesOnly;  // NO Recursive flag!
    
    for (const auto& entry : QDirListing(basePath, flags)) {
        // Templates are .json only (per Resource-Discovery-Specifications.md)
        if (!entry.fileName().endsWith(QLatin1String(".json"))) {
            continue;
        }
        
        ResourceTemplate item;
        item.setPath(entry.filePath());
        item.setName(entry.fileName());
        item.setTier(tier);
        item.setLocationKey(locationKey);
        item.setType(ResourceType::Template);
        
        // TODO: Parse JSON for template metadata
        
        items.append(item);
    }
    
    return items;
}
```

#### 3. Keep Type-Specific Logic

Existing type-specific methods are good:
- ✅ `scanColorSchemes()` - handle nested editor/render folders
- ✅ `scanRenderColors()` - parse JSON for color scheme data
- ✅ `scanEditorColors()` - different JSON structure than render
- ✅ `scanFonts()` - `.ttf`, `.otf`, `.woff`, `.woff2` extensions
- ✅ `scanExamples()` - handle Groups (categories), find attachments
- ✅ `scanTests()` - similar to examples, may have templates/ subfolder
- ✅ `scanTemplates()` - flat .json only, NO .scad files
- ✅ `scanTranslations()` - `.qm`, `.ts` files

Just update the iteration mechanism to use `QDirListing`.

#### 4. Decouple from ResourceTreeWidget

**Current Issue:** Methods like `scanToTree()` and `scanAllTiers()` populate widgets directly.

**Refactor Strategy:**
- ✅ Keep methods that return `QVector<ResourceItem>` (already exist)
- ✅ Widget population becomes separate concern (UI layer)
- ✅ Consider deprecating `scanToTree()` methods or make them wrappers

**Example:**
```cpp
// In UI code (not scanner):
void populateTreeFromScanResults(
    ResourceTreeWidget* tree,
    const QVector<ResourceItem>& items)
{
    tree->clear();
    for (const auto& item : items) {
        tree->addItem(item);
    }
}
```

---

## Phase 2D: Create ResourceDiscoveryCoordinator

### Purpose
Orchestrate the full discovery pipeline from path generation through inventory population.

### File Location
- **Header:** `src/resourceMgmt/resourceDiscoveryCoordinator.hpp`
- **Implementation:** `src/resourceMgmt/resourceDiscoveryCoordinator.cpp`

### Class Design

```cpp
namespace resourceMgmt {

/**
 * @brief Coordinates the full resource discovery pipeline
 * 
 * Orchestrates all stages from path generation through inventory building.
 * Caches intermediate results for inspection and debugging.
 */
class ResourceDiscoveryCoordinator : public QObject {
    Q_OBJECT
    
public:
    explicit ResourceDiscoveryCoordinator(QObject* parent = nullptr);
    
    /**
     * @brief Run full discovery pipeline
     * @param folderSuffix Suffix for folder names (e.g., " (Nightly)")
     * @param isNightly Whether this is a nightly build
     */
    void runFullDiscovery(const QString& folderSuffix = QString(), 
                          bool isNightly = false);
    
    /**
     * @brief Access discovered locations
     * @return List of enriched locations (after Stage 3)
     */
    const QList<ResourceLocation>& discoveredLocations() const;
    
    /**
     * @brief Access resource inventory
     * @return Reference to inventory (after Stage 5)
     */
    const ResourceInventory& inventory() const;
    
    /**
     * @brief Re-run location discovery (Stages 1-3)
     */
    void refreshLocations();
    
    /**
     * @brief Rescan specific resource type (Stage 4)
     * @param type Resource type to rescan
     */
    void rescanType(ResourceType type);
    
    /**
     * @brief Clear all cached data
     */
    void clear();
    
    /**
     * @brief Get summary of discovery results
     * @return Human-readable summary (e.g., "3 locations, 156 resources")
     */
    QString discoverySummary() const;
    
signals:
    void discoveryStarted();
    void pathsGenerated(int count);
    void locationsValidated(int count);
    void locationsEnriched(int count);
    void typeScanned(ResourceType type, int itemCount);
    void discoveryComplete();
    void discoveryError(const QString& message);
    
private:
    // Pipeline stages (cached for inspection/debugging)
    QList<PathElement> m_discoveryPaths;        // Stage 1 output
    QList<ResourceLocation> m_validLocations;   // Stage 2 output
    QList<ResourceLocation> m_enrichedLocs;     // Stage 3 output
    ResourceInventory* m_inventory;             // Stage 5 storage
    
    // Workers
    ResourceScanner* m_scanner;
    
    // Configuration
    QString m_folderSuffix;
    bool m_isNightly;
};

} // namespace resourceMgmt
```

### Orchestration Flow

```cpp
void ResourceDiscoveryCoordinator::runFullDiscovery(
    const QString& folderSuffix,
    bool isNightly)
{
    clear();
    emit discoveryStarted();
    
    try {
        // Stage 1: Generate qualified paths
        platformInfo::ResourcePaths paths;
        paths.setSuffix(folderSuffix);
        m_discoveryPaths = paths.qualifiedSearchPaths();
        emit pathsGenerated(m_discoveryPaths.size());
        
        // Stage 2: Validate existence
        m_validLocations = DiscoveryScanner::scanDiscoveryPaths(m_discoveryPaths);
        emit locationsValidated(m_validLocations.size());
        
        // Stage 3: Enrich with metadata
        m_enrichedLocs = LocationScanner::enrichLocations(m_validLocations);
        emit locationsEnriched(m_enrichedLocs.size());
        
        // Stage 4 & 5: Scan each resource type and populate inventory
        const QList<ResourceType> typesToScan = {
            ResourceType::ColorSchemes,
            ResourceType::RenderColors,
            ResourceType::EditorColors,
            ResourceType::Font,
            ResourceType::Example,
            ResourceType::Test,
            ResourceType::Template,
            ResourceType::Translation,
            ResourceType::Shader
        };
        
        for (ResourceType type : typesToScan) {
            QVector<ResourceItem> items = m_scanner->scanLocationsForType(
                m_enrichedLocs, type
            );
            m_inventory->addResources(items);
            emit typeScanned(type, items.size());
        }
        
        emit discoveryComplete();
        
    } catch (const std::exception& e) {
        emit discoveryError(QString::fromStdString(e.what()));
    }
}
```

### Open Questions

**Q2: ResourceInventory structure?**
- **Option A:** Single `QVector<ResourceItem*>` with multiple indexes
  ```cpp
  class ResourceInventory {
      QVector<ResourceItem*> m_allResources;
      QMultiMap<ResourceType, ResourceItem*> m_byType;
      QMultiMap<ResourceTier, ResourceItem*> m_byTier;
      QMultiMap<QString, ResourceItem*> m_byLocation;
  };
  ```
  - Pro: Single source of truth, fast lookups
  - Con: More complex memory management
  
- **Option B:** Separate typed vectors (from QDirListing design doc)
  ```cpp
  class ResourceInventory {
      QVector<ResourceItem> m_colorSchemes;
      QVector<ResourceItem> m_fonts;
      QVector<ResourceScript> m_examples;
      QVector<ResourceScript> m_tests;
      QVector<ResourceTemplate> m_templates;
  };
  ```
  - Pro: Type-safe, matches resource polymorphism
  - Pro: Easier iteration over specific types
  - Con: More storage, harder cross-type queries
  
- **Decision:** [FILL IN]

**Q5: Implementation order - what to tackle first?**
- **Option A:** Sequential (2A → 2B → 2C → 2D)
  - Pro: Logical progression, test each stage
  - Con: Can't see full picture until end
  
- **Option B:** Coordinator first (2D skeleton), then fill in components
  - Pro: See full data flow early
  - Pro: Can mock missing components
  - Con: More scaffolding code
  
- **Option C:** ResourceScanner refactor first (2C), then build around it
  - Pro: Validate QDirListing patterns early
  - Pro: Existing code to reference
  - Con: Working without pipeline context
  
- **Decision:** [FILL IN]

---

## Testing Strategy

### Unit Tests for Each Component

#### DiscoveryScanner Tests
```cpp
TEST(DiscoveryScanner, FiltersNonExistentPaths) {
    QList<PathElement> input = {
        {ResourceTier::Installation, "/does/not/exist"},
        {ResourceTier::User, QDir::tempPath()}  // Does exist
    };
    
    auto result = DiscoveryScanner::scanDiscoveryPaths(input);
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].path, QDir::tempPath());
}

TEST(DiscoveryScanner, DeduplicatesPaths) {
    QTemporaryDir tempDir;
    QString path = tempDir.path();
    
    QList<PathElement> input = {
        {ResourceTier::Installation, path},
        {ResourceTier::User, path}  // Same path, different tier
    };
    
    auto result = DiscoveryScanner::scanDiscoveryPaths(input);
    
    EXPECT_EQ(result.size(), 1);
    // First occurrence wins for tier
}
```

#### LocationScanner Tests
```cpp
TEST(LocationScanner, DetectsResourceFolders) {
    QTemporaryDir tempDir;
    QString basePath = tempDir.path();
    
    // Create examples/ subdirectory
    QDir(basePath).mkdir("examples");
    
    ResourceLocation loc;
    loc.path = basePath;
    loc.exists = true;
    loc.hasResourceFolders = false;
    
    auto result = LocationScanner::enrichLocations({loc});
    
    EXPECT_TRUE(result[0].hasResourceFolders);
}

TEST(LocationScanner, IgnoresNonResourceFolders) {
    QTemporaryDir tempDir;
    QString basePath = tempDir.path();
    
    // Create unrelated subdirectory
    QDir(basePath).mkdir("random_folder");
    
    ResourceLocation loc;
    loc.path = basePath;
    loc.exists = true;
    loc.hasResourceFolders = false;
    
    auto result = LocationScanner::enrichLocations({loc});
    
    EXPECT_FALSE(result[0].hasResourceFolders);
}
```

#### ResourceScanner Tests (with QDirListing)
```cpp
TEST(ResourceScanner, QDirListingScansRecursively) {
    QTemporaryDir tempDir;
    QString basePath = tempDir.path();
    
    // Create nested structure
    QDir(basePath).mkpath("examples/Basics");
    QFile file(basePath + "/examples/Basics/test.scad");
    file.open(QIODevice::WriteOnly);
    file.write("// test");
    file.close();
    
    ResourceScanner scanner;
    auto items = scanner.scanExamples(basePath, ResourceTier::Installation, "test");
    
    EXPECT_EQ(items.size(), 1);
    EXPECT_TRUE(items[0].path().contains("test.scad"));
}

TEST(ResourceScanner, TemplatesScanFlat) {
    QTemporaryDir tempDir;
    QString basePath = tempDir.path();
    
    // Create templates/ with nested folder (should be ignored)
    QDir(basePath).mkpath("templates/subfolder");
    QFile file1(basePath + "/templates/template1.json");
    file1.open(QIODevice::WriteOnly);
    file1.write("{}");
    file1.close();
    
    QFile file2(basePath + "/templates/subfolder/template2.json");
    file2.open(QIODevice::WriteOnly);
    file2.write("{}");
    file2.close();
    
    ResourceScanner scanner;
    auto items = scanner.scanTemplates(basePath, ResourceTier::User, "test");
    
    // Only template1.json should be found (flat scan only)
    EXPECT_EQ(items.size(), 1);
}
```

#### Integration Tests
```cpp
TEST(ResourceDiscoveryCoordinator, FullPipeline) {
    // Create temp directory structure
    QTemporaryDir tempDir;
    QString basePath = tempDir.path();
    QDir(basePath).mkpath("examples");
    QDir(basePath).mkpath("fonts");
    
    // Create sample resources
    QFile scad(basePath + "/examples/test.scad");
    scad.open(QIODevice::WriteOnly);
    scad.write("cube(10);");
    scad.close();
    
    // Set up paths in environment
    qputenv("OPENSCADPATH", basePath.toUtf8());
    
    // Run discovery
    ResourceDiscoveryCoordinator coordinator;
    coordinator.runFullDiscovery();
    
    // Verify results
    EXPECT_GT(coordinator.discoveredLocations().size(), 0);
    EXPECT_GT(coordinator.inventory().itemCount(ResourceType::Example), 0);
}
```

### Test Fixtures

Create reusable test fixtures with realistic directory structures:

```cpp
class ResourceTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        m_tempDir = new QTemporaryDir();
        createResourceStructure();
    }
    
    void TearDown() override {
        delete m_tempDir;
    }
    
    void createResourceStructure() {
        QString base = m_tempDir->path();
        
        // Create standard OpenSCAD structure
        QDir(base).mkpath("examples/Basics");
        QDir(base).mkpath("examples/Advanced");
        QDir(base).mkpath("fonts");
        QDir(base).mkpath("templates");
        QDir(base).mkpath("color-schemes/editor");
        QDir(base).mkpath("color-schemes/render");
        
        // Add sample files
        createFile(base + "/examples/Basics/sphere.scad", "sphere(10);");
        createFile(base + "/examples/Basics/sphere.png", "");
        createFile(base + "/templates/basic.json", "{}");
        createFile(base + "/color-schemes/editor/dark.json", "{}");
    }
    
    void createFile(const QString& path, const QString& content) {
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write(content.toUtf8());
        file.close();
    }
    
    QTemporaryDir* m_tempDir;
};
```

### Open Questions

**Q4: Testing approach preference?**
- **Option A:** Mock filesystem with `QTemporaryDir`
  - Pro: Fast, isolated, no dependencies
  - Pro: Qt provides good temporary directory support
  - Con: May not catch real filesystem edge cases
  
- **Option B:** Use test fixtures with real directory structures
  - Pro: Tests against actual filesystem behavior
  - Pro: Can commit test fixtures to repo
  - Con: Platform-dependent paths, slower tests
  
- **Option C:** Hybrid - unit tests with QTemporaryDir, integration tests with fixtures
  - Pro: Best of both worlds
  - Con: More test code to maintain
  
- **Decision:** [FILL IN]

---

## Implementation Details

### QDirListing API Reference

**Basic Usage:**
```cpp
#include <QDirListing>

using F = QDirListing::IteratorFlag;
```

**Flags:**
```cpp
F::FilesOnly      // Only regular files
F::DirsOnly       // Only directories
F::Recursive      // Recurse into subdirectories
F::FollowSymlinks // Follow symbolic links (default: don't follow)
```

**DirEntry Methods:**
```cpp
entry.fileName()        // QString - filename only
entry.filePath()        // QString - absolute path
entry.lastModified()    // QDateTime
entry.size()            // qint64 - bytes
entry.isDir()           // bool
entry.isFile()          // bool
entry.isSymLink()       // bool
```

**Performance Benefits:**
- No `QFileInfo` construction per entry
- Forward-only iteration (no random access)
- Efficient for large directories
- Lazy evaluation (yields entries as found)

### Resource Type to Subfolder Mapping

From `Resource-Discovery-Specifications.md`:

| ResourceType | Subfolder | Extensions | Scan Type |
|--------------|-----------|------------|-----------|
| ColorSchemes | `color-schemes/` | - | Container only |
| RenderColors | `color-schemes/render/` | `.json` | Flat |
| EditorColors | `color-schemes/editor/` | `.json` | Flat |
| Font | `fonts/` | `.ttf`, `.otf`, `.woff`, `.woff2` | Flat |
| Example | `examples/` | `.scad` + attachments | Recursive (1 level groups) |
| Test | `tests/` | `.scad` + attachments | Flat (optional templates/) |
| Template | `templates/` | `.json` only | Flat (NO .scad!) |
| Library | `libraries/` | `.scad` | Recursive (nested) |
| Translation | `locale/` | `.qm`, `.ts` | Flat |
| Shader | `shaders/` | `.vert`, `.frag`, `.glsl` | Flat |

**Attachment Extensions (Examples/Tests/Groups):**
```cpp
.json, .txt, .dat, .png, .jpg, .jpeg, .svg, .gif, .csv, .stl, .off, .dxf
```

### Category Extraction for Examples

Examples can have ONE level of category subfolders (Groups):

```
examples/
├── basic.scad           ← Category: "" (empty)
├── cube.scad            ← Category: "" (empty)
├── Basics/              ← Category name
│   ├── example1.scad    ← Category: "Basics"
│   └── example2.scad    ← Category: "Basics"
└── Advanced/            ← Category name
    └── mesh.scad        ← Category: "Advanced"
```

**Extraction Algorithm:**
```cpp
QString extractCategory(const QString& basePath, const QString& filePath) {
    QString relativePath = filePath;
    relativePath.remove(0, basePath.length());
    if (relativePath.startsWith('/')) {
        relativePath.remove(0, 1);
    }
    
    int slashIndex = relativePath.indexOf('/');
    if (slashIndex == -1) {
        return QString();  // No category (direct child of examples/)
    }
    
    return relativePath.left(slashIndex);
}
```

---

## Dependencies

### Required Headers
```cpp
// Platform detection and paths
#include "platformInfo/resourcePaths.hpp"
#include "platformInfo/ResourceLocation.hpp"

// Resource types
#include "resInventory/resourceItem.hpp"
#include "resInventory/resourceScanner.hpp"

// Qt
#include <QDirListing>      // Qt 6.8+ required
#include <QFileInfo>
#include <QString>
#include <QList>
#include <QVector>
#include <QDateTime>
```

### CMakeLists.txt Requirements

Ensure Qt 6.10.1 is available:
```cmake
find_package(Qt6 6.10 REQUIRED COMPONENTS Core)
```

Check C++ standard (QDirListing uses C++20 iterators):
```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

---

## Phase 2 Milestones

| Milestone | Components | Success Criteria |
|-----------|-----------|------------------|
| **2A Complete** | DiscoveryScanner | Validates paths, creates ResourceLocations, passes 10+ unit tests |
| **2B Complete** | LocationScanner | Detects resource folders, enriches locations, passes 8+ unit tests |
| **2C Complete** | ResourceScanner refactor | Uses QDirListing, maintains functionality, passes existing + new tests |
| **2D Complete** | Coordinator | Orchestrates full pipeline, emits signals, passes integration tests |
| **Phase 2 Complete** | All components | Full discovery pipeline working end-to-end, documented, tested |

---

## Next Steps (After Decisions)

1. ✅ Review this plan and answer embedded questions
2. ⏳ Start implementation based on Q5 decision
3. ⏳ Create unit test files alongside each component
4. ⏳ Document any discoveries or design changes
5. ⏳ Integration testing once all components built
6. ⏳ Update Phase 1 results document with Phase 2 outcomes

---

## Notes

- **Qt 6.10.1 Available:** QDirListing requires Qt 6.8+, we have 6.10.1 ✅
- **C++20 Ready:** QDirListing uses C++20 input_iterator concept ✅
- **Phase 1 Complete:** ResourcePaths tested and working ✅
- **ResourceScanner Exists:** Can refactor incrementally, not from scratch ✅
- **UI Decoupling:** ResourceTreeWidget population should move to UI layer
- **Memory Management:** Consider smart pointers for ResourceItem storage
- **Signal/Slot Design:** Coordinator provides progress feedback for UI

---

## References

- [QDirListing Documentation](https://doc.qt.io/qt-6/qdirlisting.html)
- `2026-01-05-resource-discovery-architecture.md` - Pipeline architecture
- `Resource-Discovery-Specifications.md` - Folder structure rules
- `ResourceRefactoring-QDirListing-Design.md` - Qt API patterns
- `src/resInventory/resourceScanner.hpp` - Current implementation
