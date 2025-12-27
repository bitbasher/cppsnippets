# Location-Based Discovery: defaultElements() Flow

## Overview

`ResourcePaths::defaultElements()` is the central method for location-based resource discovery. It replaces the old tier-based approach with a single ordered list of locations, each tagged with its tier membership.

## Key Principle

**From tier-based to location-based:**
- Old: Separate methods per tier (`defaultInstallSearchPaths()`, `defaultMachineSearchPaths()`, etc.)
- New: Single ordered list where each location carries a `ResourceTier` enum tag

## Flow Diagram

```
defaultElements() invocation
    │
    ├─► Installation Tier
    │   └─► For each s_defaultInstallSearchPaths entry:
    │       1. expandEnvVars(raw) → resolves %PROGRAMFILES%, ${HOME}, etc.
    │       2. Apply suffix rule:
    │          - If ends with "/" → append "openscad" + m_suffix
    │          - Else → use as-is
    │       3. Join with applicationPath (make absolute)
    │       4. Canonicalize (resolve .., symlinks)
    │       5. Create ResourceLocation with tier=Installation
    │
    ├─► Machine Tier
    │   └─► For each s_defaultMachineSearchPaths entry:
    │       1. expandEnvVars(raw) → resolves %PROGRAMDATA%, etc.
    │       2. No suffix rules (used as-is after expansion)
    │       3. Already absolute, just canonicalize
    │       4. Create ResourceLocation with tier=Machine
    │
    ├─► User Tier
    │   └─► For each s_defaultUserSearchPaths entry:
    │       1. expandEnvVars(raw) → resolves %APPDATA%, ${XDG_CONFIG_HOME}, etc.
    │       2. No suffix rules (used as-is after expansion)
    │       3. Already absolute, just canonicalize
    │       4. Create ResourceLocation with tier=User
    │
    └─► Result: QList<ResourcePathElement>
        Ordered: Installation → Machine → User
        Each element = { ResourceLocation, ResourceTier }
```

## Detailed Steps by Tier

### Installation Tier (Relative to Application)

**Input:** `s_defaultInstallSearchPaths` (compile-time constants)

| Platform | Raw Template | After Expansion | After Suffix | Joined with App Path | Final |
|----------|--------------|-----------------|--------------|---------------------|-------|
| Windows | `%PROGRAMFILES%/` | `C:/Program Files/` | `C:/Program Files/openscad` | `C:/Program Files/openscad` | Canonical |
| Windows | `.` | `.` | `.` | `C:/MyApp` | Canonical |
| Windows | `../share/` | `../share/` | `../share/openscad (Nightly)` | `C:/MyApp/share/openscad (Nightly)` | Canonical |
| macOS | `../Resources` | `../Resources` | `../Resources` | `/Applications/MyApp.app/Contents/Resources` | Canonical |
| Linux | `../share/` | `../share/` | `../share/openscad` | `/usr/local/share/openscad` | Canonical |

**Suffix Rule Logic:**
```cpp
if (raw.endsWith("/")) {
    // Share-style base → append app folder + suffix
    result = base + "openscad" + m_suffix;
} else {
    // Direct path → use as-is
    result = base;
}
```

**Why suffix only for install?**  
Per OpenSCAD design: nightly/dev builds get suffixed share dirs (`../share/openscad (Nightly)`), but user/machine paths stay unsuffixed (`OpenSCAD` folder name, not `OpenSCAD (Nightly)`).

### Machine Tier (System-Wide)

**Input:** `s_defaultMachineSearchPaths` (compile-time constants)

| Platform | Raw Template | After Expansion | Final |
|----------|--------------|-----------------|-------|
| Windows | `%PROGRAMDATA%/ScadTemplates` | `C:/ProgramData/ScadTemplates` | Canonical |
| Windows | `C:/ProgramData/` | `C:/ProgramData/` | Canonical |
| macOS | `/Library/Application Support/ScadTemplates` | Same | Canonical |
| Linux | `/usr/share/` | Same | Canonical |
| Linux | `/usr/local/share/` | Same | Canonical |

**Notes:**
- Paths are already absolute (system-wide locations)
- No suffix rules
- Env vars like `%PROGRAMDATA%` expanded at runtime

### User Tier (Per-User)

**Input:** `s_defaultUserSearchPaths` (compile-time constants)

| Platform | Raw Template | After Expansion | Final |
|----------|--------------|-----------------|-------|
| Windows | `%APPDATA%/ScadTemplates` | `C:/Users/Jeff/AppData/Roaming/ScadTemplates` | Canonical |
| Windows | `%LOCALAPPDATA%/ScadTemplates` | `C:/Users/Jeff/AppData/Local/ScadTemplates` | Canonical |
| macOS | `${HOME}/Library/Application Support/ScadTemplates` | `/Users/jeff/Library/Application Support/ScadTemplates` | Canonical |
| Linux | `${XDG_CONFIG_HOME}/cppsnippets` | `/home/jeff/.config/cppsnippets` | Canonical |
| Linux | `${HOME}/.config/cppsnippets` | `/home/jeff/.config/cppsnippets` | Canonical |

**Notes:**
- Home-directory templates using `${VAR}` or `%VAR%`
- No suffix rules
- Expansion happens at runtime (not compile-time)

## Runtime Environment Variable Expansion

All tiers go through `ResourcePaths::expandEnvVars()`:

```cpp
QString expandEnvVars(const QString& path) const {
    // 1. Start with user overrides (m_envVars from QSettings)
    // 2. Fall back to system environment
    // 3. Support both ${VAR} and %VAR% syntax
    // 4. Regex: \$\{([^}]+)\}|%([^%]+)%
}
```

**User override precedence:**  
If `%PROGRAMDATA%` is in `m_envVars`, that value is used instead of the system environment value. This allows testing and per-user customization.

## Canonicalization

After expansion and joining, paths are canonicalized:

```cpp
QFileInfo fi(expanded);
QString canonical = fi.canonicalFilePath();
if (canonical.isEmpty()) canonical = fi.absoluteFilePath();
```

**Purpose:**
- Resolve `..` and `.` segments
- Follow symlinks to real paths
- Deduplicate equivalent paths (e.g., `/usr/local/share` vs `/usr/local/share/`)

**Fallback:**  
If path doesn't exist, `canonicalFilePath()` returns empty → fall back to `absoluteFilePath()` (still valid for creation).

## ResourcePathElement Structure

Each location in the returned list is:

```cpp
struct ResourcePathElement {
    ResourceLocation location;  // path, displayName, exists, isWritable, etc.
    ResourceTier tier;          // Installation, Machine, or User
};
```

**Why explicit tier tags?**  
- Preserves tier membership independent of list position
- Allows filtering by tier: `elementsByTier(ResourceTier::User)`
- Simplifies preferences UI (group by tier for display)

## Integration with ResourceLocationManager

### Current (Tier-Based) Approach
```cpp
// Separate methods rebuild lists each call
QVector<ResourceLocation> installLocs = findSiblingInstallations();
QVector<ResourceLocation> machineLocs = defaultMachineLocations();
QVector<ResourceLocation> userLocs = defaultUserLocations();

// When scanning: combine and deduplicate manually
```

### New (Location-Based) Approach
```cpp
// Single call at init
const auto elements = m_resourcePaths.defaultElements();

// Store as flat list (order = search priority)
m_availableLocations.clear();
for (const auto& elem : elements) {
    m_availableLocations.append(elem.location);
}

// Load enabled state from QSettings
QStringList enabledPaths = m_settings->value("Resources/EnabledPaths").toStringList();
for (auto& loc : m_availableLocations) {
    loc.isEnabled = enabledPaths.contains(loc.path);
}

// When scanning: iterate once, skip disabled
for (const auto& loc : m_availableLocations) {
    if (loc.isEnabled && loc.exists) {
        scanLocation(loc, resourceType);
    }
}
```

## Benefits of Location-Based Approach

| Aspect | Tier-Based | Location-Based |
|--------|------------|----------------|
| **Search Order** | Implicit (method call order) | Explicit (list order) |
| **Env Expansion** | Scattered across methods | Centralized in `expandEnvVars()` |
| **Suffix Rules** | Only in `appSearchPaths()` | Uniformly applied in `defaultElements()` |
| **Tier Membership** | Inferred from method | Explicit `ResourceTier` tag |
| **Discovery** | Multiple passes (per-tier) | Single pass (ordered list) |
| **Caching** | Per-tier caches | Single cache |
| **Enabled/Disabled** | Per-tier settings keys | Flat list with `isEnabled` flag |

## Example: Windows with Nightly Build

**Application path:** `C:\Program Files\OpenSCAD (Nightly)\`  
**Suffix:** ` (Nightly)`

**defaultElements() output:**

```
Installation:
  C:\Program Files\openscad (Nightly)        (from %PROGRAMFILES%/)
  C:\Program Files\OpenSCAD (Nightly)        (from .)
  C:\Program Files\OpenSCAD (Nightly)\share\openscad (Nightly)  (from ../share/)
  C:\Program Files                            (from ..)

Machine:
  C:\ProgramData\ScadTemplates                (from %PROGRAMDATA%/ScadTemplates)
  C:\ProgramData                              (from C:/ProgramData/)

User:
  C:\Users\Jeff\AppData\Roaming\ScadTemplates (from %APPDATA%/ScadTemplates)
  C:\Users\Jeff\AppData\Local\ScadTemplates   (from %LOCALAPPDATA%/ScadTemplates)
  C:\Program Files\OpenSCAD (Nightly)         (from .)
  C:\Program Files                            (from ../)
```

**Deduplication:**  
Canonicalization collapses equivalent paths, so the final list has no duplicates.

## Usage in Preferences UI

Preferences can display locations grouped by tier:

```cpp
const auto elements = resourcePaths.defaultElements();

// Group by tier for display
QMap<ResourceTier, QVector<ResourceLocation>> byTier;
for (const auto& elem : elements) {
    byTier[elem.tier].append(elem.location);
}

// Show three tabs: Installation | Machine | User
for (const auto& [tier, locations] : byTier.toStdMap()) {
    addTab(tierName(tier), locations);
}
```

**"Restore Defaults":**  
Re-call `defaultElements()` and reset enabled state to all-enabled.

## Future: Sibling Discovery

Currently `findSiblingInstallations()` is separate. It should be integrated:

```cpp
QList<ResourcePathElement> defaultElements() const {
    // ... existing install/machine/user logic ...
    
    // Add siblings (other OpenSCAD installs)
    const auto siblings = findSiblingInstallations();
    for (const auto& sib : siblings) {
        ResourceLocation loc(sib.path, sib.displayName, sib.description, 
                            ResourceTier::Installation);
        result.append(ResourcePathElement(loc, ResourceTier::Installation));
    }
    
    return result;
}
```

This way, all discovery is in one place.

## Summary

`defaultElements()` is the single source of truth for location-based discovery:
- Expands all env vars at runtime
- Applies suffix rules for install tier
- Resolves paths relative to application
- Canonicalizes to eliminate duplicates
- Tags each location with its tier
- Returns ordered list (search priority preserved)

This replaces scattered tier-based methods and centralizes all OS/filesystem logic in `ResourcePaths`.

## Current State Analysis

### What’s Done
- Centralized env expansion: `expandEnvVars()` supports `${VAR}` and `%VAR%` with user override precedence.
- Location-based list: `defaultElements()` returns ordered, tier-tagged `ResourcePathElement`s.
- Suffix handling: Applied only for installation tier share-style bases; machine/user paths remain unsuffixed.
- Canonicalization: Uses `canonicalFilePath()` with fallback to `absoluteFilePath()` to normalize and deduplicate.
- Legacy cleanup: `OPENSCADPATH` references removed by design; XDG/home env handling retained.

### What’s Pending
- Wiring: `ResourceLocationManager` and scanners/models to consume `defaultElements()` directly (single-pass discovery).
- Siblings: Integrate `findSiblingInstallations()` into `defaultElements()` for complete install-tier discovery.
- Settings: Define and persist enable/disable state; ensure startup applies overrides to defaults.
- Tests: Unit tests for `expandEnvVars()` and `defaultElements()` paths across platforms; verify suffix logic and dedup.

### Enable/Disable Design Proposal
- Storage: Persist enabled locations in QSettings under `Resources/EnabledPaths` (list of canonical paths).
- Defaults: On first run, all discovered locations are enabled; “Restore Defaults” re-enables all.
- UI: Show tier-grouped lists with checkboxes; toggling updates QSettings immediately and triggers rescan.
- Scanning: Skip disabled or non-existent locations; rebuild inventory on change to reflect state.
- Migration: If a path disappears/changes, drop stale entries during canonicalization before applying settings.

### Risks & Mitigations
- Env variance: Different shells/envs may yield unexpected expansions → user overrides in QSettings allow controlled testing.
- Duplicate paths: Multiple templates can resolve to the same directory → canonicalization + set-based dedup guards.
- Nightly suffix: Ensure only install-tier share bases receive suffix; add tests to prevent suffix bleed into user/machine tiers.

### Next Actions
- Replace tier-based assembly in `ResourceLocationManager` with `defaultElements()` ingestion.
- Implement enable-state persistence and UI hooks; emit signals to rescan when toggled.
- Add sibling discovery into `defaultElements()` and verify search order.
- Write focused unit tests for env expansion, suffix application, canonicalization, and deduplication.
