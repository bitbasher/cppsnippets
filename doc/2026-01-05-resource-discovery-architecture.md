# Resource Discovery & Management Architecture

**Date:** January 5, 2026  
**Status:** Implementation in Progress  
**Author:** GitHub Copilot (Claude Sonnet 4.5)

---

## Overview

This document defines the data flow and component responsibilities for the resource discovery and management system. Resources (templates, examples, fonts, color schemes) are discovered in three tiers (Installation, Machine, User) and made available through a unified inventory.

---

## Data Flow Pipeline

```
1. ResourcePaths::qualifiedSearchPaths()
   └─> QList<PathElement> (s_discoveryPaths)
   
2. DiscoveryScanner(s_discoveryPaths)
   └─> QList<ResourceLocation> (validatedLocations)
   
3. LocationScanner(validatedLocations)
   └─> QList<ResourceLocation> (enrichedLocations with metadata)
   
4. ResourceScanner(s) (enrichedLocations)
   └─> Resource objects per type
   
5. ResourceInventory
   └─> Stores all discovered resources with metadata
```

---

## Component Definitions

### **1. ResourcePaths** (existing, refactored)
**File:** `src/platformInfo/resourcePaths.hpp/cpp`  
**Namespace:** `platformInfo`

**Purpose:** Generate qualified search paths for resource discovery

**Key Method:**
```cpp
QList<PathElement> qualifiedSearchPaths() const;
```

**Output:**
- **Type:** `QList<PathElement>` where `PathElement = {ResourceTier, QString path}`
- **Contains:** 
  - Environment variables expanded to absolute paths
  - Folder name suffix rules applied per tier
  - Sibling installations included (LTS ↔ Nightly)
  - User-designated paths from QSettings
- **Storage:** Not stored in ResourcePaths itself - returned as output
- **Consumer:** DiscoveryScanner

**Responsibilities:**
- ✅ Expand environment variables (`${VAR}`, `%VAR%`)
- ✅ Apply folder name appending rules (trailing `/` → append folderName + suffix)
- ✅ Detect sibling installations (bidirectional LTS ↔ Nightly)
- ✅ Load user-designated paths from QSettings
- ✅ Return single consolidated list with tier embedded in each PathElement

**Status:** ✅ **COMPLETE** (Phase 1 of refactoring)

---

### **2. DiscoveryScanner** (NEW - to be created)
**File:** `src/resourceMgmt/discoveryScanner.hpp/cpp` (proposed)  
**Namespace:** `resourceMgmt` (proposed)

**Purpose:** Validate which qualified search paths actually exist on the filesystem

**Key Method:**
```cpp
static QList<ResourceLocation> scanDiscoveryPaths(const QList<PathElement>& discoveryPaths);
```

**Input:**
- **Type:** `QList<PathElement>` from `ResourcePaths::qualifiedSearchPaths()`
- **Source:** Output of step 1

**Output:**
- **Type:** `QList<ResourceLocation>` (initial/candidate locations)
- **Contains:** Only paths that exist on disk
- **Filtering:** 
  - Removes non-existent paths
  - Removes inaccessible paths (no read permission)
  - Deduplicates identical paths from different sources
  - Preserves tier information from PathElement

**Responsibilities:**
- Check filesystem existence (`QFileInfo::exists()`)
- Check read accessibility (`QFileInfo::isReadable()`)
- Create initial `ResourceLocation` objects with:
  - `path` - absolute filesystem path
  - `tier` - from PathElement
  - `exists` - true (filtered to only existing)
  - `isWritable` - from filesystem check
  - `isEnabled` - default true (user can disable later)
  - `displayName` - derived from path
  - `hasResourceFolders` - false (not yet checked)

**Status:** ⏳ **TO BE IMPLEMENTED**

---

### **3. LocationScanner** (NEW - to be created)
**File:** `src/resourceMgmt/locationScanner.hpp/cpp` (proposed)  
**Namespace:** `resourceMgmt` (proposed)

**Purpose:** Enrich locations with resource metadata - determine what resource types exist

**Key Method:**
```cpp
static QList<ResourceLocation> enrichLocations(const QList<ResourceLocation>& candidateLocations);
```

**Input:**
- **Type:** `QList<ResourceLocation>` (candidate locations from DiscoveryScanner)
- **Source:** Output of step 2

**Output:**
- **Type:** `QList<ResourceLocation>` (enriched with metadata)
- **Enhancement:** Updates `hasResourceFolders` field based on discovery

**Responsibilities:**
- Scan each location for resource type subdirectories
  - Check for `examples/`, `fonts/`, `color-schemes/`, etc.
  - Use `ResourceTypeInfo::subdirFor()` for expected names
- Update `ResourceLocation::hasResourceFolders` flag
  - `true` if ANY resource subdirectory found
  - `false` if location has no recognizable resources
- Optionally store list of discovered types (future enhancement)
- Preserve all other location metadata from DiscoveryScanner

**Status:** ⏳ **TO BE IMPLEMENTED**

---

### **4. ResourceScanner(s)** (existing, to be refactored)
**File:** `src/resourceMgmt/resourceScanner.hpp/cpp` (existing)  
**Namespace:** `resInventory` (current)

**Purpose:** Scan locations for actual resource files and create inventory entries

**Key Method:**
```cpp
// Option A: Single scanner with type parameter
QList<ResourceItem*> scanLocationsForType(
    const QList<ResourceLocation>& locations,
    ResourceType type
);

// Option B: Per-type scanners
class TemplateScanner { ... };
class FontScanner { ... };
class ColorSchemeScanner { ... };
class ExampleScanner { ... };
```

**Input:**
- **Type:** `QList<ResourceLocation>` (enriched locations from LocationScanner)
- **Source:** Output of step 3
- **Additional:** `ResourceType` parameter (which type to scan for)

**Output:**
- **Type:** `QList<ResourceItem*>` or similar (resource objects)
- **Contains:** Actual resource files discovered
  - File paths
  - Metadata (name, description, tier)
  - Type-specific properties (for templates: category, dependencies)

**Responsibilities:**
- Visit each location's resource subdirectory for specified type
- Use `ResourceTypeInfo::extensionsFor(type)` for file filtering
- Create ResourceItem (or type-specific) objects for each found resource
- Apply type-specific parsing/validation
- Handle nested resources (e.g., templates within categories)

**Decision Point:**
- **Single scanner** vs **per-type scanners** - not decided yet
- Depends on complexity of type-specific logic

**Status:** 🔧 **EXISTS - NEEDS REFACTORING**

---

### **5. ResourceInventory** (NEW - to be created)
**File:** `src/resourceMgmt/resourceInventory.hpp/cpp` (proposed)  
**Namespace:** `resourceMgmt` (proposed)

**Purpose:** Central storage for all discovered resources with fast lookup

**Key Structure:**
```cpp
class ResourceInventory {
public:
    // Add resources from scanners
    void addResource(ResourceItem* item);
    void addResources(const QList<ResourceItem*>& items);
    
    // Query by type
    QList<ResourceItem*> resourcesByType(ResourceType type) const;
    
    // Query by tier
    QList<ResourceItem*> resourcesByTier(ResourceTier tier) const;
    
    // Query by location
    QList<ResourceItem*> resourcesAtLocation(const QString& locationPath) const;
    
    // Clear and rebuild
    void clear();
    
private:
    // Primary storage
    QList<ResourceItem*> m_allResources;
    
    // Indexes for fast lookup
    QMultiMap<ResourceType, ResourceItem*> m_byType;
    QMultiMap<ResourceTier, ResourceItem*> m_byTier;
    QMultiMap<QString, ResourceItem*> m_byLocation;
};
```

**Input:**
- **Type:** `QList<ResourceItem*>` from ResourceScanner(s)
- **Source:** Output of step 4 (multiple scanner runs)

**Storage:**
- All discovered resources
- Multiple indexes for efficient queries
- Does NOT store locations (those managed separately)

**Responsibilities:**
- Store resource inventory
- Provide fast lookup by type/tier/location
- Manage resource lifecycle (ownership)
- Notify observers on changes (signals/slots)

**Status:** ⏳ **TO BE IMPLEMENTED**

---

## Storage and Lifecycle

### **Where is `s_discoveryPaths` stored?**

**Option A:** No persistent storage
```cpp
// Generate on-demand in discovery coordinator
ResourcePaths paths;
paths.setSuffix(isNightly ? " (Nightly)" : "");
QList<PathElement> discoveryPaths = paths.qualifiedSearchPaths();

DiscoveryScanner::scanDiscoveryPaths(discoveryPaths);
```

**Option B:** Cached in discovery coordinator class
```cpp
class ResourceDiscoveryCoordinator {
public:
    void runFullDiscovery();
    
private:
    QList<PathElement> m_discoveryPaths;          // Step 1 output
    QList<ResourceLocation> m_validatedLocations;  // Step 2 output
    QList<ResourceLocation> m_enrichedLocations;   // Step 3 output
    ResourceInventory m_inventory;                 // Step 5 storage
};
```

**Recommendation:** **Option B** - cache in coordinator for inspection/debugging

---

### **ResourceLocation vs Resource**

**Critical Distinction:**

- **ResourceLocation** = A **place** where resources have been found
  - Example: `C:/Program Files/OpenSCAD/examples/`
  - Represents a directory, not a specific file
  - Multiple resource files exist at one location
  - Stored as `QList<ResourceLocation>` indexed by tier

- **ResourceItem/Resource** = An actual **resource file**
  - Example: `C:/Program Files/OpenSCAD/examples/Basics/sphere.scad`
  - Represents a specific file
  - Belongs to one location, has one type
  - Stored in ResourceInventory indexed by type/tier/location

---

## Persistence

### **QSettings Storage**

**ResourcePaths:**
- User-designated paths (additional search paths added by user)
- Key: `ScadTemplates/ResourcePaths`
- Format: `QStringList`

**ResourceRegistry (future):**
- Enabled/disabled state per location
- Key: `ScadTemplates/EnabledLocations`
- Format: QMap<QString path, bool enabled>

**ResourceInventory:**
- Not persisted - rebuilt on each startup
- Discovery runs automatically at application launch

---

## Phase Implementation Plan

### **Phase 1: Path Generation** ✅ **COMPLETE**
- [x] Environment variable expansion
- [x] Folder name appending rules
- [x] Sibling installation detection
- [x] User-designated paths from QSettings
- [x] Single `qualifiedSearchPaths()` output
- [x] Unit tests (28 tests)
- [x] Tools (settings-generator, test-env-expansion)

### **Phase 2: Discovery Pipeline** ⏳ **NEXT**
**Step 2.1:** Create DiscoveryScanner
- Validate paths exist on filesystem
- Create initial ResourceLocation objects
- Filter and deduplicate

**Step 2.2:** Create LocationScanner
- Check for resource subdirectories
- Update `hasResourceFolders` metadata
- Preserve tier and access info

**Step 2.3:** Create ResourceDiscoveryCoordinator
- Orchestrate pipeline steps
- Cache intermediate results
- Provide unified discovery API

**Step 2.4:** Tests
- Mock filesystem for DiscoveryScanner tests
- Verify LocationScanner detects subdirectories
- Integration test for full pipeline

### **Phase 3: Resource Scanning** 🔜 **FUTURE**
- Refactor existing ResourceScanner
- Create per-type scanners (if needed)
- Integrate with enriched locations

### **Phase 4: Inventory & Registry** 🔜 **FUTURE**
- Create ResourceInventory class
- Repurpose ResourceLocationManager → ResourceRegistry
- Implement QSettings persistence

### **Phase 5: GUI Integration** 🔜 **FUTURE**
- Update preferences dialog
- Tree view with tier organization
- Enable/disable per location
- Add custom paths

---

## Questions to Resolve

1. **DiscoveryScanner vs LocationScanner naming**
   - Current: DiscoveryScanner validates paths, LocationScanner enriches
   - Alternative: PathValidator, LocationEnricher
   - Alternative: Just merge into single DiscoveryScanner with two passes

2. **Single ResourceScanner vs per-type scanners**
   - Templates have complex nesting → TemplateScanner?
   - Fonts are simple → FontScanner?
   - Or single scanner with type-specific handlers?

3. **ResourceItem class hierarchy**
   - Base class `ResourceItem` with type-specific subclasses?
   - Or single class with variant properties?

4. **Storage of "what types exist at location"**
   - Add `QList<ResourceType> discoveredTypes` to ResourceLocation?
   - Or query ResourceInventory when needed?

---

## Dependencies

```
ResourcePaths (platformInfo)
    ↓
PathElement (tier + path)
    ↓
DiscoveryScanner (resourceMgmt)
    ↓
ResourceLocation (candidate locations)
    ↓
LocationScanner (resourceMgmt)
    ↓
ResourceLocation (enriched with metadata)
    ↓
ResourceScanner(s) (resInventory)
    ↓
ResourceItem (actual resources)
    ↓
ResourceInventory (resourceMgmt)
```

**Cross-cutting:**
- `ResourceTypeInfo` - used by LocationScanner and ResourceScanner
- `ResourceTier` & `Access` - used throughout pipeline
- `QSettings` - used by ResourcePaths (user paths) and future ResourceRegistry

---

## Next Steps (January 5, 2026)

1. ✅ Clarify architecture and data flow
2. ✅ Document component responsibilities
3. ✅ Update refactoring plan with discovered architecture
4. ⏳ Commit work to date (this document + Phase 1 code)
5. 🛌 Sleep
6. 🔄 Resume with Phase 2 implementation (DiscoveryScanner)

---

## Notes

- This architecture separates concerns cleanly:
  - **ResourcePaths** = WHERE to look (path generation)
  - **DiscoveryScanner** = WHICH paths are valid (filesystem validation)
  - **LocationScanner** = WHAT'S at each location (metadata enrichment)
  - **ResourceScanner** = ACTUAL resources (file scanning)
  - **ResourceInventory** = STORAGE (indexed lookup)

- Pipeline is linear and each stage can be tested independently
- Each component has single responsibility
- Intermediate results can be cached for debugging/inspection

