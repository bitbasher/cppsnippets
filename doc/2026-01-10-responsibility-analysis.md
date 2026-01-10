# Resource Classes Responsibility Analysis

**Date:** January 10, 2026  
**Purpose:** Identify functions returning lists and doing table lookups, verify they're in correct classes/namespaces  
**Status:** For Review Before Taking Action

---

## Executive Summary

**Key Findings:**

1. ✅ **ResourceTypeInfo has good data:** Subdirectories, extensions, descriptions properly defined in resourceMetadata
2. ⚠️ **Missing: Resource folder names list** - No central QStringList of expected folder names for Phase 2A validation
3. ⚠️ **Phase 2A DiscoveryEngine has logic that should be data** - Display name generation and folder name lists belong in metadata classes
4. ⚠️ **Missing: Display name patterns per tier** - Should be data-driven configuration in ResourceTier or separate class
5. ⚠️ **NewResources handling undefined** - NEWRESOURCES_CONTAINER_INTEGRATION.md describes it, but no clear home in new namespace structure

---

## Analysis by Concern

### Concern 1: Resource Folder Names

**Current State:**

| Location | What It Has |
| ---------- | ------------- |
| `resourceMetadata::ResourceTypeInfo` | Has `subdirectory` field (e.g., "templates", "fonts", "color-schemes") ✅ |
| `resourceMetadata::ResourceTypeInfo::s_resourceTypes` | QMap<ResourceType, ResourceTypeInfo> with all definitions ✅ |
| Phase 2A design doc | Has `DiscoveryEngine::expectedResourceFolders()` returning hardcoded QStringList ❌ |

**Problem:**
Phase 2A design has:

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
```

**Correct Approach:**
This should be derived from `ResourceTypeInfo::s_resourceTypes`:

```cpp
// In resourceMetadata namespace, new static method:
namespace resourceMetadata {
    // Get all resource folder names (subdirectories)
    static QStringList allResourceFolders() {
        QStringList folders;
        for (auto it = ResourceTypeInfo::s_resourceTypes.constBegin(); 
             it != ResourceTypeInfo::s_resourceTypes.constEnd(); ++it) {
            folders.append(it.value().getSubDir());
        }
        return folders;
    }
}

// Phase 2A DiscoveryEngine then uses:
QStringList DiscoveryEngine::detectResourceFolders(const QString& path)
{
    QStringList found;
    QDir dir(path);
    
    const QStringList expected = resourceMetadata::allResourceFolders();
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

**Recommendation:** ✅ Add `resourceMetadata::allResourceFolders()` helper

---

### Concern 2: NewResources Folder

**Current State:**

| Document | What It Says |
| ---------- | ------------- |
| NEWRESOURCES_CONTAINER_INTEGRATION.md | Describes "newresources" as drop-zone container with subdirectories ✅ |
| ResourceTypeInfo.hpp | Has `ResourceType::NewResources` enum value ✅ |
| ResourceTypeInfo.hpp | Has entry in `s_resourceTypes` map (presumably) ❓ |
| Phase 2A design | **NO MENTION** of "newresources" folder ❌ |
| Phase 2B design | **NO MENTION** of "newresources" folder ❌ |

**Problem:**

- NewResources is defined as a ResourceType enum but is a **container**, not a scannable type
- Phase 2A `detectResourceFolders()` should detect "newresources/" folder
- Phase 2B scanning must look inside "newresources/templates/" etc.
- No clear integration point in new namespace structure

**NewResources Characteristics:**

- Location: `<basePath>/newresources/`
- Structure: Contains subdirectories for each type (templates/, fonts/, etc.)
- Purpose: Drop-zone for user-imported resources
- Discovery: Resources inside appear as if from parent location (flattened)
- Display: Full path shown in tooltip to distinguish from standard location

**Where It Belongs:**

1. **ResourceTypeInfo** - Already has NewResources enum and s_resourceTypes entry
2. **Phase 2A DiscoveryEngine** - Should check for "newresources/" folder existence
3. **Phase 2B Scanners** - Must scan both "templates/" AND "newresources/templates/"

**Recommendation:**

- ✅ Verify ResourceTypeInfo.s_resourceTypes has NewResources entry with subdirectory="newresources"
- ✅ Phase 2A should detect "newresources" folder presence
- ✅ Phase 2B sub-phase designs must include newresources scanning
- ✅ Document this clearly in both Phase 2A and 2B revisions

---

### Concern 3: Display Name Generation

**Current State:**

| Location | What It Has |
| ---------- | ------------- |
| Phase 2A design | `DiscoveryEngine::generateDisplayName(path, tier)` - complex logic ❌ |
| Phase 2A design | `DiscoveryEngine::extractSignificantComponent(path)` - path parsing ❌ |
| ResourceTier.hpp | Enum only, no display name helpers ❌ |

**Problem:**
Display name generation is **pure presentation logic** embedded in discovery engine. Rules are:

**Installation Tier:**

- Debug/Release build → "Installation - Build Area"
- Program Files → "Installation - Program Files"
- Else → "Installation - [LastComponent]"

**Machine Tier:**

- ProgramData → "Machine - ProgramData"
- Else → "Machine - [Component]"

**User Tier:**

- Documents → "User - Documents"
- AppData/Roaming → "User - AppData Roaming"
- AppData/Local → "User - AppData Local"
- Else → "User - [LastComponent]"

**Correct Approach:**
Display name generation is **UI presentation**, not discovery logic. Options:

**Option A: Move to ResourceLocation class:**

```cpp
namespace resourceDiscovery {
    struct ResourceLocation {
        // ... existing fields ...
        
        // Generate display name from path and tier
        QString generateDisplayName() const;
        
    private:
        static QString extractSignificantComponent(const QString& path);
    };
}
```

**Option B: Move to separate DisplayNameFormatter utility:**

```cpp
namespace resourceDiscovery {
    class DisplayNameFormatter {
    public:
        static QString formatLocationName(const QString& path, 
                                         resourceMetadata::ResourceTier tier);
    private:
        static QString extractComponent(const QString& path, 
                                       resourceMetadata::ResourceTier tier);
    };
}
```

**Option C: Make it data-driven with patterns:**

```cpp
namespace resourceMetadata {
    struct TierDisplayRules {
        QString tierPrefix;     // "Installation", "Machine", "User"
        QStringList patterns;   // {"Documents" → "Documents", "AppData/Roaming" → "AppData Roaming"}
        QString defaultFormat;  // "{tier} - {lastComponent}"
    };
    
    static const QMap<ResourceTier, TierDisplayRules> displayRules = {
        { ResourceTier::Installation, {"Installation", {"Program Files", "Debug", "Release"}, ...} },
        { ResourceTier::Machine, {"Machine", {"ProgramData"}, ...} },
        { ResourceTier::User, {"User", {"Documents", "AppData"}, ...} }
    };
}
```

**Recommendation:**

- ✅ **Option A** is simplest - make it a member function of ResourceLocation
- Display name is property of the location, so it belongs with the location
- Keeps DiscoveryEngine focused on filesystem validation only

---

### Concern 4: Tier-Based Default Access

**Current State:**

| Location | What It Has |
| ---------- | ------------- |
| ResourceAccess.hpp | `static const QMap<ResourceTier, Access> accessByTier` ✅ |

**Status:** ✅ **CORRECT** - This is exactly the right approach!

**Example:**

```cpp
static const QMap<ResourceTier, Access> accessByTier = {
    { ResourceTier::Installation, Access::ReadOnly },
    { ResourceTier::Machine, Access::ReadOnly },
    { ResourceTier::User, Access::ReadWrite }
};
```

This is **data-driven table lookup** done correctly. We should follow this pattern for other concerns.

---

### Concern 5: Resource Type Extensions

**Current State:**

| Location | What It Has |
| ---------- | ------------- |
| ResourceTypeInfo | `primaryExtensions` field ✅ |
| ResourceTypeInfo | `attachmentExtensions` field ✅ |
| ResourceTypeInfo::s_resourceTypes | Complete definitions per type ✅ |
| pathDiscovery::ResourcePaths | `static QStringList resourceExtensions(ResourceType type)` ✅ |

**Analysis:**

```cpp
// In ResourcePaths.hpp
static QStringList resourceExtensions(ResourceType type);
```

This is a **convenience accessor** that delegates to ResourceTypeInfo. Current implementation likely:

```cpp
QStringList ResourcePaths::resourceExtensions(ResourceType type) {
    const ResourceTypeInfo* info = resourceTypeInfo(type);
    if (info) {
        return info->getPrimaryExtensions();
    }
    return QStringList();
}
```

**Status:** ✅ **ACCEPTABLE** - It's a convenience wrapper in pathDiscovery that delegates to resourceMetadata

**Alternative:** Could be moved to resourceMetadata namespace as static helper:

```cpp
namespace resourceMetadata {
    static QStringList getExtensionsForType(ResourceType type) {
        auto it = ResourceTypeInfo::s_resourceTypes.constFind(type);
        if (it != ResourceTypeInfo::s_resourceTypes.constEnd()) {
            return it.value().getPrimaryExtensions();
        }
        return QStringList();
    }
}
```

**Recommendation:** ⚠️ **Consider moving** to resourceMetadata for consistency, but not urgent

---

## Summary of Issues and Recommendations

| Issue | Current Location | Should Be In | Priority | Effort |
| ------- | ----------------- | -------------- | ---------- | -------- |
| **Expected resource folders list** | Phase 2A design hardcoded | `resourceMetadata::allResourceFolders()` static helper | High | 1 hour |
| **NewResources integration** | Mentioned but not integrated | Phase 2A/2B designs + ResourceTypeInfo verification | High | 2 hours |
| **Display name generation** | DiscoveryEngine (Phase 2A design) | ResourceLocation member function | Medium | 2 hours |
| **Display name patterns** | Hardcoded in logic | Could be data-driven (low priority) | Low | 4 hours |
| **Resource extensions accessor** | pathDiscovery::ResourcePaths | Could move to resourceMetadata | Low | 1 hour |

---

## Recommended Actions

### Action 1: Add resourceMetadata::allResourceFolders()

**File:** `src/resourceMetadata/ResourceTypeInfo.hpp`

**Add:**

```cpp
namespace resourceMetadata {
    /**
     * @brief Get all resource folder names for scanning
     * @return List of subdirectory names from all resource types
     */
    inline static QStringList allResourceFolders() {
        QStringList folders;
        for (auto it = ResourceTypeInfo::s_resourceTypes.constBegin(); 
             it != ResourceTypeInfo::s_resourceTypes.constEnd(); ++it) {
            const QString& subdir = it.value().getSubDir();
            if (!subdir.isEmpty()) {
                folders.append(subdir);
            }
        }
        return folders;
    }
}
```

**Update Phase 2A design:** Change `DiscoveryEngine::expectedResourceFolders()` to use `resourceMetadata::allResourceFolders()`

---

### Action 2: Verify NewResources in ResourceTypeInfo

**File:** `src/resourceMetadata/ResourceTypeInfo.cpp` (or wherever s_resourceTypes is defined)

**Verify entry exists:**

```cpp
const QMap<ResourceType, ResourceTypeInfo> ResourceTypeInfo::s_resourceTypes = {
    // ... other entries ...
    { ResourceType::NewResources, 
      ResourceTypeInfo(ResourceType::NewResources, 
                      "newresources",  // subdirectory
                      "Drop-zone for imported resources",
                      {},  // no sub-types (it's a container)
                      {},  // no primary extensions (container, not files)
                      {})  // no attachments
    }
};
```

**Update Phase 2A design:** Add section on NewResources handling

**Update Phase 2B design:** Each sub-phase scanner must check both standard and newresources paths

---

### Action 3: Move Display Name to ResourceLocation

**Update Phase 2A design:**

Change ResourceLocation struct to:

```cpp
namespace resourceDiscovery {
    struct ResourceLocation {
        QString path;
        resourceMetadata::ResourceTier tier;
        QString displayName;  // Still stored, but generated by method below
        bool isWritable;
        QStringList resourceFolders;
        
        // Constructor
        ResourceLocation(const QString& p, 
                        resourceMetadata::ResourceTier t,
                        bool w,
                        const QStringList& rf)
            : path(p), tier(t), isWritable(w), resourceFolders(rf)
        {
            displayName = generateDisplayName();  // Auto-generate
        }
        
        // Generate display name from path and tier
        QString generateDisplayName() const;
        
    private:
        static QString extractSignificantComponent(const QString& path);
    };
}
```

**Remove from DiscoveryEngine:**

- Remove `generateDisplayName()` static method
- Remove `extractSignificantComponent()` static method
- Update `discoverLocations()` to not call these

---

### Action 4: Document NewResources in Phase Designs

**Phase 2A updates:**

- Add "NewResources Container" section explaining discovery
- Update `detectResourceFolders()` to note "newresources" will be found
- Explain that Phase 2B handles actual scanning of contents

**Phase 2B updates:**

- Each sub-phase (2B.1-2B.5) must document newresources scanning
- Example for 2B.1 Template Scanner:

  ```cpp
  // Scan standard location
  count += scanDirectory(basePath, "templates", ...);
  
  // Scan newresources drop-zone
  count += scanDirectory(basePath, "newresources/templates", ...);
  ```

---

## Questions for Review

### Q1: ResourceMetadata Helper Functions

Should we add more helpers to resourceMetadata for common queries?

**Examples:**

```cpp
namespace resourceMetadata {
    // Get all resource folder names
    static QStringList allResourceFolders();
    
    // Get extensions for a type
    static QStringList getExtensionsForType(ResourceType type);
    
    // Get description for a type
    static QString getDescriptionForType(ResourceType type);
    
    // Check if type is a container
    static bool isContainerType(ResourceType type);
    
    // Get sub-resource types for a container
    static QList<ResourceType> getSubTypesForContainer(ResourceType type);
}
```

**Recommendation:** Yes, these reduce code duplication and centralize metadata access

---

### Q2: Display Name Generation Approach

Which approach do you prefer?

- **A:** Member function on ResourceLocation (simple, cohesive)
- **B:** Separate DisplayNameFormatter utility (flexible, testable)
- **C:** Data-driven with patterns (most flexible, more complex)

**My Recommendation:** A for now (simple), can refactor to C later if UI needs more customization

---

### Q3: NewResources vs Standard Scanning

Should Phase 2B scanners:

- **A:** Always scan both locations (standard + newresources) for every type
- **B:** Make newresources scanning optional/conditional
- **C:** Treat newresources as separate locations in discovery

**My Recommendation:** A - scan both automatically, simpler for users

---

### Q4: PathDiscovery Accessor Functions

Should `pathDiscovery::ResourcePaths` keep convenience accessors like:

- `resourceExtensions(ResourceType)`
- `resourceSubdirectory(ResourceType)`

Or should all code use `resourceMetadata` directly?

**My Recommendation:** Keep for now (backward compatibility), but document they delegate to resourceMetadata

---

## Next Steps

1. **Review this analysis** - Discuss questions and recommendations
2. **Prioritize actions** - Which changes to make first?
3. **Update Phase 2A design** - Incorporate corrections (allResourceFolders, displayName, NewResources)
4. **Update Phase 2B design** - Add NewResources scanning to each sub-phase
5. **Implement changes** - Once design updates agreed upon
6. **Update existing code** - If any resourceMetadata changes affect current code

---

## Related Documents

- [ResourceTypeInfo.hpp](../src/resourceMetadata/ResourceTypeInfo.hpp)
- [ResourceTier.hpp](../src/resourceMetadata/ResourceTier.hpp)
- [ResourceAccess.hpp](../src/resourceMetadata/ResourceAccess.hpp)
- [ResourcePaths.hpp](../src/pathDiscovery/ResourcePaths.hpp)
- [NEWRESOURCES_CONTAINER_INTEGRATION.md](NEWRESOURCES_CONTAINER_INTEGRATION.md)
- [Phase 2A Revised Design](2026-01-10-phase2a-revised-after-reorganization.md)
- [Phase 2B Design](2026-01-10-phase2b-resource-scanning.md)
