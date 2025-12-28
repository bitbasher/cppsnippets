# Phase 1 Complete: Unified Discovery Architecture

**Date:** December 29, 2025  
**Completed By:** AI Assistant  
**Status:** ✅ Build Successful

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
cmake --build build --config Debug --parallel 4
```

**Result:** ✅ Build Successful
- No compilation errors
- No warnings
- All DLLs and executables generated
- Qt plugins copied correctly

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

## Testing Recommendations

Before Phase 2:
1. ✅ Verify build succeeds
2. ⏳ Test discovery finds all locations
3. ⏳ Verify tier tags are correct
4. ⏳ Check status flags (`exists`, `hasResourceFolders`)
5. ⏳ Ensure duplicates are removed

After Phase 2:
1. Test disabled list functionality
2. Verify newly discovered locations default enabled
3. Test persistence across restarts
4. Validate UI shows correct state

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
- Prepares for Phase 2: disabled-only model

Refs: doc/2025-12-28-resource-location-enablement-refactoring.md
```

---

**Phase 1 Complete** ✅  
**Next:** Phase 2 - Disabled-Only Model
