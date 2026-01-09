# Namespace and Folder Reorganization Roadmap

**Date:** 2026-01-09  
**Purpose:** Reorganize code structure to clarify responsibilities and dependencies

---

## Current Problems

1. **Mixed namespaces** - `resourceInfo` and `platformInfo` in same file
2. **Unclear responsibilities** - Resource knowledge mixed with platform knowledge
3. **Legacy folder names** - Don't match current responsibilities
4. **Missing discovery phase** - Path generation jumps directly to scanning
5. **No clear dependency hierarchy** - Hard to understand what depends on what

---

## Responsibility Definitions

### PlatformInfo
- **Purpose:** Per-platform info, locations, env vars
- **Knowledge:** OS type, CPU arch, home directory, executable location
- **Deliverables:** Works on macOS, Windows, Linux
- **Does NOT:** Know about resources

### ResourceMetadata
- **Purpose:** Per-resource information and definitions
- **Knowledge:** Resource types, unique lowercase names, extensions, subdirectories
- **Deliverables:** Type definitions, tier concepts (Installation/Machine/User)
- **Does NOT:** Know about specific paths or discovery

### PathDiscovery (ResourcePaths)
- **Purpose:** WHERE to look - generates candidate search paths
- **Knowledge:** Default path lists, env var templates, folder name rules
- **Uses:** QStandardPaths for runtime path info
- **Output:** `QList<PathElement>` - qualified search paths with tier tags
- **Does NOT:** Validate existence or scan for files

### ResourceDiscovery (NEW)
- **Purpose:** WHICH paths actually exist and contain resources
- **Input:** `QList<PathElement>` from PathDiscovery
- **Validation:** Checks existence, readability, presence of resource subdirs
- **Output:** `QList<ResourceLocation>` - validated folders with metadata
- **Does NOT:** Look inside resource files

### ResourceScanning
- **Purpose:** WHAT files exist in validated locations
- **Input:** `QList<ResourceLocation>` from ResourceDiscovery
- **Process:** Uses QDirListing to find files, validates extensions
- **Output:** Populates inventory objects
- **Does NOT:** Store resources (delegates to inventory)

### ResourceInventory
- **Purpose:** HOW to store discovered resources
- **Storage:** Lists for flat resources, Trees for hierarchical
- **Per-Type:** TemplateStore, LibraryStore, etc.
- **Does NOT:** Discover or scan (receives already-discovered resources)

---

## Proposed Folder Structure

```
src/
├── platformInfo/                      # Platform Abstraction Layer
│   ├── platformInfo.hpp/cpp           # OS info, CPU arch, screens
│   └── export.hpp                     # PLATFORMINFO_API macro
│
├── resourceMetadata/                  # Resource Type Definitions (NEW FOLDER)
│   ├── ResourceType.hpp               # ResourceType enum
│   ├── ResourceTypeInfo.hpp/cpp       # Per-type metadata
│   ├── ResourceTier.hpp               # Installation/Machine/User enum
│   ├── ResourceAccess.hpp             # Access enum, accessByTier map
│   └── export.hpp                     # RESOURCEMETADATA_API macro
│
├── pathDiscovery/                     # Path List Generation (NEW FOLDER)
│   ├── PathElement.hpp                # (tier, path) pair
│   ├── ResourcePaths.hpp/cpp          # Default paths, env expansion
│   └── export.hpp                     # PATHDISCOVERY_API macro
│
├── resourceDiscovery/                 # Location Validation (NEW FOLDER)
│   ├── ResourceLocation.hpp/cpp       # Validated folder metadata
│   ├── DiscoveryEngine.hpp/cpp        # Path validation logic
│   └── export.hpp                     # RESOURCEDISCOVERY_API macro
│
├── resourceScanning/                  # File Discovery (NEW FOLDER)
│   ├── ResourceScanner.hpp/cpp        # Base scanner interface
│   ├── DirectoryScanner.hpp/cpp       # QDirListing-based scanning
│   ├── FileValidator.hpp/cpp          # Extension/format validation
│   └── export.hpp                     # RESOURCESCANNING_API macro
│
└── resourceInventory/                 # Storage (RENAMED from resInventory)
    ├── ResourceStore.hpp/cpp          # Base storage interface
    ├── ResourceList.hpp/cpp           # List-based storage
    ├── ResourceTree.hpp/cpp           # Tree-based storage
    ├── TemplateStore.hpp/cpp          # Template-specific storage
    ├── LibraryStore.hpp/cpp           # Library-specific storage
    └── export.hpp                     # RESOURCEINVENTORY_API macro
```

---

## Namespace Hierarchy

```
platformInfo (no resource knowledge)
    ↓
resourceMetadata (pure definitions)
    ↓
pathDiscovery (generates candidate paths)
    ↓
resourceDiscovery (validates paths → locations)
    ↓
resourceScanning (discovers files in locations)
    ↓
resourceInventory (stores discovered resources)
```

### Dependency Rules:
- Each layer only depends on layers above it
- No circular dependencies
- platformInfo is completely independent
- resourceMetadata is pure definitions (no Qt dependencies beyond QString)

---

## Migration Map

| Current Location | Current Namespace | Move To | New Namespace |
|-----------------|-------------------|---------|---------------|
| platformInfo/platformInfo.* | platformInfo | platformInfo/ | platformInfo |
| platformInfo/ResourceTypeInfo.* | (mixed) | resourceMetadata/ | resourceMetadata |
| platformInfo/resourcePaths.* (enums) | resourceInfo | resourceMetadata/ | resourceMetadata |
| platformInfo/resourcePaths.* (PathElement) | platformInfo | pathDiscovery/ | pathDiscovery |
| platformInfo/resourcePaths.* (ResourcePaths) | platformInfo | pathDiscovery/ | pathDiscovery |
| *(new code)* | - | resourceDiscovery/ | resourceDiscovery |
| *(new code)* | - | resourceScanning/ | resourceScanning |
| resInventory/* | (mixed) | resourceInventory/ | resourceInventory |

---

## Implementation Phases

### Phase 1: Create New Folder Structure
- Create 5 new folders: resourceMetadata, pathDiscovery, resourceDiscovery, resourceScanning, resourceInventory
- Create export.hpp in each with appropriate macros
- Update CMakeLists.txt to include new folders

### Phase 2: Split resourceMetadata
- Move ResourceType enum → resourceMetadata/ResourceType.hpp
- Move ResourceTypeInfo class → resourceMetadata/ResourceTypeInfo.*
- Create ResourceTier.hpp (from resourcePaths.hpp enums)
- Create ResourceAccess.hpp (Access enum, accessByTier map)
- Update all includes

### Phase 3: Split pathDiscovery
- Move PathElement → pathDiscovery/PathElement.hpp
- Move ResourcePaths → pathDiscovery/ResourcePaths.*
- Remove resourceInfo namespace references
- Update all includes

### Phase 4: Create resourceDiscovery
- Design ResourceLocation class (tier, path, exists, readable, resourceTypes[])
- Implement DiscoveryEngine::discoverLocations()
- Write unit tests for validation logic

### Phase 5: Create resourceScanning
- Design ResourceScanner interface
- Implement DirectoryScanner using QDirListing
- Implement FileValidator for extension checking
- Write unit tests

### Phase 6: Rename and Reorganize Inventory
- Rename resInventory → resourceInventory
- Consolidate namespace to resourceInventory
- Update all includes and CMakeLists.txt

### Phase 7: Integration and Testing
- Wire all phases together
- Update main application to use new structure
- Run full test suite
- Update documentation

---

## Benefits of This Reorganization

✅ **Clear separation of concerns** - Each namespace has ONE well-defined job  
✅ **Linear dependency flow** - No circular dependencies  
✅ **Discovery phase is explicit** - No longer hidden in path generation  
✅ **Namespaces match folder names** - Easy to locate code  
✅ **Platform independent of resources** - Can test platform detection separately  
✅ **Scanning separated from storage** - Can change one without affecting the other  
✅ **Easier to understand** - New developers can follow the flow  
✅ **Easier to test** - Each layer can be unit tested independently  
✅ **Easier to extend** - Adding new resource types or storage methods is isolated  

---

## Notes

- This is a significant refactoring that will touch many files
- Should be done incrementally with commits after each phase
- Each phase should maintain buildability
- Tests should pass after each phase
- Consider feature branch for this work
- Documentation should be updated alongside code changes

---

## Related Documents

- [QStandardPaths StandardLocation Reference](doc/stdpathstable.md)
- [QtExploration README](QtExploration/README.md) - Test findings on path expansion
- [Phase 2A DiscoveryScanner Design](doc/2026-01-06-phase2a-discoveryscanner-detailed-design.md)
