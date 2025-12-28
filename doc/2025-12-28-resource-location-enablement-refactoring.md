# Resource Location Enablement Refactoring

**Date:** December 28-29, 2025  
**Project:** cppsnippets (OpenSCAD Resource Management)  
**Components:** ResourceLocationManager, PreferencesDialog  
**Status:** Phase 1 Complete - Unified Discovery Architecture

---

## Refactoring Strategy

### Phase 1: Unified Tier Discovery ✅ COMPLETE

**Goal:** Create single discovery function that returns all locations tagged by tier

**Rationale:**
- Tiers are conceptual groupings, not file system structure
- Discovery should operate on a single list of locations
- Tier tag used only for UI grouping and applying tier-specific disabled lists
- Eliminates duplicated discovery logic across tiers

**Implementation:**
- Added `discoverAllLocations()` - single entry point for ALL location discovery
- Added tier-specific helpers: `discoverInstallationLocations()`, `discoverMachineLocations()`, `discoverUserLocations()`
- Each helper returns locations with proper tier tag
- Status checking (`exists`, `hasResourceFolders`) happens once at the end
- Platform-specific logic remains in helpers (legitimate tier differences)

**Benefits:**
- ✅ One entry point for complete location discovery
- ✅ Tier tags are simple enum markers, not separate code paths
- ✅ Status checking centralized
- ✅ Easy to apply disabled-only model in Phase 2

### Phase 2: Disabled-Only Model (IN PROGRESS)

**Goal:** Remove all enabled path tracking, use only disabled lists

**Changes Required:**
1. Remove enabled path persistence:
   - `KEY_ENABLED_PATHS`
   - `KEY_SIBLING_PATHS`
   - `KEY_MACHINE_PATHS`
   - `KEY_USER_PATHS`
   - `loadEnabledPaths()`
   - `saveEnabledPaths()`
   - `setEnabledSiblingPaths()`
   - `updateEnabledForTier()`

2. Keep only disabled path tracking:
   - `KEY_DISABLED_INSTALLATION`
   - `KEY_DISABLED_MACHINE`
   - `KEY_DISABLED_USER`
   - `disabledPathsForTier()`
   - `setDisabledPathsForTier()`

3. Simplified enable state logic:
```cpp
QVector<ResourceLocation> availableLocations() const {
    // 1. Discover everything
    QVector<ResourceLocation> all = discoverAllLocations();
    
    // 2. Load disabled sets per tier
    QSet<QString> disabledInstall = disabledPathsForTier(ResourceTier::Installation);
    QSet<QString> disabledMachine = disabledPathsForTier(ResourceTier::Machine);
    QSet<QString> disabledUser = disabledPathsForTier(ResourceTier::User);
    
    // 3. Apply disabled state
    for (auto& loc : all) {
        QSet<QString> disabled = (loc.tier == Installation ? disabledInstall :
                                  loc.tier == Machine ? disabledMachine : disabledUser);
        QString canonical = QFileInfo(loc.path).canonicalFilePath();
        loc.isEnabled = !disabled.contains(canonical);
    }
    
    return all;
}
```

### Phase 3: UI Display Rules (PENDING)

**Goal:** Visual feedback based on state + content

**Rules:**
- Disabled → gray background, dark gray text, unchecked
- Enabled + no content → gray styling + "(no content)" label
- Enabled + has content → black on white, checked

---

## Overview

This document describes a comprehensive refactoring of the resource location discovery and enablement system to support user-configurable enable/disable state for all discovered locations across all tiers (Installation, Machine, User), while ensuring newly discovered locations default to enabled and persist user preferences.

---

## User Requirements

### Primary Objectives

1. **Default-Enable New Discoveries**: All newly discovered resource locations should default to enabled state
2. **User-Disableable Locations**: Any location (including primary installation) can be disabled by the user
3. **No Forced Enablement**: Remove special-case logic that forces primary installation to always be enabled
4. **Include Siblings**: Sibling installations should be discovered and shown in preferences
5. **Persist User Choices**: Enable/disable state must persist across sessions via QSettings
6. **UI Consistency**: PreferencesDialog and main window must reflect ResourceLocationManager state
7. **Single-Line Display**: Location labels show "Name - Path" format

### Clarifications

- **No "Seen" Marking Required**: The enabled/disabled state in QSettings is sufficient for tracking which locations have been discovered and configured; no separate "seen" flag is needed
- **Siblings Allowed**: Multiple installations (siblings) can coexist and be selectively enabled/disabled
- **Per-Tier Persistence**: Each tier (Installation, Machine, User) maintains separate enabled and disabled path lists

---

## Functional Specifications

### Location Discovery

#### Qt Application Location Detection

The system uses **`QCoreApplication::applicationDirPath()`** to determine the running application's location. This is the authoritative source for finding the primary installation's resource directory.

**Key Function:**
```cpp
QString ResourceLocationManager::findInstallationResourceDir() const {
    QString appDir = QCoreApplication::applicationDirPath();
    // Search for resource subdirectories relative to appDir
    // ...
}
```

#### Discovery Process

1. **Installation Tier**:
   - Primary installation: Located via `QCoreApplication::applicationDirPath()`
   - Sibling installations: Discovered by searching platform-specific default installation paths
   - All discovered installations default to enabled unless explicitly disabled in settings

2. **Machine Tier**:
   - Predefined platform-specific system-wide locations
   - Configurable via JSON config files
   - XDG_DATA_DIRS environment variable entries (POSIX/Mac)

3. **User Tier**:
   - User-specific directories (e.g., AppData, .local/share)
   - Configurable via JSON config files
   - XDG_DATA_HOME environment variable entries (POSIX/Mac)

### Settings Persistence

#### QSettings Keys

**Enabled Paths (Per-Tier):**
- `KEY_SIBLING_PATHS` - Installation tier enabled paths
- `KEY_MACHINE_PATHS` - Machine tier enabled paths
- `KEY_USER_PATHS` - User tier enabled paths

**Disabled Paths (Per-Tier):**
- `KEY_DISABLED_INSTALLATION` - Installation tier disabled paths
- `KEY_DISABLED_MACHINE` - Machine tier disabled paths
- `KEY_DISABLED_USER` - User tier disabled paths

#### Settings Operation During Discovery

**Loading Phase:**
1. Discover all available locations (via filesystem search, config files, environment variables)
2. Load enabled/disabled path lists from QSettings for each tier
3. Apply enabled state:
   - If path is in disabled set → mark as disabled
   - If path is in enabled set → mark as enabled
   - Otherwise → default to enabled (for newly discovered locations)

**Display Phase:**
1. PreferencesDialog queries `ResourceLocationManager::availableMachineLocations()` and `availableUserLocations()`
2. Manager returns locations with `isEnabled` flag already set based on settings
3. UI displays checkboxes reflecting the manager's state
4. Non-existent locations are shown as disabled but remain in the list for reference

**Saving Phase:**
1. Collect enabled and disabled paths from UI
2. Persist both enabled and disabled lists to QSettings per tier
3. Manager uses these lists on next load to restore user preferences

### Enable/Disable Logic

#### No Forced Enablement

Previously, the primary installation was forced to enabled state regardless of user preference:

```cpp
// OLD BEHAVIOR (removed):
if (primaryPath == effectiveInstallDir) {
    loc.isEnabled = true;  // Force enable - NOT ALLOWED ANYMORE
}
```

New behavior allows all locations to respect user disable preference:

```cpp
// NEW BEHAVIOR:
void ResourceLocationManager::applyEnabledState(
    QVector<ResourceLocation>& locs, 
    resourceInfo::ResourceTier tier) const 
{
    const QStringList enabledList = enabledPathsForTier(tier);
    const QStringList disabledList = disabledPathsForTier(tier);
    const QSet<QString> enabledSet(enabledList.cbegin(), enabledList.cend());
    const QSet<QString> disabledSet(disabledList.cbegin(), disabledList.cend());
    
    for (auto& loc : locs) {
        QString canonical = canonicalPath(loc.path);
        if (disabledSet.contains(canonical)) {
            loc.isEnabled = false;  // Respect user disable
        } else if (enabledSet.contains(canonical)) {
            loc.isEnabled = true;
        }
        // else: default enabled (newly discovered)
    }
}
```

#### Canonical Path Matching

All path comparisons use canonicalized paths to handle symlinks, relative paths, and case-sensitivity:

```cpp
auto canonical = [](const QString& path) {
    QFileInfo fi(path);
    QString canon = fi.canonicalFilePath();
    return canon.isEmpty() ? fi.absoluteFilePath() : canon;
};
```

---

## Implementation Details

### ResourceLocationManager Changes

#### Header Additions (`resourceLocationManager.h`)

```cpp
// Disabled path keys for QSettings
static constexpr const char* KEY_DISABLED_INSTALLATION = "Paths/disabled_installation";
static constexpr const char* KEY_DISABLED_MACHINE = "Paths/disabled_machine";
static constexpr const char* KEY_DISABLED_USER = "Paths/disabled_user";

// Per-tier enabled/disabled path getters
QStringList enabledPathsForTier(resourceInfo::ResourceTier tier) const;
QStringList disabledPathsForTier(resourceInfo::ResourceTier tier) const;
void setDisabledPathsForTier(resourceInfo::ResourceTier tier, const QStringList& paths) const;

// Apply enabled state to discovered locations
void applyEnabledState(QVector<ResourceLocation>& locs, resourceInfo::ResourceTier tier) const;

// Updated collectTier signature to accept disabled set
QVector<ResourceLocation> collectTier(
    const QList<ResourcePathElement>& elements,
    resourceInfo::ResourceTier tier,
    const QSet<QString>& enabledSet,
    const QSet<QString>& disabledSet,
    bool includeDisabled) const;

// Setters for disabled paths
void setDisabledInstallationPaths(const QStringList& paths);
void setDisabledMachineLocations(const QStringList& paths);
void setDisabledUserLocations(const QStringList& paths);

// Getter for disabled installation paths
QStringList disabledInstallationPaths() const;
```

#### Implementation Changes (`resourceLocationManager.cpp`)

1. **Removed Forced Primary Install Enablement** in `loadEnabledPaths()`:
   - Deleted logic that always enabled primary installation path
   - Allows user to disable any location

2. **Added Per-Tier Helper Functions**:
   - `enabledPathsForTier()` - retrieves enabled paths for specific tier
   - `disabledPathsForTier()` - retrieves disabled paths for specific tier
   - `setDisabledPathsForTier()` - persists disabled paths for tier

3. **Added `applyEnabledState()` Function**:
   - Applies enabled/disabled state to location vectors after discovery
   - Called by `loadMachineLocationsConfig()` and `loadUserLocationsConfig()`

4. **Updated `collectTier()` Function**:
   - Now accepts both `enabledSet` and `disabledSet` parameters
   - Checks disabled set first (takes precedence)
   - Falls back to enabled set, then default enabled for new discoveries

5. **Updated `buildEnabledTieredLocations()`**:
   - Builds enabled/disabled sets for all three tiers
   - Passes both sets to `collectTier()` for each tier

6. **Updated Location Loading**:
   - `loadMachineLocationsConfig()` calls `applyEnabledState()` after loading
   - `loadUserLocationsConfig()` calls `applyEnabledState()` after loading
   - Default location builders pass empty disabled sets (all default-enabled)

### PreferencesDialog Changes (`preferencesdialog.cpp`)

#### Installation Tab Loading

```cpp
// Load enabled siblings and disabled install paths from settings
const QSet<QString> enabledSiblings = /* from manager */;
const QSet<QString> disabledInstall = /* from manager */;

// Primary installation
if (!installDir.isEmpty()) {
    ResourceLocation loc(installDir, tr("Application Resources"), ...);
    loc.isEnabled = !disabledInstall.contains(canonical(installDir));
    // ...
}

// Sibling installations (no deduplication - user may disable any)
for (auto& sibling : siblings) {
    const QString key = canonical(sibling.path);
    if (disabledInstall.contains(key)) {
        sibling.isEnabled = false;
    } else if (enabledSiblings.contains(sibling.path)) {
        sibling.isEnabled = true;
    } // else keep default (typically true)
    installLocations.append(sibling);
}
```

#### Saving Changes

```cpp
void PreferencesDialog::saveSettings() {
    // Installation tier
    QStringList enabledSiblings;
    QStringList disabledInstall;
    for (const auto& loc : installLocs) {
        if (isPrimaryOrUserInstall(loc.path)) {
            if (!loc.isEnabled) {
                disabledInstall.append(loc.path);
            }
        } else {
            if (loc.isEnabled) {
                enabledSiblings.append(loc.path);
            } else {
                disabledInstall.append(loc.path);
            }
        }
    }
    m_manager->setEnabledSiblingPaths(enabledSiblings);
    m_manager->setDisabledInstallationPaths(disabledInstall);
    
    // Machine tier - collect enabled and disabled
    QStringList disabled;
    for (const auto& loc : machineLocs) {
        if (!loc.isEnabled && !loc.path.isEmpty()) {
            disabled.append(loc.path);
        }
    }
    m_manager->setDisabledMachineLocations(disabled);
    
    // User tier - collect enabled and disabled
    // (similar to machine)
}
```

---

## Key Design Decisions

### 1. No "Seen" Flag Required

**Rationale:** The enabled/disabled state stored in QSettings provides sufficient information:
- Locations in enabled list = discovered and explicitly enabled
- Locations in disabled list = discovered and explicitly disabled
- Locations in neither list = newly discovered, default to enabled

This approach:
- Simplifies the data model
- Reduces QSettings key count
- Makes the system more maintainable
- Avoids redundant state tracking

### 2. Disabled Set Takes Precedence

When determining a location's enabled state:
1. Check disabled set first (explicit user disable)
2. Check enabled set second (explicit user enable)
3. Default to enabled (newly discovered)

This ensures user's disable action is always respected, even if the path appears in both lists due to migration or corruption.

### 3. Separate Per-Tier Persistence

Each tier maintains independent enabled/disabled lists because:
- Tiers have different discovery mechanisms
- User may want different policies per tier
- Simplifies restore-to-defaults operation
- Matches the three-tab UI structure

### 4. Canonical Path Normalization

All paths are canonicalized before storage and comparison to:
- Handle symlinks correctly
- Resolve relative paths
- Eliminate duplicates from case-sensitivity issues
- Ensure consistent cross-platform behavior

---

## Testing Recommendations

1. **New Installation Discovery**:
   - Verify new locations default to enabled
   - Confirm they appear checked in preferences

2. **User Disable Primary**:
   - Disable primary installation in preferences
   - Confirm it remains disabled after restart
   - Verify resources from that location are not loaded

3. **Sibling Handling**:
   - Install multiple OpenSCAD versions
   - Verify all appear in Installation tab
   - Test selective enable/disable

4. **Settings Migration**:
   - Test with legacy settings files
   - Verify enabled paths migrate correctly
   - Confirm disabled lists are created

5. **Persistence**:
   - Enable/disable various locations
   - Restart application
   - Verify all states restored correctly

---

## Migration Notes

### From Previous System

Old system used:
- Single unified `KEY_ENABLED_PATHS` list
- Forced primary installation enablement
- No disabled path tracking

New system uses:
- Per-tier enabled keys (`KEY_SIBLING_PATHS`, `KEY_MACHINE_PATHS`, `KEY_USER_PATHS`)
- Per-tier disabled keys (`KEY_DISABLED_*`)
- No forced enablement

**Backward Compatibility:**
- `loadEnabledPaths()` falls back to legacy unified key if per-tier keys are empty
- Missing disabled keys treated as empty lists (all default-enabled)

---

## Related Files

- `src/platformInfo/resourceLocationManager.h`
- `src/platformInfo/resourceLocationManager.cpp`
- `src/gui/preferencesdialog.cpp`
- `src/gui/preferencesdialog.h`
- `src/resInventory/ResourceLocation.h`

---

## Conclusion

This refactoring establishes a robust, user-friendly system for managing resource location enable/disable state. The use of Qt's native application path detection, combined with per-tier settings persistence and proper canonical path handling, ensures reliable operation across platforms and user configurations. The removal of forced-enablement logic empowers users with full control over their resource discovery paths.
