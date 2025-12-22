# Platform Info Module

This module provides platform-specific path definitions and resource type metadata for locating OpenSCAD resources across different operating systems.

## Architecture Overview

### Two Distinct Concepts

The resource management system separates **immutable defaults** from **user selections**:

```
ResourcePaths (defaults)          ◄── Immutable, compile-time constants
       │
       ▼
ResourceIterator.scan()           ◄── Finds what actually exists on disk
       │
       ▼
ResLocMap / ResLocTree            ◄── User's working inventory (in resInventory module)
       │
       ├── User enables/disables via preferences
       ├── User adds custom locations
       └── Saved to QSettings
```

### 1. Default Search Paths (Immutable) - `ResourcePaths` class

- **Compile-time constants** per platform using `#ifdef`
- Used as the **"Restore Defaults"** source in preferences
- **User cannot modify these**
- These are the *potential* locations to look for resources

```cpp
// Platform-specific defaults - immutable
static const QStringList s_defaultInstallSearchPaths = {
#ifdef Q_OS_WIN
    QStringLiteral("."),
    QStringLiteral("../share/openscad"),
    QStringLiteral("..")
#elif defined(Q_OS_MACOS)
    QStringLiteral("../Resources"),
    QStringLiteral("../../.."),
    // ...
#else  // Linux/POSIX
    QStringLiteral("../share/openscad"),
    QStringLiteral("../../share/openscad"),
    // ...
#endif
};
```

### 2. Resource Inventory (User's Selection) - `resInventory` module

- Stored in `ResLocMap` (flat) or `ResLocTree` (hierarchical)
- **Populated by scanning** the default paths with `ResourceIterator`
- User can **enable/disable** locations via checkboxes in preferences
- User can **add custom locations** beyond the defaults
- **Persisted to QSettings** for the user's session

## Resource Tiers

Resources are organized into three tiers based on scope:

| Tier | Description | Writability |
|------|-------------|-------------|
| **Installation** | Built-in resources from app installation | Read-only |
| **Machine** | System-wide resources for all users | Admin-only |
| **User** | Personal resources in user's home directory | Writable |

## Classes

### ResourcePaths

Provides platform-specific search paths and resource type definitions.

**Key Responsibilities:**
- Define default search paths per platform (compile-time)
- Provide resource type metadata (subdirectory names, file extensions)
- Resolve user config and documents paths

**Key Methods:**

| Method | Purpose |
|--------|---------|
| `defaultInstallSearchPaths()` | Get immutable list of installation search paths |
| `defaultMachineSearchPaths()` | Get immutable list of machine-level search paths |
| `defaultUserSearchPaths()` | Get immutable list of user-level search paths |
| `allResourceTypes()` | Get metadata for all resource types |
| `resourceSubdirectory(type)` | Get subdirectory name for a resource type |
| `resourceExtensions(type)` | Get file extensions for a resource type |
| `userConfigBasePath()` | Get platform-specific user config directory |
| `userDocumentsPath()` | Get platform-specific user documents directory |

### ResourceTypeInfo

Metadata structure for each resource type.

| Field | Description |
|-------|-------------|
| `type` | ResourceType enum value |
| `subdirectory` | Folder name under resource directory (e.g., "fonts") |
| `description` | Human-readable description |
| `primaryExtensions` | Main file extensions (e.g., ".ttf", ".otf") |
| `attachmentExtensions` | Optional attachment extensions |

### ResourceType Enum

| Type | Subdirectory | Description |
|------|--------------|-------------|
| `Examples` | examples | Example .scad files |
| `Tests` | tests | Test .scad files |
| `Fonts` | fonts | TrueType/OpenType fonts |
| `ColorSchemes` | color-schemes | Editor color schemes |
| `Shaders` | shaders | OpenGL shader files |
| `Templates` | templates | Template files |
| `Libraries` | libraries | OpenSCAD library scripts |
| `Translations` | locale | Translation files |

## Platform-Specific Paths

### Installation Search Paths

Relative to application executable:

| Platform | Paths |
|----------|-------|
| **Windows** | `.`, `../share/openscad`, `..` |
| **macOS** | `../Resources`, `../../..`, `../../../..`, `..`, `../share/openscad` |
| **Linux** | `../share/openscad`, `../../share/openscad`, `.`, `..`, `../..` |

### User Config Paths

| Platform | Base Path |
|----------|-----------|
| **Windows** | `CSIDL_LOCAL_APPDATA` (e.g., `C:\Users\<user>\AppData\Local`) |
| **macOS** | `~/Library/Application Support` |
| **Linux** | `$XDG_CONFIG_HOME` or `~/.config` |

### User Documents Paths

| Platform | Path |
|----------|------|
| **Windows** | `CSIDL_PERSONAL` (My Documents) |
| **macOS** | `~/Documents` |
| **Linux** | `~/.local/share` |

## File Structure

> **NOTE:** This structure was refactored to match the OpenSCAD coding standard where 
> headers (.h) are co-located with their source files (.cpp) in the same directory.
> The original project using this module will need to be updated to reflect this change.

**Current structure (headers alongside sources):**

```
src/
├── platformInfo/
│   ├── export.h                  # DLL export macros
│   ├── extnOSVersRef.h           # OS version reference
│   ├── extnOSVersRef.cpp
│   ├── platformInfo.h            # Platform detection
│   ├── platformInfo.cpp
│   ├── ResourceLocation.h        # ResourceLocation struct
│   ├── resourceLocationManager.h # High-level location management
│   ├── resourceLocationManager.cpp
│   ├── resourcePaths.h           # Path definitions and resource types
│   ├── resourcePaths.cpp
│   └── README.md                 # This file
├── resInventory/
│   ├── resLocMap.h/.cpp          # Flat location storage
│   ├── resLocTree.h/.cpp         # Hierarchical location storage
│   ├── resourceItem.h/.cpp       # Resource item representation
│   ├── resourceIterator.h/.cpp   # Directory scanning iterator
│   ├── resourceScanner.h/.cpp    # Resource scanning logic
│   ├── resourceScannerDirListing.h/.cpp  # QDirListing-based scanner
│   ├── resourceStore.h/.cpp      # Resource storage
│   ├── resourceTreeWidget.h/.cpp # Tree widget for resources
│   └── templateTreeModel.h/.cpp  # Tree model for templates
├── scadtemplates/
│   ├── export.h                  # DLL export macros
│   ├── template.h/.cpp           # Template class
│   ├── template_parser.h/.cpp    # Template parsing
│   ├── template_manager.h/.cpp   # Template management
│   └── ...
└── gui/
    ├── preferencesdialog.h/.cpp  # Preferences dialog
    ├── machineTab.hpp/.cpp       # Machine tier tab
    ├── userTab.hpp/.cpp          # User tier tab
    └── ...
```

**Previous structure (separate include directory - deprecated):**

```
src/
├── include/
│   ├── platformInfo/
│   │   ├── resourcePaths.h
│   │   ├── ResourceLocation.h
│   │   └── ...
│   ├── resInventory/
│   │   └── ...
│   └── scadtemplates/
│       └── ...
├── platformInfo.cpp
├── resourcePaths.cpp
└── ...
```

## Usage Example

```cpp
#include "platformInfo/resourcePaths.h"

using namespace platformInfo;

// Get default paths for preferences "Restore Defaults"
QStringList installDefaults = ResourcePaths::defaultInstallSearchPaths();
QStringList machineDefaults = ResourcePaths::defaultMachineSearchPaths();
QStringList userDefaults = ResourcePaths::defaultUserSearchPaths();

// Get resource type info
const ResourceTypeInfo* fontInfo = ResourcePaths::resourceTypeInfo(ResourceType::Fonts);
QString fontSubdir = fontInfo->subdirectory;  // "fonts"
QStringList fontExts = fontInfo->primaryExtensions;  // {".ttf", ".otf"}

// Get user paths
ResourcePaths paths;
QString userOpenSCAD = paths.userOpenSCADPath();  // e.g., ~/.config/OpenSCAD
QString userFonts = paths.userResourcePath(ResourceType::Fonts);  // ~/.config/OpenSCAD/fonts
```

## Related Modules

- **resInventory** - Storage structures (`ResLocMap`, `ResLocTree`) and `ResourceIterator` for scanning and user selections
- **gui** - `PreferencesDialog` with tier tabs for user configuration
