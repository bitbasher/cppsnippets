# Phase 2B: ResourceScanning - File Discovery and Inventory Population

**Date:** January 10, 2026  
**Status:** Planning  
**Author:** GitHub Copilot (Claude Sonnet 4.5)  
**Phase:** 2B - Resource File Scanning and Inventory Population

---

## Prerequisites

✅ **Phase 2A Must Be Complete:**
- `resourceDiscovery::DiscoveryEngine` implemented and tested
- `QList<resourceDiscovery::ResourceLocation>` available as input
- Each ResourceLocation contains:
  - `path` (absolute filesystem path)
  - `tier` (Installation/Machine/User)
  - `displayName` (human-readable)
  - `isWritable` (permission flag)
  - `resourceFolders` (list of folders found: templates/, libraries/, etc.)

**Current Pipeline:**
```
pathDiscovery::ResourcePaths 
    → QList<PathElement> (12 paths with tiers)
        → resourceDiscovery::DiscoveryEngine
            → QList<ResourceLocation> (validated, contains resource folders)
                → [Phase 2B goes here]
                    → resourceInventory populated
```

---

## Design Philosophy

**Incremental Implementation:** Break file scanning into manageable sub-phases, one resource type at a time.

**Test-Driven:** Each sub-phase has comprehensive tests before moving to next.

**Clear Separation:** Scanning logic separate from storage logic.

**Qt Native:** Use QDirIterator for cross-platform file listing.

---

## Scope: What Phase 2B Does

✅ **In Scope:**

- Scan resource folders for files matching expected extensions
- Validate file formats (JSON structure, font headers, etc.)
- Extract metadata from files (template names, font families, etc.)
- Populate inventory structures (TemplateStore, FontStore, LibraryStore, etc.)
- Handle hierarchical structures (examples with categories)
- Tag each resource with tier and source location
- Provide progress reporting for long scans

❌ **Out of Scope:**

- GUI integration (Phase 5)
- Manual enable/disable of individual resources (Phase 5)
- Resource editing/modification (Phase 5)
- Resource validation beyond format checking (Phase 3)
- Dependency resolution between resources (Phase 3)

---

## Namespace: resourceScanning

**New Folder:** `src/resourceScanning/`

**Purpose:** File discovery and metadata extraction. Bridges between validated locations (Phase 2A) and populated inventory (storage).

**Responsibility:** Read files, validate formats, extract metadata, delegate to inventory for storage.

**Dependencies:**
- ✅ `platformInfo` (QDir, QFileInfo, QDirIterator)
- ✅ `resourceMetadata` (ResourceType enum, type-specific metadata)
- ✅ `resourceDiscovery` (ResourceLocation input)
- ✅ `resourceInventory` (storage structures for output)
- ❌ Does NOT depend on GUI or settings persistence

---

## Sub-Phase Breakdown

### Sub-Phase 2B.1: Template Scanning (Simplest)

**Why First:** Templates are flat structure, single file type (.json), well-defined format.

**Input:** ResourceLocation with "templates" in resourceFolders list

**Process:**
1. Iterate `location.path/templates/*.json`
2. For each .json file:
   - Read file content
   - Parse JSON (QJsonDocument)
   - Validate structure (has required fields)
   - Extract metadata (name, description, category)
   - Create ResourceItem with tier tag
   - Add to TemplateStore

**Output:** TemplateStore populated with all discovered templates

**Estimated Effort:** 6 hours

---

### Sub-Phase 2B.2: Font Scanning

**Why Second:** Fonts are flat structure, two file types (.ttf, .otf), binary format (no parsing needed for Phase 2B).

**Input:** ResourceLocation with "fonts" in resourceFolders list

**Process:**
1. Iterate `location.path/fonts/*.{ttf,otf}`
2. For each font file:
   - Check file is readable
   - Extract font family name (optional: use QFontDatabase)
   - Create ResourceItem with tier tag
   - Add to FontStore

**Output:** FontStore populated with all discovered fonts

**Estimated Effort:** 4 hours

---

### Sub-Phase 2B.3: Color Scheme Scanning

**Why Third:** Similar to templates but different schema, still flat structure.

**Input:** ResourceLocation with "color-schemes" in resourceFolders list

**Process:**
1. Iterate `location.path/color-schemes/*.json`
2. For each .json file:
   - Read and parse JSON
   - Validate color scheme structure
   - Extract metadata (name, scheme type)
   - Create ResourceItem with tier tag
   - Add to ColorSchemeStore

**Output:** ColorSchemeStore populated with all discovered color schemes

**Estimated Effort:** 4 hours

---

### Sub-Phase 2B.4: Library Scanning

**Why Fourth:** .scad files, need to handle includes/dependencies (Phase 3), but basic scanning is straightforward.

**Input:** ResourceLocation with "libraries" in resourceFolders list

**Process:**
1. Iterate `location.path/libraries/*.scad`
2. For each .scad file:
   - Check file is readable
   - Extract library name from filename
   - Create ResourceItem with tier tag
   - Add to LibraryStore
   - (Dependency analysis deferred to Phase 3)

**Output:** LibraryStore populated with all discovered libraries

**Estimated Effort:** 5 hours

---

### Sub-Phase 2B.5: Example Scanning (Hierarchical)

**Why Last:** Examples have nested directory structure with categories, most complex scanning.

**Input:** ResourceLocation with "examples" in resourceFolders list

**Process:**
1. Recursively iterate `location.path/examples/**/*.scad`
2. For each .scad file:
   - Extract category from parent directory path
   - Check for metadata file (examples.json)
   - Create ResourceItem with tier tag and category
   - Add to ExampleStore (tree structure)

**Output:** ExampleStore populated with categorized examples

**Estimated Effort:** 8 hours

---

## Architecture Overview

### Class Structure

```
ResourceScanner (abstract base)
    ├── TypeScanner (abstract per-type interface)
    │   ├── TemplateScanner
    │   ├── FontScanner
    │   ├── ColorSchemeScanner
    │   ├── LibraryScanner
    │   └── ExampleScanner
    │
    ├── FileValidator (format checking)
    │   ├── JsonValidator
    │   └── FileAccessValidator
    │
    └── MetadataExtractor (per-type metadata)
        ├── TemplateMetadataExtractor
        ├── FontMetadataExtractor
        └── ColorSchemeMetadataExtractor
```

### Scanning Workflow

```cpp
// For each ResourceLocation
for (const auto& location : validatedLocations) {
    // For each resource folder in this location
    for (const QString& folderName : location.resourceFolders) {
        // Determine resource type from folder name
        ResourceType type = typeFromFolderName(folderName);
        
        // Get appropriate scanner
        TypeScanner* scanner = getScannerForType(type);
        
        // Scan and populate inventory
        scanner->scanLocation(location, folderName);
    }
}
```

---

## Detailed Design: ResourceScanner Base

### File Structure

```
src/
├── resourceScanning/
│   ├── ResourceScanner.hpp          (abstract base)
│   ├── ResourceScanner.cpp
│   ├── TypeScanner.hpp              (abstract per-type interface)
│   ├── TemplateScanner.hpp/cpp      (Sub-phase 2B.1)
│   ├── FontScanner.hpp/cpp          (Sub-phase 2B.2)
│   ├── ColorSchemeScanner.hpp/cpp   (Sub-phase 2B.3)
│   ├── LibraryScanner.hpp/cpp       (Sub-phase 2B.4)
│   ├── ExampleScanner.hpp/cpp       (Sub-phase 2B.5)
│   ├── FileValidator.hpp/cpp        (validation utilities)
│   ├── MetadataExtractor.hpp/cpp    (metadata extraction utilities)
│   └── export.hpp                   (RESOURCESCANNING_API macro)
│
tests/
└── resourceScanning/
    ├── test_template_scanner.cpp    (Sub-phase 2B.1)
    ├── test_font_scanner.cpp        (Sub-phase 2B.2)
    ├── test_colorscheme_scanner.cpp (Sub-phase 2B.3)
    ├── test_library_scanner.cpp     (Sub-phase 2B.4)
    └── test_example_scanner.cpp     (Sub-phase 2B.5)
```

---

## Technical Reference: Qt Directory Scanning

### QDirIterator (Recommended)

**Why QDirIterator:** Cross-platform, memory-efficient, supports recursive scanning, filtering.

```cpp
#include <QDirIterator>

// Non-recursive scan for .json files
QDirIterator it(location.path + "/templates", 
                QStringList() << "*.json",
                QDir::Files | QDir::Readable,
                QDirIterator::NoIteratorFlags);

while (it.hasNext()) {
    QString filePath = it.next();
    // Process file
}

// Recursive scan for examples
QDirIterator exampleIt(location.path + "/examples",
                       QStringList() << "*.scad",
                       QDir::Files | QDir::Readable,
                       QDirIterator::Subdirectories);

while (exampleIt.hasNext()) {
    QString filePath = exampleIt.next();
    QFileInfo info(filePath);
    QString category = extractCategory(info.absolutePath());
    // Process file with category
}
```

### QDir::Filters Reference

Filtering options for QDir operations (from Qt documentation):

| Constant | Value | Description |
|----------|-------|-------------|
| QDir::Dirs | 0x001 | List directories that match the filters |
| QDir::Files | 0x002 | List files |
| QDir::Readable | 0x010 | List files for which the application has read access (combine with Dirs or Files) |
| QDir::Writable | 0x020 | List files for which the application has write access (combine with Dirs or Files) |
| QDir::Hidden | 0x100 | List hidden files (on Unix, files starting with ".") |
| QDir::NoDotAndDotDot | NoDot \| NoDotDot | Do not list "." and ".." |
| QDir::AllDirs | 0x400 | List all directories (don't apply filters to directory names) |
| QDir::AllEntries | Dirs \| Files \| Drives | List directories, files, drives and symlinks |
| QDir::NoSymLinks | 0x008 | Do not list symbolic links |

**Common Combinations:**
- File scanning: `QDir::Files | QDir::Readable | QDir::NoDotAndDotDot`
- Directory scanning: `QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot`
- Recursive: Use `QDirIterator::Subdirectories` flag

---

## Technical Reference: QStandardPaths

### Cross-Platform Standard Locations

Used by pathDiscovery, useful context for understanding where resources come from:

| Path Type | Windows | macOS | Linux | Description |
|-----------|---------|-------|-------|-------------|
| **DocumentsLocation** | `C:/Users/<USER>/Documents` | `~/Documents` | `~/Documents` | User document files (generic) |
| **FontsLocation** | `C:/Windows/Fonts` (RO) | `/System/Library/Fonts` (RO) | `~/.fonts`, `/usr/share/fonts` | System and user fonts |
| **AppDataLocation** | `C:/Users/<USER>/AppData/Roaming/<APPNAME>` | `~/Library/Application Support/<APPNAME>` | `~/.local/share/<APPNAME>` | Per-app persistent data |
| **AppLocalDataLocation** | `C:/Users/<USER>/AppData/Local/<APPNAME>` | Same as AppDataLocation | Same as AppDataLocation | Local (non-roaming) app data |
| **ConfigLocation** | `C:/Users/<USER>/AppData/Local/<APPNAME>` | `~/Library/Preferences` | `~/.config` | User-specific config files |
| **GenericDataLocation** | `C:/Users/<USER>/AppData/Local`, `C:/ProgramData` | `~/Library/Application Support` | `~/.local/share`, `/usr/share` | Shared persistent data |
| **HomeLocation** | `C:/Users/<USER>` | `~` | `~` | User's home directory |
| **TempLocation** | `C:/Users/<USER>/AppData/Local/Temp` | OS-generated | `/tmp` | Temporary files |

**Note:** Phase 2B doesn't use QStandardPaths directly (that's Phase 2A's job), but understanding these helps with testing and debugging location origins.

---

## Sub-Phase 2B.1 Detailed Design: Template Scanner

### TemplateScanner Class

**File:** `src/resourceScanning/TemplateScanner.hpp`

```cpp
#ifndef TEMPLATESCANNER_HPP
#define TEMPLATESCANNER_HPP

#include <QString>
#include <QList>
#include "TypeScanner.hpp"
#include "resourceDiscovery/ResourceLocation.hpp"
#include "resourceInventory/TemplateStore.hpp"

namespace resourceScanning {

/**
 * @brief Scans template folders for .json files
 * 
 * Responsibilities:
 * - Iterate templates/ folder for .json files
 * - Parse JSON structure
 * - Validate required fields
 * - Extract template metadata
 * - Add to TemplateStore with tier and location tags
 */
class TemplateScanner : public TypeScanner {
public:
    /**
     * @brief Scan templates folder in location
     * 
     * @param location Validated resource location
     * @param folderName Resource folder name (should be "templates")
     * @param store Template store to populate
     * @return Number of templates discovered
     */
    static int scanLocation(
        const resourceDiscovery::ResourceLocation& location,
        const QString& folderName,
        resourceInventory::TemplateStore& store);
    
private:
    /**
     * @brief Validate JSON template structure
     * 
     * Checks for required fields:
     * - name (string)
     * - category (string, optional)
     * - parameters (array, optional)
     * 
     * @param json Parsed JSON document
     * @return true if valid template structure
     */
    static bool validateTemplateJson(const QJsonDocument& json);
    
    /**
     * @brief Extract template metadata from JSON
     * 
     * @param json Parsed JSON document
     * @param filePath Source file path
     * @return Template metadata object
     */
    static TemplateMetadata extractMetadata(
        const QJsonDocument& json,
        const QString& filePath);
    
    /**
     * @brief Get list of template file extensions
     * @return List of extensions (e.g., {"json"})
     */
    static QStringList templateExtensions();
};

} // namespace resourceScanning

#endif // TEMPLATESCANNER_HPP
```

### Template Metadata Structure

**File:** `src/resourceInventory/TemplateMetadata.hpp`

```cpp
#ifndef TEMPLATEMETADATA_HPP
#define TEMPLATEMETADATA_HPP

#include <QString>
#include <QStringList>
#include "resourceMetadata/ResourceTier.hpp"

namespace resourceInventory {

/**
 * @brief Metadata extracted from template JSON
 */
struct TemplateMetadata {
    QString name;                          // Template display name
    QString filePath;                      // Absolute path to .json file
    QString category;                      // Optional category
    QStringList parameters;                // Parameter names
    resourceMetadata::ResourceTier tier;   // Source tier
    QString sourceLocation;                // DisplayName from ResourceLocation
    
    // Constructor
    TemplateMetadata(const QString& n, 
                    const QString& fp,
                    const QString& cat,
                    const QStringList& params,
                    resourceMetadata::ResourceTier t,
                    const QString& src)
        : name(n), filePath(fp), category(cat), 
          parameters(params), tier(t), sourceLocation(src) {}
    
    // Default constructor for Qt containers
    TemplateMetadata() 
        : tier(resourceMetadata::ResourceTier::Unknown) {}
};

} // namespace resourceInventory

#endif // TEMPLATEMETADATA_HPP
```

### TemplateScanner Implementation Sketch

```cpp
#include "TemplateScanner.hpp"
#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace resourceScanning {

int TemplateScanner::scanLocation(
    const resourceDiscovery::ResourceLocation& location,
    const QString& folderName,
    resourceInventory::TemplateStore& store)
{
    int count = 0;
    QString templatesPath = location.path + "/" + folderName;
    
    qDebug() << "TemplateScanner: Scanning" << templatesPath;
    
    // Iterate .json files in templates folder
    QDirIterator it(templatesPath,
                    QStringList() << "*.json",
                    QDir::Files | QDir::Readable | QDir::NoDotAndDotDot,
                    QDirIterator::NoIteratorFlags);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        
        // Read file
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "TemplateScanner: Cannot read" << filePath;
            continue;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        // Parse JSON
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "TemplateScanner: JSON parse error in" 
                       << filePath << ":" << parseError.errorString();
            continue;
        }
        
        // Validate structure
        if (!validateTemplateJson(doc)) {
            qWarning() << "TemplateScanner: Invalid template structure in" << filePath;
            continue;
        }
        
        // Extract metadata
        TemplateMetadata metadata = extractMetadata(doc, filePath);
        metadata.tier = location.tier;
        metadata.sourceLocation = location.displayName;
        
        // Add to store
        store.addTemplate(metadata);
        count++;
        
        qDebug() << "TemplateScanner: Added template" 
                 << metadata.name << "from" << metadata.sourceLocation;
    }
    
    qDebug() << "TemplateScanner: Found" << count << "templates in" << templatesPath;
    return count;
}

bool TemplateScanner::validateTemplateJson(const QJsonDocument& json)
{
    if (!json.isObject()) {
        return false;
    }
    
    QJsonObject obj = json.object();
    
    // Required: name field
    if (!obj.contains("name") || !obj["name"].isString()) {
        return false;
    }
    
    // Optional but validate if present: category
    if (obj.contains("category") && !obj["category"].isString()) {
        return false;
    }
    
    // Optional but validate if present: parameters
    if (obj.contains("parameters") && !obj["parameters"].isArray()) {
        return false;
    }
    
    return true;
}

TemplateMetadata TemplateScanner::extractMetadata(
    const QJsonDocument& json,
    const QString& filePath)
{
    QJsonObject obj = json.object();
    
    QString name = obj["name"].toString();
    QString category = obj.contains("category") 
                      ? obj["category"].toString() 
                      : QString();
    
    QStringList parameters;
    if (obj.contains("parameters")) {
        QJsonArray paramsArray = obj["parameters"].toArray();
        for (const QJsonValue& val : paramsArray) {
            if (val.isString()) {
                parameters.append(val.toString());
            }
        }
    }
    
    return TemplateMetadata(name, filePath, category, parameters, 
                           resourceMetadata::ResourceTier::Unknown, 
                           QString());
}

} // namespace resourceScanning
```

### Testing Strategy for Sub-Phase 2B.1

**File:** `tests/resourceScanning/test_template_scanner.cpp`

**Test Categories:**

1. **Basic Functionality (6 tests)**
   - Empty folder returns 0 templates
   - Single valid template found and parsed
   - Multiple templates all discovered
   - Template with all optional fields
   - Template with minimal fields
   - Tier tag correctly applied

2. **Error Handling (5 tests)**
   - Invalid JSON syntax skipped with warning
   - Missing required field skipped
   - Non-template .json files skipped
   - Unreadable file skipped
   - Empty file skipped

3. **Metadata Extraction (6 tests)**
   - Name extracted correctly
   - Category extracted when present
   - Parameters array extracted
   - FilePath is absolute path
   - SourceLocation matches ResourceLocation.displayName
   - Tier matches ResourceLocation.tier

4. **Integration (3 tests)**
   - Real testFileStructure templates discovered
   - Templates from multiple locations kept separate
   - Installation vs User tier templates differentiated

**Total Estimated Tests:** ~20 tests for Sub-phase 2B.1

---

## Testing Infrastructure

### testFileStructure Requirements

Phase 2B requires populated test structure:

```
testFileStructure/
├── templates/
│   ├── basic_cube.json
│   ├── cylinder_with_params.json
│   ├── sphere_simple.json
│   └── invalid_template.json  (for error testing)
├── fonts/
│   ├── TestFont.ttf
│   └── AnotherFont.otf
├── color-schemes/
│   ├── dark_theme.json
│   └── light_theme.json
├── libraries/
│   ├── MCAD_library.scad
│   └── utils.scad
└── examples/
    ├── Basics/
    │   ├── cube_example.scad
    │   └── sphere_example.scad
    └── Advanced/
        └── parametric_gear.scad
```

**Action Item:** Create testFileStructure before implementing Sub-phase 2B.1

---

## Integration Example: Full Pipeline

```cpp
#include "pathDiscovery/ResourcePaths.hpp"
#include "resourceDiscovery/DiscoveryEngine.hpp"
#include "resourceScanning/TemplateScanner.hpp"
#include "resourceInventory/TemplateStore.hpp"

void discoverAndScanResources() {
    // Phase 1: Generate candidate paths (pathDiscovery)
    pathDiscovery::ResourcePaths pathGenerator;
    auto candidatePaths = pathGenerator.qualifiedSearchPaths();
    qDebug() << "Phase 1: Generated" << candidatePaths.size() << "candidate paths";
    
    // Phase 2A: Validate paths (resourceDiscovery)
    auto validatedLocations = 
        resourceDiscovery::DiscoveryEngine::discoverLocations(candidatePaths);
    qDebug() << "Phase 2A: Found" << validatedLocations.size() << "valid locations";
    
    // Phase 2B.1: Scan templates (resourceScanning)
    resourceInventory::TemplateStore templateStore;
    int totalTemplates = 0;
    
    for (const auto& location : validatedLocations) {
        // Only scan if this location has templates folder
        if (location.resourceFolders.contains("templates")) {
            int count = resourceScanning::TemplateScanner::scanLocation(
                location, "templates", templateStore);
            totalTemplates += count;
        }
    }
    
    qDebug() << "Phase 2B.1: Discovered" << totalTemplates << "templates";
    
    // Display results
    auto allTemplates = templateStore.getAllTemplates();
    for (const auto& tmpl : allTemplates) {
        qDebug() << "Template:" << tmpl.name
                 << "- Category:" << tmpl.category
                 << "- Source:" << tmpl.sourceLocation
                 << "- Tier:" << (int)tmpl.tier;
    }
}
```

---

## Build Integration

### CMakeLists.txt

```cmake
# Create resourceScanning library
add_library(resourceScanning SHARED
    src/resourceScanning/ResourceScanner.cpp
    src/resourceScanning/TypeScanner.cpp
    src/resourceScanning/TemplateScanner.cpp
    # More scanners added in later sub-phases
    src/resourceScanning/FileValidator.cpp
    src/resourceScanning/MetadataExtractor.cpp
)

target_include_directories(resourceScanning PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(resourceScanning PUBLIC
    Qt6::Core
    platformInfo
    resourceMetadata
    resourceDiscovery
    resourceInventory
)

# Tests for Sub-phase 2B.1
add_executable(test_template_scanner
    tests/resourceScanning/test_template_scanner.cpp
)

target_link_libraries(test_template_scanner PRIVATE
    resourceScanning
    resourceDiscovery
    resourceInventory
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
)

add_test(NAME TemplateScannerTests COMMAND test_template_scanner)
```

---

## Success Criteria Per Sub-Phase

### Sub-Phase 2B.1 Complete When:
1. ✅ TemplateScanner class implemented
2. ✅ All 20 tests pass
3. ✅ testFileStructure/templates/ populated with test data
4. ✅ Integration with Phase 2A confirmed
5. ✅ TemplateStore populated correctly
6. ✅ Tier tags applied correctly
7. ✅ Code reviewed and documented

### Sub-Phase 2B.2-2B.5: Similar criteria adapted per type

---

## Estimated Total Effort for Phase 2B

| Sub-Phase | Component | Hours |
|-----------|-----------|-------|
| **2B.1** | Template Scanner | 6 |
| **2B.2** | Font Scanner | 4 |
| **2B.3** | Color Scheme Scanner | 4 |
| **2B.4** | Library Scanner | 5 |
| **2B.5** | Example Scanner | 8 |
| | **Infrastructure** | |
| | Base classes (ResourceScanner, TypeScanner) | 3 |
| | FileValidator utilities | 2 |
| | MetadataExtractor utilities | 2 |
| | testFileStructure population | 3 |
| | Integration testing | 3 |
| | Documentation | 2 |
| **Total** | | **42 hours** |

**Recommended Approach:** Implement one sub-phase at a time, with full testing and commit before moving to next.

---

## Risks and Mitigations

### Risk 1: Large Directory Scans

**Problem:** Scanning thousands of files could block UI

**Mitigation:**
- Use QDirIterator (memory efficient)
- Provide progress callbacks
- Consider background threading (Phase 3)
- Implement cancellation mechanism

### Risk 2: Invalid File Formats

**Problem:** Malformed JSON, corrupted fonts, etc.

**Mitigation:**
- Robust validation in FileValidator
- Try/catch around parsing
- Log errors, skip invalid files
- Don't abort entire scan on single file error

### Risk 3: Duplicate Resources

**Problem:** Same template in multiple locations

**Mitigation:**
- Tier-based precedence (User > Machine > Installation)
- Allow duplicates but mark tier in metadata
- UI (Phase 5) shows source location
- User can choose which to use

### Risk 4: Performance

**Problem:** JSON parsing is expensive

**Mitigation:**
- Only parse files once
- Cache results in inventory
- Lazy loading (parse on demand, not all upfront)
- Consider binary cache (Phase 3)

---

## Next Steps After Phase 2B

### Phase 3: Resource Management
- Dependency resolution (libraries using other libraries)
- Resource validation (deeper than format checking)
- Resource caching and indexing
- Background scanning with progress
- Resource update detection

### Phase 4: Settings and Persistence
- Save/load resource preferences
- Remember which locations are enabled
- Cache scan results
- Track user customizations

### Phase 5: GUI Integration
- Resource browser UI
- Enable/disable locations
- Resource preview
- Resource editing
- Resource management

---

## Related Documents

- [2026-01-10 Phase 2A Revised Design](2026-01-10-phase2a-revised-after-reorganization.md)
- [2026-01-09 Namespace Reorganization Roadmap](2026-01-09-namespace-folder-reorganization-roadmap.md)
- [QDirIterator Documentation](https://doc.qt.io/qt-6/qdiriterator.html)
- [QJsonDocument Documentation](https://doc.qt.io/qt-6/qjsondocument.html)
- [QDir Filters Documentation](https://doc.qt.io/qt-6/qdir.html#Filter-enum)

---

## Summary

**Phase 2B Goal:** Discover and catalog all resource files in validated locations.

**Approach:** Incremental implementation, one resource type at a time.

**Input:** QList<ResourceLocation> from Phase 2A  
**Process:** File iteration, format validation, metadata extraction  
**Output:** Populated inventory stores (TemplateStore, FontStore, etc.)

**Why 5 Sub-Phases:**
- Each resource type has unique characteristics
- Incremental testing reduces integration risk
- Can ship partial functionality
- Clear progress milestones
- Easier to debug and maintain

**Status:** Ready to implement Sub-phase 2B.1 (Template Scanner) once Phase 2A is complete.

**Recommendation:** Start with testFileStructure population and Template Scanner as proof-of-concept for entire Phase 2B architecture.
