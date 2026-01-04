# Phase 1 Step 5: Environment Variable Expansion

**Date:** January 4, 2026  
**Status:** ✅ Complete - Build Successful

---

## Summary

Completed **Step 1.5: Environment Variable Expansion**, the final step of Phase 1 Type Consolidation. Implemented clean, Qt-based environment variable expansion for default search path templates.

---

## What Was Done

### Added Methods to ResourcePaths

**Header (`resourcePaths.hpp`):**
```cpp
// Resolved search paths with environment variables expanded to absolute paths
static QStringList resolvedInstallSearchPaths();
static QStringList resolvedMachineSearchPaths();
static QStringList resolvedUserSearchPaths();

// Environment variable expansion helper
static QString expandEnvVars(const QString& path);
```

**Implementation (`resourcePaths.cpp`):**

1. **Three Tier-Specific Resolved Methods:**
   - `resolvedInstallSearchPaths()` - expands defaults for Installation tier
   - `resolvedMachineSearchPaths()` - expands defaults for Machine tier
   - `resolvedUserSearchPaths()` - expands defaults for User tier
   - All return `QStringList` with fully expanded paths

2. **Environment Variable Expansion Helper:**
   - `expandEnvVars(const QString& path)` - static private method
   - Supports both `${VAR}` and `%VAR%` syntax (Unix and Windows styles)
   - Uses `QProcessEnvironment::systemEnvironment()` for safe environment access
   - Undefined variables expand to empty string (graceful fallback)
   - Uses regex pattern matching: `R"(\$\{([^}]+)\}|%([^%]+)%)"`
   - Builds result string efficiently without multiple allocations

### Design Principles Applied

**Pure Qt Facilities:**
- ✅ No custom environment variable registry
- ✅ No QSettings persistence (not needed in Phase 1)
- ✅ Only `QProcessEnvironment` from Qt Core
- ✅ Only `QRegularExpression` for pattern matching
- ✅ Minimal, focused implementation

**ResourcePaths as Source of Truth:**
- Compile-time defaults with env var templates (immutable)
- Runtime expansion produces absolute paths (mutable as-needed)
- Tier-specific accessors separate concerns
- No side effects or state modification

**Clean Architecture:**
- Separation between "default templates" and "resolved paths"
- Clear method naming (prefix `resolved*` indicates expansion applied)
- Self-documenting code with detailed comments
- Single responsibility: expand environment variables

---

## Examples

### Windows Expansion
```
Template:  "%APPDATA%/openscad"
System:    %APPDATA% = "C:\Users\Jeff\AppData\Roaming"
Result:    "C:/Users/Jeff/AppData/Roaming/openscad"
```

### Linux Expansion
```
Template:  "${HOME}/.config/openscad"
System:    HOME = "/home/jeff"
Result:    "/home/jeff/.config/openscad"
```

### macOS Expansion
```
Template:  "${HOME}/Library/Application Support"
System:    HOME = "/Users/jeff"
Result:    "/Users/jeff/Library/Application Support"
```

---

## Build Status

✅ **Compilation:** Successful  
✅ **scadtemplates.dll:** Built  
✅ **scadtemplates.exe:** Built  
✅ **No compiler errors or warnings**

---

## Phase 1 Completion Status

**All 5 Steps Complete:**

| Step | Task | Status |
|------|------|--------|
| 1.1 | Fix ResourceTypeInfo.hpp | ✅ Complete |
| 1.2 | Consolidate ResourceTier | ✅ Complete |
| 1.3 | Remove ResourceType duplicates | ✅ Complete |
| 1.4 | Update all includes | ✅ Complete |
| 1.5 | Environment variable expansion | ✅ Complete |

**Phase 1 Objective - ACHIEVED:**
> Single source of truth for enums and metadata, with immutable compile-time defaults that can be expanded to runtime absolute paths.

---

## What This Enables

Phase 1 completion provides a solid foundation for Phase 2:

1. **Defaults are now usable** - can be expanded to actual file paths
2. **Type consolidation is complete** - no duplicate enums or definitions
3. **Clean separation of concerns:**
   - ResourcePaths: "Where to look" (immutable + expansion)
   - ResourceLocationManager/Registry: "What was found" (discovery + persistence)
   - ResourceDiscovery: "How to find" (scanning logic)

4. **Ready for next phases:**
   - Phase 2: Redesign class responsibilities (discovery, registry, etc.)
   - Phase 3: Integration and testing

---

## Code Review Notes

**Strengths:**
- Uses only Qt Core APIs (portable across platforms)
- Supports both Unix (`${VAR}`) and Windows (`%VAR%`) syntax
- Regex pattern handles both capture groups cleanly
- Result string built efficiently (single allocation, append operations)
- Undefined variables fail gracefully (expand to empty string)

**No Overcomplexity:**
- ❌ Rejected: Custom env var registry (not needed in Phase 1)
- ❌ Rejected: QSettings persistence (belongs in Phase 2 Registry)
- ❌ Rejected: Process environment modification (read-only is sufficient)
- ✅ Kept: Simple, focused, Qt-native implementation

---

## Next Steps

Ready to proceed to **Phase 2: Redesign Class Responsibilities**

Phase 2 will:
- Define ResourceDiscovery class (find resources on disk)
- Repurpose ResourceLocationManager → ResourceRegistry (track found + enabled locations)
- Integrate with QSettings for persistence
- Implement drag-drop resource folder addition
- Create resource inventory model/view
