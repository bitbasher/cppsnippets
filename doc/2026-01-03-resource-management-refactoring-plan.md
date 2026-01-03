# Resource Management Refactoring Plan

**Date:** January 3, 2026  
**Status:** Planning  
**Author:** GitHub Copilot (Claude Sonnet 4.5)

---

## Current State Analysis

### Problems Identified

1. **Duplicate `ResourceType` enum** - defined in both `resourcePaths.hpp` and `ResourceTypeInfo.hpp`
2. **Duplicate `ResourceTier` enum** - defined in both `resourceTier.h` and `resourceLocationManager.hpp` \
3. **Static map placement issue** - `s_resourceTypes` is `inline const` in header (line 145 ResourceTypeInfo.hpp) which is correct for C++17, but causes ODR issues
4. **Mixed namespaces** - `resourceInfo` vs `platformInfo`
5. **Inconsistent file naming** - `resourceTier.h` vs `.hpp`
6. **Incomplete ResourceTypeInfo class** - getters take parameters they ignore and don't actually return the member values
7. **ResourceLocationManager** - mostly stubbed out, unclear purpose vs ResourcePaths
8. **No separation of concerns** - ResourcePaths trying to do too much

### Updates from Alignment (2026-01-03)
- Env var expansion: defaults now include env vars (HOME/USERPROFILE/APPDATA/XDG_*); ResourcePaths must expand to absolute paths at runtime.
- Discovery: no validation step; run discovery at startup over all paths (hardcoded + user-added). Drag/drop can add custom locations (choose tier), see NEWRESOURCES_CONTAINER_INTEGRATION.md.
- Editable resources: only color schemes and templates get editing UI (existing preference panes/templates editor to be ported). Template tree: tier → location → template items. Color schemes: two preference panels.
- ResourceRegistry (repurposed ResourceLocationManager): holds resource inventory (metadata + editable resources). Persistence of enabled/disabled/custom locations moves to a new `resourceSettings` (QSettings); no auto-removal of missing paths, just mark missing in UI.
- ResourceLocation: may need richer model/view support per QDir listing/resource metadata design docs; drag/drop of folders is a way to add custom locations.
- Tiers: still used to group defaults/access (installation: RO, machine: RO, user: RW) but not the primary structuring axis for resources.
- Work items: (1) Rework ResourcePaths methods to match new intentions (env expansion, resolved paths). (2) Repurpose ResourceLocationManager → ResourceRegistry as above.

---

## Refactoring Plan: Step-by-Step

### **Phase 1: Consolidate Type Definitions** ✅
**Goal:** Single source of truth for enums and metadata

#### **Step 1.1: Fix ResourceTypeInfo.hpp**
- Fix getter methods (they should return members, not take and return parameters)
- Make getters const
- Add missing getters for all members
- Move `s_resourceTypes` to be `static` method returning `const QMap&` (Meyer's singleton pattern)
- Remove duplicate inline constants that conflict with resourcePaths.hpp

#### **Step 1.2: Consolidate ResourceTier**
- Keep `resourceTier.h` as single source
- Rename to `resourceTier.hpp` for consistency
- Move to `resourceInfo` namespace (matches the namespace already used)
- Remove duplicate from `resourceLocationManager.hpp`
- Add utility methods for tier info

#### **Step 1.3: Remove ResourceType from resourcePaths.hpp**
- Delete duplicate `ResourceType` enum
- Delete duplicate `allTopLevelResTypes`
- Include `ResourceTypeInfo.hpp` instead

#### **Step 1.4: Update all includes**
- Search/replace all files that include these headers
- Update namespace usages

#### **Tests:**
- Unit test for ResourceTypeInfo getters
- Unit test for ResourceTierInfo methods
- Verify all enum values accessible

---

### **Phase 2: Redesign Class Responsibilities** 🎯
**Goal:** Clear separation of concerns

#### **Step 2.1: Define New Architecture**

```
ResourceTypeInfo (ResourceTypeInfo.hpp)
├─ Owns: ResourceType enum, metadata, file extensions
├─ Purpose: What types of resources exist and their properties
└─ Methods: Static queries about resource types

ResourceTierInfo (resourceTier.hpp) 
├─ Owns: ResourceTier enum, access rules, tier properties
├─ Purpose: What tiers exist and their access modes
└─ Methods: Static queries about tiers

ResourcePaths (resourcePaths.hpp/cpp)
├─ Owns: Platform-specific search paths (immutable defaults)
├─ Purpose: WHERE to look for resources on each platform
├─ Depends on: ResourceTypeInfo, ResourceTierInfo, PlatformInfo
└─ Methods:
    - defaultSearchPaths(tier) → QStringList (immutable defaults)
    - resolveInstallationPaths() → QStringList (resolved from app path)
    - resolveMachinePaths() → QStringList (resolved system paths)
    - resolveUserPaths() → QStringList (resolved user paths)
    - findResourceDir(basePath, type) → QString

ResourceLocation (ResourceLocation.hpp)
├─ Owns: Single discovered location metadata
├─ Purpose: Represents ONE folder that contains resources
└─ Properties: path, tier, enabled, writable, discovered types

ResourceDiscovery (NEW - resourceDiscovery.hpp/cpp)
├─ Owns: Discovery logic - scans filesystem
├─ Purpose: FIND actual resource folders on disk
├─ Depends on: ResourcePaths, ResourceTypeInfo, ResourceLocation
└─ Methods:
    - discoverAll() → QMap<ResourceTier, QList<ResourceLocation>>
    - discoverTier(tier) → QList<ResourceLocation>
    - validateLocation(path) → bool (checks for resource subdirs)
    - scanForTypes(path) → QList<ResourceType> (what's in folder)

ResourceRegistry (NEW or repurpose ResourceLocationManager)
├─ Owns: User preferences (enabled/disabled locations)
├─ Purpose: Track what locations user has enabled
├─ Depends on: ResourceLocation, QSettings
├─ Persists: Enabled locations, tier overrides
└─ Methods:
    - setLocations(tier, locations)
    - getEnabledLocations(tier) → QList<ResourceLocation>
    - setLocationEnabled(path, bool)
    - resetToDefaults(tier)
    - saveSettings()
    - loadSettings()
```

#### **Step 2.2: Create ResourceDiscovery class**
- New files: `resourceDiscovery.hpp/cpp`
- Extract discovery logic from ResourceLocationManager
- Implement tier-by-tier discovery
- Add validation (check for resource type subdirs)

#### **Step 2.3: Simplify ResourcePaths**
- Remove settings/preference logic
- Keep only immutable defaults and path resolution
- Remove ResourceLocation dependencies
- Focus on "where COULD resources be"

#### **Step 2.4: Rename/Repurpose ResourceLocationManager → ResourceRegistry**
- Rename files to `resourceRegistry.hpp/cpp`
- Remove discovery logic (moved to ResourceDiscovery)
- Focus on QSettings persistence
- Track enabled/disabled state
- Provide "Restore Defaults" functionality

#### **Tests:**
- Unit test ResourceDiscovery.discoverTier() with mock filesystem
- Unit test ResourcePaths.defaultSearchPaths()
- Unit test ResourceRegistry.setLocationEnabled()
- Integration test: Discovery → Registry → Enabled locations

---

### **Phase 3: Fix ResourceTypeInfo Implementation** 🔧
**Goal:** Correct implementation of metadata access

#### **Step 3.1: Fix getter methods**
```cpp
// BEFORE (wrong - takes and ignores parameters)
ResourceType gettype(const ResourceType t) { return type; }

// AFTER (correct - const getter)
ResourceType getType() const { return type; }
QString getSubDir() const { return subdirectory; }
QString getDescription() const { return description; }
QList<ResourceType> getSubResTypes() const { return subResTypes; }
QStringList getPrimaryExtensions() const { return primaryExtensions; }
QStringList getAttachmentExtensions() const { return attachmentExtensions; }
```

#### **Step 3.2: Fix static map access**
```cpp
// Make s_resourceTypes a static method (Meyer's singleton)
static const QMap<ResourceType, ResourceTypeInfo>& resourceTypes() {
    static const QMap<ResourceType, ResourceTypeInfo> map = { /* ...data... */ };
    return map;
}
```

#### **Step 3.3: Add scanner-focused query methods**
```cpp
class ResourceTypeInfo {
public:
    // Existing constructors/getters...
    
    // Static queries for scanner
    static const ResourceTypeInfo* forType(ResourceType type);
    static QString subdirFor(ResourceType type);
    static QStringList extensionsFor(ResourceType type);
    static bool isContainer(ResourceType type); // has subResTypes
    static bool canContainType(ResourceType parent, ResourceType child);
    static QList<ResourceType> topLevelTypes(); // non-nested types
    static QList<ResourceType> allTypes();
};
```

#### **Tests:**
- Unit test all getters return correct values
- Unit test static queries
- Unit test isContainer() logic
- Unit test canContainType() validation

---

### **Phase 4: Update ResourceScanner** 📁
**Goal:** Use new APIs consistently

#### **Step 4.1: Update scanner to use ResourceTypeInfo**
- Replace hardcoded extension lists
- Use ResourceTypeInfo::extensionsFor()
- Use ResourceTypeInfo::subdirFor()
- Use ResourceTypeInfo::canContainType() for nesting validation

#### **Step 4.2: Update scanner to use ResourceDiscovery**
- Scanner gets locations from ResourceRegistry
- Registry gets locations from ResourceDiscovery
- Scanner doesn't do discovery itself

#### **Tests:**
- Update test_resource_discovery.cpp
- Mock ResourceDiscovery for scanner tests
- Verify scanner uses correct extensions per type

---

### **Phase 5: Update GUI Integration** 🖥️
**Goal:** Preferences dialog uses new architecture

#### **Step 5.1: Update preferences tabs**
- InstallationTab: Shows ResourceDiscovery results for Installation tier
- MachineTab: Shows ResourceDiscovery results for Machine tier  
- UserTab: Shows ResourceDiscovery results for User tier
- Each tab: Enable/disable via ResourceRegistry

#### **Step 5.2: Add "Restore Defaults" buttons**
- Call ResourcePaths::defaultSearchPaths(tier)
- Re-run ResourceDiscovery for that tier
- Update ResourceRegistry with new locations

#### **Tests:**
- Qt Test for preferences dialog
- Mock ResourceRegistry for GUI tests

---

### **Phase 6: Namespace Consolidation** 🏗️
**Goal:** Consistent namespace usage

#### **Step 6.1: Decide namespace strategy**
```
platformInfo::           // Platform detection, OS info
  - PlatformInfo

resourceInfo::           // Resource metadata
  - ResourceType
  - ResourceTypeInfo  
  - ResourceTier
  - ResourceTierInfo

resourceLocation::       // Discovery and management
  - ResourceLocation
  - ResourcePaths
  - ResourceDiscovery
  - ResourceRegistry

resInventory::           // Scanning and inventory
  - ResourceScanner
  - ResourceItem
  - ResourceTreeModel
```

#### **Step 6.2: Move classes to correct namespaces**
- Update all namespace declarations
- Update all using declarations
- Update all qualified names

#### **Tests:**
- Recompile all - verify no ambiguities

---

### **Phase 7: Documentation & Cleanup** 📝
**Goal:** Clear, maintainable codebase

#### **Step 7.1: Update header comments**
- Document purpose of each class
- Document relationships between classes
- Add usage examples

#### **Step 7.2: Create architecture diagram**
- Visual representation of class relationships
- Data flow diagrams
- Sequence diagrams for common operations

#### **Step 7.3: Clean up dead code**
- Remove old applicationNameInfo files if still present
- Remove .cpp-recovered, .h-recovered files
- Remove unused methods

#### **Step 7.4: Update README.md**
- Document new architecture
- Explain tier system
- Provide code examples

---

## Priority & Dependencies

### Immediate (must do first):
- Phase 1.1-1.4: Fix duplicates and type definitions
- Phase 3.1-3.2: Fix ResourceTypeInfo implementation

### Next (builds on above):
- Phase 2.1-2.4: Redesign architecture
- Phase 3.3: Add scanner-focused methods

### Then (integration):
- Phase 4: Update scanner
- Phase 5: Update GUI

### Finally (polish):
- Phase 6: Namespace consolidation
- Phase 7: Documentation

---

## Risk Assessment

### Low Risk:
- Fixing getters in ResourceTypeInfo (isolated)
- Adding new methods (backward compatible)
- Documentation updates

### Medium Risk:
- Moving ResourceType enum (many files affected)
- Namespace changes (compilation errors easy to fix)

### High Risk:
- Splitting ResourceLocationManager (complex dependencies)
- Changing ResourceScanner API (used by GUI)

---

## Recommendation

Start with **Phase 1** and **Phase 3.1-3.2** - these are low-risk, high-value changes that establish the foundation. Get these building and tested before proceeding to Phase 2 (the architectural redesign).

---

## Progress Log

### 2026-01-03
- ✅ Initial plan created
- ⏳ Awaiting user review and approval
- 📅 Scheduled to resume: Morning of 2026-01-04

---

## Notes

- This plan addresses the core issues identified during the header file relocation refactoring
- Focus is on establishing clear boundaries between classes
- Each phase can be committed independently for easy rollback
- Test coverage must be maintained/improved throughout
