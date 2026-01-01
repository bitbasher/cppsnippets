# Single-Pipeline Resource Architecture

## Goals
- Consolidate to one discovery + scanning pipeline.
- Centralize OS/filesystem location intelligence under `platformInfo`.
- Make `ResourcePaths` the home for build suffix and env vars.
- Keep tier management and QSettings integration in `ResourceLocationManager`.

## Current State
- `platformInfo/ResourcePaths`: default search paths, resource types, user/machine path helpers, env var expansion.
- `platformInfo/ResourceLocationManager`: three-tier locations (Installation/Machine/User), sibling discovery, enabled/available sets, QSettings persistence, path resolution by resource type. Holds a `ResourcePaths` instance for env var management.
- `resInventory`: scanners and iterators build inventory from locations, plus UI models.
- `MainWindow`: calls `m_inventoryManager->buildInventory(*m_resourceManager)` and also `refreshInventory()` via a DirListing pipeline (causing duplicate sibling scans).

## Proposed Responsibilities

### ResourcePaths (platformInfo)
- **Suffix management**: `setSuffix()` and `suffix()` (build variant like " (Nightly)") for install-tier only.
- **Env vars registry**: `addEnvVar()`, `removeEnvVar()`, `envVarValue()`, `expandEnvVars()`; persist via `saveEnvVars(QSettings&)` and `loadEnvVars(QSettings&)`.
- **Default path constants**: `defaultInstallSearchPaths()`, `defaultMachineSearchPaths()`, `defaultUserSearchPaths()`; and expanded forms for machine/user.
- **Resource type metadata**: `resourceSubdirectory()`, `resourceExtensions()`, `allTopLevelResourceTypes()`.
- **No scanning, no QSettings keys beyond EnvVars. No sibling discovery.**

### ResourceLocationManager (platformInfo)
- **Tier management**: available + enabled locations for Installation/Machine/User; caching and status refresh.
- **Sibling discovery**: `findSiblingInstallations()` (Windows/macOS), cached and exposed.
- **Effective install path**: `effectiveInstallationPath()` prioritizing user-specified path then application-path.
- **Settings**: machine/user enabled paths, sibling enabled paths, user-specified install path; plus `saveEnvVarsToSettings()` delegating to `ResourcePaths`.
- **Resource resolution**: `resourcePathsForType(ResourceType)` returns ordered absolute directories using `ResourcePaths::resourceSubdirectory()` and env expansion.
- **No directory scanning; delegates to resInventory.**

### ResInventory (Scanning + Models)
- **Single scanner entrypoint** using `ResourceLocationManager`:
  - Build inventory for all resource types from `resourcePathsForType()`.
  - Populate `ResourceStore`, `TemplateTreeModel`, etc.
- **No OS-specific path logic; no sibling discovery.**

## Single Pipeline Flow
1. App startup:
   - `ResourceLocationManager` created; `setApplicationPath()` and `setSuffix()` applied.
   - `ResourceLocationManager` loads env vars from QSettings via its `ResourcePaths`.
   - Sibling discovery runs once (and caches).
2. Inventory build:
   - `ResourceInventoryManager::buildInventory(*ResourceLocationManager)` pulls `resourcePathsForType()` per type and scans.
3. UI:
   - Models (`TemplateTreeModel`, etc.) consume inventory results; `MainWindow` triggers `buildInventory()` as the sole refresh hook.

## API Touchpoints
- `ResourcePaths` (already present):
  - Keep `setSuffix()` and `expandedUserSearchPaths()` / `expandedMachineSearchPaths()`.
- `ResourceLocationManager`:
  - Use `ResourcePaths::expandEnvVars()` whenever reading defaults or config-defined locations.
  - Ensure `resourcePathsForType(ResourceType)` covers Installation → Machine → User with existence checks and subdir append.

## Migration Plan (Stepwise)
1. **Disable duplicate path discovery**: Remove the initial `refreshInventory()` call in `MainWindow` or make it delegate to `buildInventory()`.
2. **Unify template UI**: Make `TemplateTreeModel` populate from the inventory built by `ResourceInventoryManager` (no separate DirListing scan).
3. **Audit ResourceManager mentions**: Treat `resourceManager` as `ResourceLocationManager`; move any path logic from scanners/UI into `ResourceLocationManager`.
4. **Preferences wiring**: "Restore Defaults" uses `ResourcePaths::default*SearchPaths()`; enabled paths stored in `ResourceLocationManager` QSettings keys.

## Role of `resourceManager`
- In current code, `resourceManager` refers to `platformInfo::ResourceLocationManager`.
- It should be the single source of truth for locations and search order, while scanners just scan the paths it provides.
- If it "does too much": keep it focused on locations + settings; move any scanning or UI coupling out to resInventory and GUI.

## Next Steps
- Confirm `ResourceLocationManager::resourcePathsForType()` usage across scanners.
- Update `MainWindow` to call only `buildInventory()`.
- Align preferences to read defaults from `ResourcePaths` and enabled selections from `ResourceLocationManager`.
