# Resource Discovery & Management Architecture

**Date:** January 6, 2026  
**Status:** Implementation in Progress  
**Author:** GitHub Copilot (Claude Sonnet 4.5)
**Editor:** Jeff Hayes

---

## Overview

This document defines the data flow and component responsibilities for the resource discovery and management system. Resources (templates, examples, fonts, color schemes) are discovered in three tiers (Installation, Machine, User) and made available through a unified inventory.

---

## Data Flow Pipeline

```markdown
1. ResourcePaths::qualifiedSearchPaths()
   └─> QList<PathElement> (s_discoveryPaths)
   
2. DiscoveryScanner(s_discoveryPaths)
   └─> QList<ResourceLocation> (validatedLocations)
   
3. LocationScanner(validatedLocations)  (*new input in following sections*)
   └─> QVariant<resourceBase> 
   └─> QTree<resExamples>     s_examples     (pointers to .scad etc)
   └─> QTree<resTests>        s_tests        (pointers to .scad etc)
   └─> QTree<resLibraries>    s_ibraries     (pointers to .scad etc)
   └─> QList<resTemplates>    s_templates    (templates in QJsonObjects)
   └─> QList<resEditorColors> s_editorColors (theme color pallette in QJsonObjects)
   └─> QList<resRenderColors> s_renderColors (theme color pallette in QJsonObjects)
   └─> QList<resFonts>        s_renderColors (pointers to font files)
   └─> QList<res .. etc >     s_ etc         (pointers to .. etc)

  - groups will only ever appear in trees of resources that have heirarchy
  - "newResourecs" folder will be scanned during discovery so that any resources dropped in may be appended to the appropriate storager structure

   
4. ResourceScanner(s) (enrichedLocations)
   XXX Resource objects per type
  I no longer see a need for this "layer" of processing .. 
  the LocationScanner becomes the sorting engine for resources
   
5. ResourceInventory
   └─> Stores all discovered resources with metadata
Location Scanner can fill the resource structure or structures plural

the last time i wrote a spec for all this i thought the QVariant class would be of use in making the resource inventory as a single structure .. but we (I) have experience with only templates and color theme files .. all of which are json files that have to be read in to be able to use them. Resources from binary files (i.e. fonts) need only a file name reference and whatever metadata is needed by the app to use them

so .. we can proceed with the first few steps in the plan and think more about how to build the resource inventory

```

---

## Component Definitions

### **1. ResourcePaths** (existing, refactored)

**File:** `src/platformInfo/resourcePaths.hpp/cpp`  
**Namespace:** `platformInfo`

**Purpose:** Generate qualified search paths for resource discovery

**Key Method:**

```cpp
QList<PathElement> qualifiedSearchPaths() const;
```

**Output:**

- **Type:** `PathElement = {ResourceTier, QString path}`
- **Type:** `QList<PathElement> discoveryPaths;`
- **Contains:** 
  - Environment variables expanded when found in a path
  - User-designated paths from QSettings
- **Special Case:** Installation tier
  - Secial Case : install folder suffix : default "", or given as compiler option
  - Sibling installations, if present (LTS ↔ Nightly)

- **Output** discoveryPaths
- **Consumer:** DiscoveryScanner

**Responsibilities:**

- ✅ Expand environment variables (`${VAR}`, `%VAR%`)
- ✅ Apply folder name appending rules (trailing `/` → append folderName + suffix)
- ✅ Detect sibling installations (bidirectional LTS ↔ Nightly)
- ✅ Load user-designated paths from QSettings
- ✅ Return single consolidated list with tier embedded in each PathElement

**Status:** ✅ **COMPLETE** (Phase 1 of refactoring)

---

### **2. DiscoveryScanner** (NEW - to be created)

**File:** `src/resourceMgmt/discoveryScanner.hpp/cpp` (proposed)  
**Namespace:** `resourceMgmt` (proposed)

**Purpose:** Search the given paths for resource folders

**Key Method:**

```cpp
static QList<ResourceLocation> scanDiscoveryPaths(const QList<PathElement>& discoveryPaths);
```

**Input:**

- **Type:** `QList<PathElement> discoveryPaths` - Output of step 1

**Process: Filtering:**

- empty paths are dropped
- Flags paths (error msg) that exist but are not readable
- Preserves tier information from PathElement

>> FYI .. there is no such thing as a duplicate path.
The only possibility is that a user may input a discovery path in that is already on the list .. but as we do not have the GUI to take that input we do not need to worry about it.

**Tech:**
Use Qt based search features, nameliy QDirIterator

**QDirIterator**
Use QDirIterator to navigate entries of a directory one at a time. It is similar to QDir::entryList() and QDir::entryInfoList(), but because it lists entries one at a time instead of all at once, it scales better and is more suitable for large directories. It also supports listing directory contents recursively, and following symbolic links.

The QDirIterator constructor takes a QDir or a directory as argument. After construction, the iterator is located before the first directory entry. 

The constructor of interest is :

```cpp
QDirIterator::QDirIterator(const QString &path, QDir::Filters filters, QDirIterator::IteratorFlags flags = NoIteratorFlags)
```

Constructs a QDirIterator that can iterate over path, with no name filtering and filters for entry filtering. You can pass options via flags to decide how the directory should be iterated.

By default, filters is QDir::NoFilter, and flags is NoIteratorFlags, which provides the same behavior as in QDir::entryList().

**QFilter:**

```cpp
enum QDir::Filter
flags QDir::Filters
```

This enum describes the filtering options available to QDir; e.g. for entryList() and entryInfoList(). The filter value is specified by combining values from the following list using the bitwise OR operator:

**Responsibilities:**

For each path

- check that the folder exists
- what access this user has 
- instantiate a `ResourceLocation` objects:
- copy `path` and `tier` from PathElement
- determine the user's `access`and create the approriate QFilter to record that
- `displayName` - derived from path

Bad Location Warnings

- if a path folder is found, but is empty OR is not readable its condition needs to be reported .. in the case of this app a qdebug message will be enough

FYI :

- resourceLocation objects are only instantiated when a given path actually exists and has at least one resource folder in it.
There is no need for memeber booleans for existance or content

- **Output** QList<resourceLocation> s_locations
- **Consumer:** LocationScanner

**Status:** ⏳ **TO BE IMPLEMENTED**

---

### **3. LocationScanner** (NEW - to be created)

**File:** `src/locationMgmt/locationScanner.hpp/cpp` (proposed)  
**Namespace:** `locationMgmt` (proposed)

**Purpose:** THe Discovery scanner has processed the paths to where resources _might be_ found and has produced the Locations list of places where resources are known to exist.

Now the Location Scanner searchs the paths to find the ones that do have resources for the app.

**Key Method:**

```cpp
static QList<ResourceLocation> enrichLocations(const QList<ResourceLocation>& s_locations);
```

**Input:**

- **Type:** `QList<resourceLocation> s_locations` known resource locations
- **Source:** Output of step 2 - DiscoveryScanner

**Output:**

- **Type:** `resource inventory storage objects - based on resourceItem class`

**New Responsibilites given reduction to four processing steps from five:**

- At each location
  - check which resource folders are present by names: `examples`, `fonts/`, etc.
  - Use `ResourceTypeInfo` to know
    - what filter to build for member files - primaryExtensions
    - what attachment files are present - attachmentExtensions
    - resource sub-folders need to be looked for - subResTypes
  - use the appropriate resource-specific scanner to catalog 
    - resoruces found
    - sub-folders (groups)

**Special Case: Groups:**

A folder in a Location folder that does not have one of the resource folder names should be ignored.

But in a resource folder that \_may_ have a resourceTypeInfo::Group as a sub-folder the scanner  shall enter the group into the resource inventory tree using its folder name to indentify it.

Group folders may only contain resources, not folders, so any sub-folders shall be ignored.
The group will use primaryExtensions, attachmentExtensions, and subResTypes the same as the parent resourceType.

**Special Case: NewResources:**

This is to be a folder in the User Tier that must be made by the user in one of the paths in the default list OR in a user defined path.

This folder must have the name "newresources" and have read-write access for the user.

If it does not exist, or is not readable and writeable, then drag-drop of resource files, as known by their extensions, will fail with appropriate warning mesages to console.

**Special Case: Examples:**

The legacy of Example folders is that they may have sub-folders that define "categories".
All examples from any source that has sub-folders of the same name will group their .scad files into the those categories.

Say there is an example sub folder named "objectHandling" in more than one example folder.
All example .scad files in those folders will be displayed together in the GUI.

In this app the Group resources will handle this case naturally, taking the category name from the name of the subfolders.
The only functional change is in the ExampleScanner code to form lists of .scad files for each group resource instead of a tree.

**Tech:**

use of this stuff is NOT required but may be useful.

**QVariant:**

docs  https://doc.qt.io/qt-6/qvariant.html

> A QVariant object holds a single value of a single typeId() at a time. You can even store QList<QVariant> and QMap<QString, QVariant> values in a variant, so you can easily construct arbitrarily complex data structures of arbitrary types. This is very powerful and versatile, but may prove less memory and speed efficient than storing specific types in standard data structures.

- Type-safe union for Qt data types
- Custom types via `Q_DECLARE_METATYPE`
- `QVariant::fromValue<T>()` and `value<T>()` for custom types
- `QVariantList` and `QVariantMap` aliases for complex structures


**Status:** ⏳ **TO BE IMPLEMENTED**

---

### **4. ResourceScanner(s)** (removed from plan)

**Responsibilities:**

see step 3 - "resource scanning" is to be folded into Location scanning

**Status: Deleted from plan:**

---

### **5. ResourceInventory** (NEW - to be created)

**File:** `src/resourceMgmt/resourceInventory.hpp/cpp` (proposed)  
**Namespace:** `resourceMgmt` (proposed)

**Purpose:** storage for all discovered resources

**Input:**

- **Type:** from ResourceScanner(s)
  - `QList<ResourceItem*>` flat resources like templates
  - `QTree<ResourceItem*>` hierarchical resources like Libraries

- **Source:** Output of Location Scanning

**Storage:**

- for discovered resources, their attachments and contained resource structures
- Fancy indexing is not needed as resources will almost always be accessed sequentially
- certain res types can be edited by the user, but will be selected in the GUI from lists or trees shown in widgets so indexing can be left to the iterator methods on QList, QMap, or QTree

**Responsibilities:**

- Store resource inventory
- for editable resources
  - provide two-way access to GUI editing widgets
  - provide CRUD operations on resInvnetory and files in File System
  - provide file read-write facilites

**Tech:**

use of this stuff is NOT required but may be useful.

**QDataWidgetMapper:**

docs https://doc.qt.io/qt-6/qdatawidgetmapper.html

> QDataWidgetMapper can be used to create data-aware widgets by mapping them to sections of an item model. A section is a column of a model if the orientation is horizontal (the default), otherwise a row.

- Maps model columns/rows to UI widgets
- `AutoSubmit` or `ManualSubmit` policies
- `addMapping(widget, section)` binds widgets to model columns
- Works with any `QAbstractItemModel`

**QDataWidgetMapper Example**:

```cpp
QDataWidgetMapper *mapper = new QDataWidgetMapper;
mapper->setModel(model);
mapper->addMapping(nameLineEdit, 0);
mapper->addMapping(ageSpinBox, 1);
mapper->toFirst();
```

**Status:** ⏳ **TO BE IMPLEMENTED**

---

## Storage and Lifecycle

### **Where is `s_discoveryPaths` stored?**

**Option A:** No persistent storage

```cpp
// Generate on-demand in discovery coordinator
// NO refereces to file name elements, like suffix, needed at this point
ResourcePaths paths;
QList<PathElement> discoveryPaths = paths.qualifiedSearchPaths();

DiscoveryScanner::scanDiscoveryPaths(discoveryPaths);
```

**Recommendation:** **Option B** - cache in coordinator for inspection/debugging

>> FYI
 Sorry but no. USE Option A.
More complexity than it is worth.
The discovery scanner will be relatively simple and with clearly defined inputs and outputs it will be easy to verify correct operation.
Even if we have to use Google Mock techniques it will be better to avoid the work and risks of another complex class

---

### **ResourceLocation vs Resource**

**Critical Distinction:**

- **ResourceLocation** = A **place** where resources _have been found_
  - Example: `C:/Program Files/OpenSCAD/examples/`
  - Represents a directory, not a specific file
  - will contain resource folders, not files
  - Stored as `QList<ResourceLocation>` with each location annotated by its tier

- **ResourceItem/Resource** = An actual **resource file**
  - Example: `C:/Program Files/OpenSCAD/examples/Basics/sphere.scad`
  - Represents a specific file
  - IF the resource is a file its extension will determine its type _in most cases_
    - the exception is .json files that are known to be used for three different resource types at the moment
  - Hierarchical resources will have to be structured in trees to be able to display them correctly in the GUI .. normally in Preference Panels for editing but also for selecting an example or test .scad file to edit and run in the app

---

## Persistence via QSettings

**ResourcePaths:**

- Key: `ScadTemplates/ResourcePaths`
- Format: `QStringList`

**ResourcePaths: NOT TO BE Persisted in Settings:**

- Hard coded default path lists

**ResourcePaths: Persisted in Settings:**

- designated paths added by the user - may be an empty list
- a feature that we can use in testing to inject badly structured resoruces or locations

**Discovery Paths: NOT TO BE Persisted in Settings:**

**Resource Locations:**

_Significaqnt change_ : we will NOT implement the Enabling feature - while there may be a future requirement for a user to selectively enable and disable certain Locations, or even individual resources it is an overly complex feature for the current project.

**ResourceRegistry: WILL NOT BE USED:**

**ResourceInventory:**

- Not persisted - rebuilt by Discovery process during app startup

---

## Phase Implementation Plan

### **Phase 1: Path Generation** ✅ **COMPLETE**

- [x] Environment variable expansion
- [x] Folder name appending rules
- [x] Sibling installation detection
- [x] User-designated paths from QSettings
- [x] Single `qualifiedSearchPaths()` output
- [x] Unit tests (28 tests)
- [x] Tools (settings-generator, test-env-expansion)

### **Phase 2: Discovery Pipeline** ⏳ **NEXT**

**Step 2.1:** Create DiscoveryScanner

- Create ResourceLocation objects

**Step 2.2:** Create LocationScanner

- Check for resource subdirectories
- Preserve tier and access info

**Step 2.3:** Create ResourceDiscoveryCoordinator

- _SCRAPPED_

**Step 2.4:** Tests

- Mock filesystem for DiscoveryScanner tests (exists - may need to be expanded)
- Verify LocationScanner detects subdirectories
- Integration test for full pipeline

### **Phase 3: Resource Scanning** 🔜 **FUTURE**

- Refactor existing ResourceScanner - possibly (better to write new?)
- Create per-resource type scanners (if needed)

### **Phase 4: Inventory & Registry** 🔜 **FUTURE**

- Create ResourceInventory class(es ?)
- Repurpose ResourceLocationManager → ResourceRegistry _SCRAPPED_
- Implement QSettings persistence - CHANGED, see Persistence Section

### **Phase 5: GUI Integration** 🔜 **FUTURE**

>> FYI 

- Update preferences dialog
- templates tree view (limit on current project)
  - tier
    - Jeff-Documents
      - template1.json
    - Jeff-Appdata
      - template1.json    << same name permitted
- Enable/disable per location _SCRAPPED_
- User defined path CRUD

- general tree view (future)
  - tier 
    - type
    - Examples
      - category1   << same category as 
        - examplefile.scad
    - Library
      - a.scad
      - Examples
        - LibExample1.scad
        - LibExample1.png
        - category1          << this category means ...
          - examplefile.scad << will show as a duplicate in the GUI
    - fonts
    etc etc.


---

## Questions to Resolve

1. **DiscoveryScanner vs LocationScanner naming**
   - Current: DiscoveryScanner validates paths

2. **Single ResourceScanner vs per-type scanners**
   - Templates have NOT nexting << clarification
   - Fonts are to be listed and other wise ignored
choice depends on complexity of implementation
   - Or single scanner with type-specific handlers?

It is already known that some resources are hierarchical, other flat, so the resource inventory structures need to be ready to support the GUI widgets that will display them

3. **ResourceItem class hierarchy CHANGE:**
   - Base class `ResourceItem`

unless new information comes out of the development process Resource Items should probably be built on the basis of the QVariant class

4. **Storage of "what types exist at location"**
   - By the time that resources are being used in the GUI the info about them, and, were applicaable, their content, needs to come from the ResourceItems in the resInventory

---

## Dependencies

this section needs to be rewritten based on the new information in the preceeding sections

**Cross-cutting:**

- `ResourceTypeInfo` - used by LocationScanner << now folding this -ResourceScanner- itto Location Scanner
- `ResourceTier` - used throughout pipeline
- `Access` - used only as far as the Discovery Scanner .. Locations are, by definition, at least readable
- `QSettings` - used by ResourcePaths to persist user paths ONLY

