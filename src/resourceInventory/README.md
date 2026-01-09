# Resource Inventory Module

This module provides storage structures, iterators, and scanning utilities for managing resource locations across the three resource tiers: **Installation**, **Machine**, and **User**.

## Overview

Resources are organized into three tiers based on their scope and accessibility:

| Tier | Description |
|------|-------------|
| **Installation** | Built-in resources from the application installation |
| **Machine** | System-wide resources available to all users |
| **User** | Personal resources in the user's home directory |

## Architecture

The module has two parallel approaches to resource management:

### 1. Location-Based Storage (ResLocMap, ResLocTree)
Lower-level storage classes that hold `ResourceLocation` objects from the `platformInfo` module. These track *where* resources can be found.

### 2. Qt Model/View System (ResourceItem, ResourceTreeWidget, ResourceScanner)
Higher-level Qt-centric classes that represent actual discovered resources. These track *what* resources exist and their metadata.

### Relationship to Template Classes

The existing `scadtemplates` classes (`Template`, `TemplateManager`, `TemplateParser`) remain unchanged and use `std::string`-based APIs. The new Qt-centric resource classes use `QString` and Qt containers.

Over time, you can compare the two approaches:
- **Template classes**: Lightweight, C++ standard library based
- **Resource classes**: Qt-integrated, ready for UI binding

## Usage Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                    ResourceLocationManager                       │
│   (from platformInfo - provides default search paths)           │
└─────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                   ResourceInventoryManager                       │
│   - setInstallLocations()/setMachineLocations()/setUserLocations()
│   - Coordinates scanning across all tiers                        │
└─────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                      ResourceScanner                             │
│   - Scans file system for resources                             │
│   - Detects resource types by folder/extension                  │
│   - Finds attachments for scripts                               │
└─────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                    ResourceTreeWidget                            │
│   - Displays discovered resources                                │
│   - Supports category grouping                                   │
│   - Checkbox-based enable/disable                                │
│   - Ready for embedding in UI                                    │
└─────────────────────────────────────────────────────────────────┘
```

---

## Classes

### ResLocMap

Storage class for resource locations organized by tier.

Provides three `QMap<QString, ResourceLocation>` members for installation, machine, and user tiers. Best suited for simple/flat resources like fonts, color schemes, shaders, templates, and translations.

**Key Methods:**
- `addInstallLocation()`, `addMachineLocation()`, `addUserLocation()` - Add locations to a tier
- `removeInstallLocation()`, `removeMachineLocation()`, `removeUserLocation()` - Remove locations
- `allInstallLocations()`, `allMachineLocations()`, `allUserLocations()` - Get all locations for a tier
- `hasInstallLocation()`, `hasMachineLocation()`, `hasUserLocation()` - Check if a location exists
- `clear()` - Clear all tiers

---

### ResLocTree

Tree widget for hierarchical resource locations.

Uses Qt's `QTreeWidget`/`QTreeWidgetItem` pattern to display resources in a tree structure. Suitable for libraries with examples, tests, and other nested resources.

**Features:**
- 3 columns: Name, Path, Description
- Visual indicators for enabled/disabled/missing states
- Checkbox support for items that exist and have resource folders

**Key Methods:**
- `addTopLevelLocation()` - Add a root-level resource
- `addChildLocation()` - Add a child resource under a parent
- `findItemByPath()`, `findItemByName()` - Search for items
- `removeItem()` - Remove an item from the tree
- `allLocations()` - Get all locations (flattened list)
- `selectedLocation()` - Get the currently selected location

---

### ResLocTreeItem

Custom tree widget item that holds a `ResourceLocation`.

Extends `QTreeWidgetItem` to store `ResourceLocation` data alongside the standard tree item properties.

**Columns:**
| Index | Name | Content |
|-------|------|---------|
| 0 | ColName | Display name or path |
| 1 | ColPath | Full path |
| 2 | ColDescription | Description text |

---

## Resource Inventory System (Qt Model/View)

These classes provide a Qt-centric approach to resource management, designed to work directly with Qt's model/view architecture.

### ResourceItem

Base class for all resource items. Represents a single resource with its location, metadata, and state.

**Key Properties:**
| Property | Description |
|----------|-------------|
| `path` | Full filesystem path |
| `name` | Base name (from filename) |
| `displayName` | User-friendly name |
| `category` | Category/subfolder grouping |
| `type` | ResourceType enum |
| `tier` | ResourceTier (Installation/Machine/User) |
| `access` | ReadOnly or Writable |
| `sourcePath` | Original path where found (for updates) |
| `sourceLocationKey` | Key in location map (for updates) |

**Resource Types:**
```cpp
enum class ResourceType {
    Unknown,
    ColorScheme,    // .json files
    Font,           // .ttf, .otf, .woff files
    Library,        // Folder with .scad files
    Example,        // .scad script with optional attachments
    Test,           // Like example, for library testing
    Template,       // Writable .scad template
    Shader,         // .vert, .frag, .glsl files
    Translation     // .ts, .qm files
};
```

**Resource Tiers:**
```cpp
enum class ResourceTier {
    Installation,   // Built-in with application
    Machine,        // System-wide (all users)
    User            // User-specific (personal)
};
```

---

### ResourceScript

Extends `ResourceItem` for scripts with attachments (examples, tests).

Scripts may have associated files like images, JSON data, or text files that are used as imports.

**Additional Properties:**
- `scriptPath` - Path to the main .scad file
- `attachments` - List of associated file paths

---

### ResourceTreeItem

Qt tree widget item for hierarchical display.

**Columns:**
| Index | Name | Content |
|-------|------|---------|
| 0 | ColName | Display name (with checkbox) |
| 1 | ColCategory | Category/subfolder |
| 2 | ColPath | Full path |
| 3 | ColTier | Tier name |

**Visual Indicators:**
- Gray text: Resource doesn't exist
- Dark gray text: Resource disabled
- Checkbox: Toggle enabled/disabled (only if exists)

---

### ResourceTreeWidget

Qt tree widget for displaying resources hierarchically.

**Features:**
- Category grouping (optional, for templates)
- Visual indicators for tier, state, and access
- Checkbox-based enabling/disabling
- Selection and multi-selection support

**Key Methods:**
```cpp
// Adding items
ResourceTreeItem* addResource(const ResourceItem& item);
ResourceTreeItem* addResourceToCategory(const ResourceItem& item, const QString& category);
ResourceTreeItem* addChildResource(ResourceTreeItem* parent, const ResourceItem& item);

// Finding items
ResourceTreeItem* findItemByPath(const QString& path);
ResourceTreeItem* findItemByName(const QString& name);
QList<ResourceTreeItem*> findItemsByType(ResourceType type);
QList<ResourceTreeItem*> findItemsByTier(ResourceTier tier);

// Getting items
QList<ResourceItem> allItems() const;
QList<ResourceItem> enabledItems() const;
ResourceItem selectedItem() const;
QList<ResourceItem> selectedResourceItems() const;

// Batch operations
void setAllEnabled(bool enabled);
void setTierEnabled(ResourceTier tier, bool enabled);
```

**Signals:**
- `itemEnabledChanged(const ResourceItem& item, bool enabled)`
- `itemSelected(const ResourceItem& item)`

---

### ResourceScanner

Scans file system locations and builds resource inventories.

**OpenSCAD Folder Conventions:**
| Subfolder | Resource Type |
|-----------|---------------|
| `color-schemes/` | ColorScheme |
| `examples/` | Example |
| `fonts/` | Font |
| `libraries/` | Library |
| `templates/` | Template |
| `locale/` | Translation |

**Key Methods:**
```cpp
// Scan single location
QVector<ResourceItem> scanLocation(const ResourceLocation& location,
                                    ResourceType type,
                                    ResourceTier tier);

// Scan to tree widget
void scanToTree(const QVector<ResourceLocation>& locations,
                ResourceType type,
                ResourceTier tier,
                ResourceTreeWidget* tree);

// Scan all tiers
void scanAllTiers(const QVector<ResourceLocation>& installLocs,
                  const QVector<ResourceLocation>& machineLocs,
                  const QVector<ResourceLocation>& userLocs,
                  ResourceType type,
                  ResourceTreeWidget* tree);

// Special library scanning (hierarchical)
void scanLibraries(const QVector<ResourceLocation>& locations,
                   ResourceTier tier,
                   ResourceTreeWidget* tree);
```

**Signals:**
- `scanStarted(ResourceType type, int locationCount)`
- `locationScanned(const QString& path, int itemCount)`
- `scanCompleted(ResourceType type, int totalItems)`
- `scanError(const QString& message)`

---

### ResourceInventoryManager

High-level manager that coordinates scanning and maintains inventories.

**Example Usage:**
```cpp
ResourceInventoryManager manager;

// Set locations for each tier
manager.setInstallLocations(installLocs);
manager.setMachineLocations(machineLocs);
manager.setUserLocations(userLocs);

// Build inventory for a specific type
ResourceTreeWidget* colorSchemes = manager.buildInventory(ResourceType::ColorScheme);

// Get or build inventory (lazy loading)
ResourceTreeWidget* fonts = manager.inventory(ResourceType::Font);

// Refresh after changes
manager.refreshInventory(ResourceType::Template);
```

---

## Template Handling

Templates have special handling:
- **Writable**: Users can create and modify templates
- **Category merging**: Subfolders define categories; same-named subfolders from different locations merge
- **Category display**: Tree widget shows category grouping when `setShowCategories(true)`

---

## Library Hierarchy

Libraries are scanned hierarchically:
```
libraries/
├── MyLibrary/
│   ├── main.scad         # Library entry point
│   ├── utils.scad        # Additional library files
│   ├── examples/
│   │   ├── demo1.scad    # Nested as Example under library
│   │   └── demo2.scad
│   └── tests/
│       └── test_main.scad # Nested as Test under library
└── AnotherLibrary/
    └── ...
```

---

### ResourceIteratorBase

Abstract base class for resource iterators.

Provides the interface for iterating over resource locations and collecting found resources into appropriate storage structures.

**Key Methods:**
- `scan()` - Scan the configured folders for resources (pure virtual)
- `isFlat()` - Check if this iterator produces flat (map) results
- `isHierarchical()` - Check if this iterator produces tree results
- `tier()` - Get the resource tier being scanned
- `folderLocations()` - Get the folder paths being scanned
- `resourceTypes()` - Get the resource types being searched for

---

### ResourceIteratorFlat

Iterator for flat/simple resources.

Scans folder locations for resources that have a simple flat structure, such as fonts, color schemes, shaders, templates, and translations. Results are collected into a `ResLocMap` organized by tier.

**Example Usage:**
```cpp
QStringList folders = {"/usr/share/openscad", "/opt/openscad"};
QVector<ResourceType> types = {ResourceType::Fonts, ResourceType::ColorSchemes};

ResourceIteratorFlat iterator(ResourceTier::Machine, folders, types);
if (iterator.scan()) {
    ResLocMap& results = iterator.results();
    // Process found resources...
}
```

---

### ResourceIteratorTree

Iterator for hierarchical resources.

Scans folder locations for resources that have a hierarchical structure, such as libraries that may contain examples, tests, and sub-libraries. Results are collected into a `ResLocTree` with parent-child relationships preserved.

**Example Usage:**
```cpp
QStringList folders = {"~/.local/share/OpenSCAD/libraries"};
QVector<ResourceType> types = {ResourceType::Libraries};

ResourceIteratorTree iterator(ResourceTier::User, folders, types);
if (iterator.scan()) {
    ResLocTree* tree = iterator.results();
    // Navigate the tree structure...
}
```

**Key Methods:**
- `scan()` - Scan and collect resources into the internal tree
- `results()` - Get the scan results (pointer to `ResLocTree`)
- `takeResults()` - Transfer ownership of the results tree to the caller

---

### ResourceIteratorFactory

Factory class for creating appropriate resource iterators.

Creates either a flat (`ResourceIteratorFlat`) or hierarchical (`ResourceIteratorTree`) iterator based on the resource types requested.

**Resource Type to Iterator Mapping:**

| Resource Type | Iterator Type |
|---------------|---------------|
| Examples | Flat |
| Tests | Flat |
| Fonts | Flat |
| ColorSchemes | Flat |
| Shaders | Flat |
| Templates | Flat |
| Libraries | **Hierarchical** |
| Translations | Flat |

**Example Usage:**
```cpp
// Automatic selection based on resource type
auto fontIter = ResourceIteratorFactory::create(
    ResourceTier::User,
    userFolders,
    {ResourceType::Fonts}
);

if (fontIter->isFlat()) {
    auto* flatIter = static_cast<ResourceIteratorFlat*>(fontIter.get());
    flatIter->scan();
    // Use flatIter->results()...
}

// Create iterator for libraries (will be hierarchical)
auto libIter = ResourceIteratorFactory::create(
    ResourceTier::Machine,
    machineFolders,
    {ResourceType::Libraries}
);

if (libIter->isHierarchical()) {
    auto* treeIter = static_cast<ResourceIteratorTree*>(libIter.get());
    treeIter->scan();
    // Use treeIter->results()...
}
```

**Factory Methods:**
- `create()` - Automatically select iterator type based on resource types
- `createFlat()` - Explicitly create a flat iterator
- `createTree()` - Explicitly create a hierarchical iterator
- `isHierarchicalType()` - Check if a resource type requires hierarchical iteration
- `containsHierarchicalType()` - Check if any types in a list are hierarchical

---

## File Structure

```
src/
├── include/resInventory/
│   ├── resLocMap.h           # Flat storage (QMap-based)
│   ├── resLocTree.h          # Hierarchical storage (QTreeWidget-based)
│   ├── resourceIterator.h    # Iterator classes and factory
│   ├── resourceItem.h        # ResourceItem, ResourceScript base classes
│   ├── resourceTreeWidget.h  # Qt tree widget for resources
│   └── resourceScanner.h     # Scanner and inventory manager
└── resInventory/
    ├── resLocMap.cpp
    ├── resLocTree.cpp
    ├── resourceIterator.cpp
    ├── resourceItem.cpp
    ├── resourceTreeWidget.cpp
    ├── resourceScanner.cpp
    └── README.md             # This file
```

---

## Complete Example

```cpp
#include "resInventory/resourceScanner.h"
#include "resInventory/resourceTreeWidget.h"
#include "platformInfo/resourceLocationManager.h"

using namespace resInventory;

// Create inventory manager
ResourceInventoryManager manager;

// Get locations from the resource location manager (platformInfo module)
platformInfo::ResourceLocationManager locMgr;
manager.setInstallLocations(locMgr.availableInstallLocations());
manager.setMachineLocations(locMgr.availableMachineLocations());
manager.setUserLocations(locMgr.availableUserLocations());

// Build inventories for different resource types
ResourceTreeWidget* colorSchemes = manager.inventory(ResourceType::ColorScheme);
ResourceTreeWidget* fonts = manager.inventory(ResourceType::Font);
ResourceTreeWidget* libraries = manager.inventory(ResourceType::Library);
ResourceTreeWidget* templates = manager.inventory(ResourceType::Template);

// Templates have category support enabled automatically
// Categories are based on subfolder names and merge across tiers

// Add widgets to your UI
colorSchemesTab->layout()->addWidget(colorSchemes);
fontsTab->layout()->addWidget(fonts);
librariesTab->layout()->addWidget(libraries);
templatesTab->layout()->addWidget(templates);

// Connect to signals
connect(templates, &ResourceTreeWidget::itemEnabledChanged,
        this, [](const ResourceItem& item, bool enabled) {
    qDebug() << "Template" << item.name() << "enabled:" << enabled;
});

// Get enabled items for actual use
QList<ResourceItem> activeTemplates = templates->enabledItems();
for (const auto& tmpl : activeTemplates) {
    qDebug() << "Using template:" << tmpl.path();
}
```

---

## Writable vs Read-Only Resources

| Resource Type | Access | Can User Create? | Can User Modify? |
|---------------|--------|------------------|------------------|
| ColorScheme | ReadOnly | No | No |
| Font | ReadOnly | No | No |
| Library | ReadOnly | No | No |
| Example | ReadOnly | No | No |
| Test | ReadOnly | No | No |
| **Template** | **Writable** | **Yes** | **Yes** |
| Shader | ReadOnly | No | No |
| Translation | ReadOnly | No | No |

Templates are the only writable resource type. When creating or modifying templates:
- They should be saved to User tier locations
- The `sourcePath` and `sourceLocationKey` track the original location
- Category is preserved for proper grouping

---

## Source Tracking

Each `ResourceItem` tracks its origin for update scenarios:

```cpp
ResourceItem item = ...;

// Where the resource currently lives
QString currentPath = item.path();

// Where it was originally found (for updates/reimport)
QString originalPath = item.sourcePath();

// Which location entry it came from
QString locationKey = item.sourceLocationKey();
// e.g., "OPENSCAD_PATH", "XDG_DATA_DIRS: /usr/share/openscad", etc.
```

This allows the application to:
1. Detect when upstream resources have changed
2. Offer to re-import or update local copies
3. Show users where each resource originated
