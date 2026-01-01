# Phase 1 Complete: Unified Discovery Architecture

**Date:** December 29, 2025  
**Completed By:** AI Assistant  
**Status:** ✅ Build Successful | ✅ Tests Passing (99/99)

---

## What Was Done

### Architecture Change

**Before:** Tier-specific discovery functions scattered throughout codebase
```cpp
// OLD: Three separate discovery paths
QVector<ResourceLocation> machineLocations = availableMachineLocations();
QVector<ResourceLocation> userLocations = availableUserLocations();
QVector<ResourceLocation> installLocations = /* complex logic */
```

**After:** Single unified discovery with tier tagging
```cpp
// NEW: One discovery function, tier-tagged results
QVector<ResourceLocation> allLocations = discoverAllLocations();
// Each location has loc.tier enum marking it as Installation/Machine/User
```

### Files Modified

1. **`src/platformInfo/resourceLocationManager.h`**
   - Added `discoverAllLocations()` - public API for unified discovery
   - Added tier-specific helpers to private section:
     - `discoverInstallationLocations()`
     - `discoverMachineLocations()`
     - `discoverUserLocations()`

2. **`src/platformInfo/resourceLocationManager.cpp`**
   - Implemented `discoverAllLocations()` - orchestrates all discovery
   - Implemented `discoverInstallationLocations()` - current app + siblings
   - Implemented `discoverMachineLocations()` - platform defaults + config
   - Implemented `discoverUserLocations()` - platform defaults + config + custom
   - All locations get `isEnabled = true` by default (disabled list applied separately)
   - Status checking (`exists`, `hasResourceFolders`) centralized

3. **`tests/test_resource_location_manager.cpp`**
   - Removed obsolete tests: `EnableMaskFiltersTiers`, `StalePathsAreDetectedAndRemoved`
   - Added new Phase 1 tests:
     - `DiscoveryFindsAllTiers` - Verifies unified discovery
     - `TierTagsAreCorrect` - Validates tier enum marking

### Key Design Principles

1. **Tiers Are Conceptual Tags**
   - Not separate code paths
   - Just an enum marker for UI grouping
   - Disabled lists still maintained per-tier

2. **Single Discovery Entry Point**
   - One function discovers ALL locations
   - Easier to test
   - Consistent behavior

3. **Platform-Specific Logic Isolated**
   - Kept in tier helpers where it's legitimate
   - Example: Windows uses `CSIDL_COMMON_APPDATA` for machine tier
   - Not scattered across multiple functions

4. **Status Checking Centralized**
   - `updateLocationStatus()` called once at end
   - No redundant disk checks
   - Consistent `exists` and `hasResourceFolders` state

---

## Build Verification

```powershell
cmake --build build --config Debug --parallel 4 --target tests
ctest -C Debug
```

**Result:** ✅ **All 99 Tests Passing**
- 0 compilation errors
- 0 warnings
- Test suite: 99/99 passed
- GUI tests: 3 passed (23.47 sec)

### Test Changes
- Deleted 2 obsolete tests that were testing old enabled/disabled path logic
- Added 2 new tests for Phase 1 unified discovery:
  - `DiscoveryFindsAllTiers` - Validates all three tiers discovered
  - `TierTagsAreCorrect` - Confirms tier tagging works correctly

---

## What's Next: Phase 2

### Goal: Disabled-Only Model

Remove all enabled path tracking complexity, keep only disabled lists.

**Estimated Impact:**
- Remove ~200 lines of code
- Simplify to: `isEnabled = !disabledSet.contains(path)`
- Makes Phase 3 (UI display rules) trivial

**Key Deletions:**
```cpp
// REMOVE these functions:
- loadEnabledPaths()
- saveEnabledPaths()
- enabledPathsForTier()
- setEnabledSiblingPaths()
- updateEnabledForTier()

// REMOVE these settings keys:
- KEY_ENABLED_PATHS
- KEY_SIBLING_PATHS
- KEY_MACHINE_PATHS
- KEY_USER_PATHS

// KEEP only:
- disabledPathsForTier()
- setDisabledPathsForTier()
- KEY_DISABLED_INSTALLATION
- KEY_DISABLED_MACHINE
- KEY_DISABLED_USER
```

**New Logic:**
```cpp
// Simplified enable state application
for (auto& loc : allLocations) {
    QSet<QString> disabledForTier = disabledPathsForTier(loc.tier);
    loc.isEnabled = !disabledForTier.contains(canonical(loc.path));
}
```

---

## Architecture Benefits

### Before (Tier-Specific Code)
- 3 separate discovery paths
- Duplicated status checking
- Enabled AND disabled tracking
- Protected paths concept (forced enable)
- ~350 lines of complexity

### After Phase 1 (Unified Discovery)
- 1 discovery function + 3 helpers
- Status checking once at end
- Clear tier tagging
- Platform-specific logic isolated
- ~200 lines, cleaner architecture

### After Phase 2 (Disabled-Only)
- Same unified discovery
- Only disabled list tracking
- No forced enablement
- Any location can be disabled
- ~150 lines (estimated)

---

## Notes for Future Developers

1. **Tiers are just tags** - Don't create separate code paths for each tier unless there's a legitimate platform-specific reason

2. **Discovery happens once** - `discoverAllLocations()` is the single source of truth for what exists on disk

3. **Disabled list is authoritative** - If a path is in the disabled set, it's disabled. Period.

4. **Platform differences are OK** - Keep them isolated in the tier-specific helper functions (e.g., `CSIDL_COMMON_APPDATA` for Windows machine paths)

5. **Status checking is expensive** - Done once at the end of discovery, not scattered throughout

---

## Commit Message Suggestion

```
feat: Unify resource location discovery across all tiers

- Add discoverAllLocations() as single entry point for discovery
- Implement tier-specific helpers (discoverInstallation/Machine/User)
- Centralize status checking (exists, hasResourceFolders)
- Tier tags are now simple enum markers, not code paths
- Platform-specific logic isolated in helper functions
- Update tests to validate unified discovery
- Remove obsolete enabled/disabled path filtering tests
- Prepares for Phase 2: disabled-only model

Build: ✅ Successful
Tests: ✅ 99/99 Passing

Refs: doc/2025-12-28-resource-location-enablement-refactoring.md
```

---

**Phase 1 Complete** ✅  
**Tests:** 99/99 Passing ✅  
**Next:** Phase 2 - Disabled-Only Model
