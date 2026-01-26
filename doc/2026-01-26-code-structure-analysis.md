# Project Structure Analysis for OpenSCAD Integration
**Date:** 2026-01-26  
**Author:** GitHub Copilot  
**Purpose:** Identify working components, dead code, and portability readiness  

---

## Executive Summary

This document analyzes the cppsnippets project to identify:
1. **Working Components** - Ready for OpenSCAD integration
2. **Dead/Disabled Code** - Leftovers from failed attempts
3. **Experimental Code** - Test harnesses and UI factoring tests
4. **Dependencies** - What needs to be ported vs what can stay

### Key Findings

✅ **Ready for OpenSCAD:**
- Resource discovery pipeline (complete, tested)
- Inventory system (Templates & Examples working)
- JSON template parsing
- Resource metadata system

⚠️ **Disabled/Incomplete:**
- GUI application (BUILD_APP=OFF)
- Preferences dialog system (removed during refactoring)
- Resource location widgets (old architecture)
- Model-view tree implementation (needs QVariant update)

🧪 **Experimental:**
- Three test_main_*.cpp files for UI factoring
- Standalone test utilities

---

## 1. Project Architecture Overview

### Five Core Components (Your Requirements)

#### 1. Resource Discovery ✅ COMPLETE
**Location:** `src/pathDiscovery/`, `src/platformInfo/`

**Key Classes:**
- `ResourcePaths` - Discovers all resource locations across tiers
- `ResourceLocation` - Represents a single discoverable location
- `PathElement` - Internal representation during discovery

**Status:** Fully implemented and tested
- Handles Installation/Machine/User tiers
- Environment variable expansion
- Sibling installation detection
- Writable location detection

**Test Coverage:**
- `test_sa_path_discovery` - Path transformation visual tool
- `test_sa_location_discovery` - Location-based discovery
- `test_sa_tier` - Tier testing
- Multiple unit tests in GoogleTest suite

**OpenSCAD Integration Notes:**
- This component is **fully portable**
- Replace application name in `applicationNameInfo.hpp`
- Adjust default paths for OpenSCAD structure
- Keep tier system (Installation/Machine/User)

---

#### 2. Inventory Building ✅ COMPLETE
**Location:** `src/resourceInventory/`

**Key Classes:**
- `TemplatesInventory` - Stores discovered templates
- `ExamplesInventory` - Stores discovered examples
- `ResourceItem` (base class) - Common resource properties
- `ResourceTemplate` (derived) - Template-specific data
- `ResourceScript` (derived) - Example-specific data with attachments

**Storage Pattern:**
```cpp
QHash<QString, QVariant> m_templates;  // TemplatesInventory
QHash<QString, QVariant> m_scripts;    // ExamplesInventory
```

**Status:** Phase 2 complete (QVariant pipeline validated)
- No object slicing
- Preserves derived class data (attachments, editSubtype)
- Type-safe retrieval with `canConvert<T>()`

**Test Coverage:**
- `test_unit_g_phase2_inventory_pipeline` (5 tests, all passing)
- `test_unit_g_templates_inventory` 
- `test_unit_g_examples_inventory`
- `test_unit_g_examples_attachments_categories`

**OpenSCAD Integration Notes:**
- **Directly portable** - inventories are library code
- `addFolder()` methods scan filesystem and populate inventory
- `getAll()` returns `QList<QVariant>` for UI binding
- Consider: Do you need UnknownInventory placeholders for Fonts/Shaders/etc?

---

#### 3. Model Construction ⚠️ NEEDS UPDATE
**Location:** `src/resourceInventory/` (should be in `src/gui/`)

**Current State:**
- `TemplateTreeModel` exists but commented out in CMakeLists.txt
- Uses old architecture (pre-QVariant)
- Not compatible with current inventory system

**What It Needs:**
```cpp
class TemplateTreeModel : public QAbstractItemModel {
    // REQUIRED CHANGES:
    // 1. Accept TemplatesInventory* in constructor
    // 2. Use inventory->getAll() → QList<QVariant>
    // 3. Extract items with .value<ResourceTemplate>()
    // 4. Implement two-level hierarchy:
    //    - Root: Loose templates + Category folders
    //    - Children: Templates within categories
    // 5. Strip .json extension from display names
    // 6. Merge categories across tiers (Installation/Machine/User)
};
```

**OpenSCAD Integration Notes:**
- **Needs reimplementation** before port
- Follow [2026-01-20-multi-resource-tabbed-ui.md](2026-01-20-multi-resource-tabbed-ui.md) design
- Create `ExampleTreeModel` for .scad scripts
- Use Qt's Model/View architecture (see [2026-01-19-qt-model-view-architecture.md](2026-01-19-qt-model-view-architecture.md))

---

#### 4a. GUI Setup - Template/Example Selection Panel ⚠️ IN PROGRESS
**Location:** `src/app/mainwindow.cpp`

**Current Implementation:**
- `MainWindow::setupTemplatesTab()` - Creates template tree + small editor
- `MainWindow::setupExamplesTab()` - Creates example tree (full height)
- Uses `QTabWidget` for resource type selection
- Connects to `TemplatesInventory` and `ExamplesInventory`

**Status:** 
- Method signatures exist (public for testing)
- **BUILD_APP=OFF** so not compiled
- Needs model-view reimplementation

**Test Harnesses:**
- `test_main_templates.cpp` - Tests Templates tab in isolation
- `test_main_examples.cpp` - Tests Examples tab in isolation
- `test_main_combined.cpp` - Tests both tabs together

**OpenSCAD Integration Notes:**
- **Wait for model reimplementation** before porting
- MainWindow pattern is reusable
- TabDialog architecture (see README.md) is proven approach
- Consider: Does OpenSCAD want tabbed UI or separate dialogs?

---

#### 4b. GUI Setup - Text Editor with Template Insertion ❓ NOT YET DESIGNED
**Location:** TBD (not yet implemented)

**Requirement (your description):**
> "GUI setup of a general purpose text editor that will offer a template insertion feature"

**OpenSCAD Context:**
- OpenSCAD has built-in text editor (QScintilla-based?)
- Integration point: Context menu or command palette
- Template insertion: Replace selected text with template body
- Tab stop handling: OpenSCAD uses $1, $2, etc. placeholders

**Design Questions:**
1. Is this for OpenSCAD's existing editor or a new standalone editor?
2. Should templates appear in editor's context menu?
3. How to handle OpenSCAD-specific syntax ($fn, $fa, $fs variables)?
4. Template preview before insertion?

**Recommendation:**
- **Defer this component** until templates/examples UI working
- Study OpenSCAD's editor integration points first
- May need QScintilla-specific hooks

---

#### 5. GUI Operations to Edit Templates ⚠️ PARTIALLY IMPLEMENTED
**Location:** `src/app/mainwindow.cpp`

**Current Slots:**
- `onNewTemplate()` - Create new template (User tier)
- `onDeleteTemplate()` - Delete template (User tier only)
- `onCopyTemplate()` - Copy template to User tier
- `onSaveTemplate()` - Save template changes
- `onEditTemplate()` - Enter edit mode
- `onCancelEdit()` - Cancel edit mode

**Small Editor Panel (Templates Tab):**
```cpp
QLineEdit* m_prefixEdit;        // Template trigger text
QTextEdit* m_bodyEdit;          // Template body
QTextEdit* m_descriptionEdit;   // Description
QLineEdit* m_sourceEdit;        // Read-only provenance
```

**Disabled:** BUILD_APP=OFF

**OpenSCAD Integration Notes:**
- **Ready for adaptation** once model-view working
- Tier-based write protection implemented
- JSON reading/writing via `TemplateParser` class
- File I/O design documented in [2026-01-16-template-file-io-design-discussion.md](2026-01-16-template-file-io-design-discussion.md)

---

## 2. Disabled/Dead Code Analysis

### 2.1 Old Preferences System ❌ REMOVED

**Location:** `src/gui/` (files exist but not in build)

**Disabled Files:**
```
src/gui/resourcelocationwidget.cpp     ❌ Not compiled
src/gui/resourceLocationWidget.hpp     ❌ Not compiled
src/gui/locationInputWidget.cpp        ❌ Not compiled
src/gui/locationInputWidget.hpp        ❌ Not compiled
src/gui/installationTab.cpp            ❌ Not compiled
src/gui/installationTab.hpp            ❌ Not compiled
src/gui/machineTab.cpp                 ❌ Not compiled
src/gui/machineTab.hpp                 ❌ Not compiled
src/gui/userTab.cpp                    ❌ Not compiled
src/gui/userTab.hpp                    ❌ Not compiled
```

**Why Disabled:**
> "# DISABLED: Part of old preferences system - needs reimplementation"  
> — `CMakeLists.txt` line 357

**What It Was:**
- UI for managing custom resource locations (per tier)
- Add/remove custom paths
- Enable/disable locations via checkboxes
- Browse for folders
- XDG_DATA_DIRS and OPENSCAD_PATH env var handling

**Current Placeholder:**
```cpp
// preferencesdialog.cpp, line 35
QLabel* placeholder = new QLabel(tr(
    "Preferences dialog is temporarily disabled.\n\n"
    "ResourceLocationManager was removed during refactoring.\n"
    "This will be reimplemented with the new architecture."));
```

**Decision:**
- **DELETE these files** before OpenSCAD port
- User-designated paths now handled via QSettings
- No UI for path management in Phase 1
- Path discovery is automatic (Installation/Machine/User)

**What's Working Instead:**
- `src/tools/settings_generator.cpp` - CLI tool to set custom paths
- Paths stored in QSettings under "UserDesignatedPaths"
- Discovery layer reads QSettings automatically

---

### 2.2 Old Tree Widget ❌ REMOVED

**Location:** Referenced but doesn't exist

**CMakeLists.txt comment (line 379):**
```cmake
# DISABLED: Uses old API - needs reimplementation
#src/resourceInventory/resourceTreeWidget.cpp
#src/resourceInventory/resourceTreeWidget.hpp
```

**Files:** `resourceTreeWidget.cpp/hpp` do not exist in filesystem

**Decision:**
- **Already removed** - no action needed
- Will be replaced by Model/View pattern (see Section 3 above)

---

### 2.3 Legacy Template Converter ✅ KEEP

**Location:** `src/scadtemplates/legacy_template_converter.cpp`

**Purpose:** Convert old OpenSCAD template format to JSON

**Status:** Active and working
- Used by `TemplateParser` when reading old-format files
- Marks converted templates with `"source": "legacy-converted"`
- Documented in code comments

**Decision:**
- **KEEP and port to OpenSCAD**
- OpenSCAD likely has old templates that need conversion
- Converter is stable and well-tested

**Example:**
```cpp
// TemplateParser detects old format:
if (!jsonDoc.object().contains("_format")) {
    // Fallback for templates without _format marker (old converted files)
    return parseLegacyFormat(jsonDoc.object());
}
```

---

## 3. Experimental/Test Code

### 3.1 UI Factoring Test Executables 🧪

**Purpose:** Test individual UI components in isolation

**Files:**
- `src/app/test_main_templates.cpp` - Templates tab only
- `src/app/test_main_examples.cpp` - Examples tab only
- `src/app/test_main_combined.cpp` - Both tabs together

**What They Do:**
1. Create empty inventories (no resource discovery)
2. Create minimal QApplication
3. Call `MainWindow::setupTemplatesTab()` or `setupExamplesTab()`
4. Display UI for manual testing

**CMakeLists.txt targets:**
```cmake
add_executable(test_main_templates EXCLUDE_FROM_ALL ...)
add_executable(test_main_examples EXCLUDE_FROM_ALL ...)
add_executable(test_main_combined EXCLUDE_FROM_ALL ...)
```

**Decision:**
- **KEEP during development**
- Useful for incremental UI testing
- `EXCLUDE_FROM_ALL` means won't build unless explicitly requested
- **DELETE before final OpenSCAD integration**

---

### 3.2 Standalone Test Utilities 🧪

**Location:** `src/tools/`, `tests/`

**CLI Diagnostic Tools:**
- `test_sa_path_discovery` - Visualize path resolution workflow
- `test_sa_location_discovery` - Test ResourceLocation construction
- `test_sa_tier` - Test tier classification
- `test_sa_env_expansion` - Test environment variable expansion
- `settings_generator` - Generate user-designated paths config
- `test_sa_discovery_inventory` - End-to-end discovery test
- `test_sa_templates_discovery` - Template-specific discovery
- `test_sa_examples_discovery` - Example-specific discovery

**Decision:**
- **KEEP test_sa_path_discovery** - Invaluable debugging tool
- **KEEP settings_generator** - Users need it
- **Optional:** Other test_sa_* tools (useful but not essential)

---

## 4. Dependencies & Portability

### 4.1 Qt Dependencies

**Required by Library (scadtemplates.dll):**
- Qt6::Core - QString, QVector, QSettings, QCoreApplication
- Qt6::Gui - QScreen (platform detection)

**Required by GUI (scadtemplates_app):**
- Qt6::Widgets - All UI classes

**OpenSCAD Status:**
- OpenSCAD already uses Qt
- Version: Check OpenSCAD's Qt version (likely Qt5 or Qt6)
- **Action:** May need to support both Qt5 and Qt6

---

### 4.2 External Dependencies

**JSON Handling:**
- nlohmann-json (JSON parsing)
- nlohmann_json_schema_validator (JSON schema validation)

**OpenSCAD Status:**
- OpenSCAD may already have nlohmann-json
- **Action:** Check OpenSCAD's dependency management (vcpkg? system packages?)

**Testing:**
- GoogleTest 1.14.0 (fetched via CMake FetchContent)
- Qt Test framework

**OpenSCAD Status:**
- OpenSCAD has its own test suite
- **Action:** Adapt tests to OpenSCAD's test framework

---

### 4.3 Build System

**Current:** CMake with Visual Studio 2022 generator

**OpenSCAD:** CMake-based build

**Portability Checklist:**
- ✅ CMake is common ground
- ⚠️ Visual Studio specific: `version.rc` (Windows resource file)
- ✅ Cross-platform code (uses Qt abstractions)
- ⚠️ Job pool limits (specific to this project's build issues)

---

## 5. Ready-to-Port Components

### High Priority (Core Functionality)

| Component | Location | Status | Notes |
|-----------|----------|--------|-------|
| **Resource Discovery** | `src/pathDiscovery/`, `src/platformInfo/` | ✅ Ready | Change app name, test with OpenSCAD paths |
| **Resource Metadata** | `src/resourceMetadata/` | ✅ Ready | Defines resource types, tiers, access |
| **Inventory System** | `src/resourceInventory/` | ✅ Ready | Templates & Examples fully working |
| **Template Parser** | `src/scadtemplates/template_parser.cpp` | ✅ Ready | JSON template reading/writing |
| **Legacy Converter** | `src/scadtemplates/legacy_template_converter.cpp` | ✅ Ready | Convert old OpenSCAD templates |

---

### Medium Priority (Needs Adaptation)

| Component | Location | Status | Notes |
|-----------|----------|--------|-------|
| **Model/View** | TBD | ⚠️ Needs rewrite | Follow Qt Model/View pattern |
| **MainWindow** | `src/app/mainwindow.cpp` | ⚠️ Disabled | UI shell exists, needs model connection |
| **Template Editor UI** | `src/app/mainwindow.cpp` | ⚠️ Disabled | Edit panel exists, needs wiring |

---

### Low Priority (Not Needed for Initial Port)

| Component | Location | Status | Notes |
|-----------|----------|--------|-------|
| **Preferences Dialog** | `src/gui/preferencesdialog.cpp` | ❌ Placeholder | Defer path management UI |
| **Location Widgets** | `src/gui/*Tab.cpp` | ❌ Disabled | Not needed in Phase 1 |

---

## 6. Code Cleanup Checklist

### Files to DELETE

```
❌ src/gui/resourcelocationwidget.cpp
❌ src/gui/resourceLocationWidget.hpp
❌ src/gui/locationInputWidget.cpp
❌ src/gui/locationInputWidget.hpp
❌ src/gui/installationTab.cpp
❌ src/gui/installationTab.hpp
❌ src/gui/machineTab.cpp
❌ src/gui/machineTab.hpp
❌ src/gui/userTab.cpp
❌ src/gui/userTab.hpp
```

**Reason:** Old preferences system, replaced by automatic discovery

---

### Files to KEEP but DISABLE in CMake

```
🧪 src/app/test_main_templates.cpp (EXCLUDE_FROM_ALL)
🧪 src/app/test_main_examples.cpp (EXCLUDE_FROM_ALL)
🧪 src/app/test_main_combined.cpp (EXCLUDE_FROM_ALL)
```

**Reason:** Useful during UI development, excluded from normal builds

---

### Files to REWRITE

```
⚠️ src/resourceInventory/TemplateTreeModel.hpp (if it exists)
⚠️ src/app/mainwindow.cpp (update for QVariant-based models)
```

**Reason:** Old architecture, needs QVariant + Model/View update

---

## 7. Integration Roadmap

### Phase 1: Core Library Port (No GUI)
**Goal:** Get discovery and inventory working in OpenSCAD

1. Port resource discovery layer
   - Copy `src/pathDiscovery/`, `src/platformInfo/`, `src/resourceMetadata/`
   - Update `applicationNameInfo.hpp` with "OpenSCAD"
   - Adjust default paths for OpenSCAD structure

2. Port inventory system
   - Copy `src/resourceInventory/`
   - Test `TemplatesInventory` and `ExamplesInventory`
   - Verify QVariant storage works

3. Port template parser
   - Copy `src/scadtemplates/`
   - Test JSON reading/writing
   - Test legacy template conversion

4. Integration testing
   - Run discovery in OpenSCAD environment
   - Verify templates/examples found
   - Check tier classification

**Deliverable:** OpenSCAD can discover and load templates programmatically

---

### Phase 2: Model/View Implementation
**Goal:** Create Qt models for tree views

1. Implement `TemplateTreeModel : public QAbstractItemModel`
   - Two-level hierarchy (loose + categories)
   - Strip .json extension
   - Merge categories across tiers

2. Implement `ExampleTreeModel : public QAbstractItemModel`
   - Similar structure for .scad scripts
   - Handle attachments (for future use)

3. Test with standalone QTreeView
   - Verify selection works
   - Test with empty inventories
   - Test with large inventories (100+ items)

**Deliverable:** Models ready for UI integration

---

### Phase 3: UI Integration
**Goal:** Create template/example browser UI

1. Decision point: Tabbed UI vs separate dialogs?
   - Current design: QTabWidget with Templates/Examples tabs
   - OpenSCAD preference: ?

2. Implement selection UI
   - Create tree views
   - Connect to models
   - Handle selection changes

3. Implement preview panel (optional)
   - Show template body/description
   - Show example code

**Deliverable:** Users can browse templates/examples

---

### Phase 4: Editor Integration
**Goal:** Insert templates into OpenSCAD editor

1. Study OpenSCAD's editor API
   - How to insert text at cursor?
   - How to handle tab stops ($1, $2)?
   - Context menu integration?

2. Implement template insertion
   - Replace placeholders
   - Position cursor at first tab stop
   - Handle multi-line templates

3. Add keyboard shortcut
   - Ctrl+Space? Ctrl+T?
   - Fuzzy search for template names

**Deliverable:** Templates insertable in editor

---

### Phase 5: Template Editing
**Goal:** Users can create/edit custom templates

1. Implement template editor dialog
   - Prefix, body, description fields
   - Preview panel
   - Save to User tier only

2. Implement CRUD operations
   - New template (User tier)
   - Copy to User tier
   - Edit (User tier only)
   - Delete (User tier only)

3. Refresh inventory after changes
   - No app restart needed

**Deliverable:** Full template management

---

## 8. Open Questions for OpenSCAD Integration

### Architecture Decisions

1. **UI Style:**
   - Tabbed interface (Templates/Examples in same window)?
   - Separate dialogs?
   - Dock widget?

2. **Editor Integration:**
   - Does OpenSCAD use QScintilla?
   - How to hook into editor?
   - Command palette vs context menu vs keyboard shortcut?

3. **Resource Types:**
   - Only Templates and Examples?
   - Add Libraries? (OpenSCAD has .scad libraries)
   - Fonts? (OpenSCAD uses system fonts + custom)

4. **Tier System:**
   - Keep Installation/Machine/User tiers?
   - Add Custom tier for OPENSCAD_PATH locations?

### Technical Questions

5. **Qt Version:**
   - What Qt version does OpenSCAD use?
   - Need to support Qt5 and Qt6?

6. **Dependencies:**
   - Does OpenSCAD already have nlohmann-json?
   - Use vcpkg? system packages? FetchContent?

7. **Build System:**
   - Integrate as subdirectory or external project?
   - Shared library or static?

8. **Testing:**
   - Use OpenSCAD's test framework or keep GoogleTest?
   - How to run tests in OpenSCAD CI?

---

## 9. Documentation Status

### Excellent Documentation (Keep & Port)

- [Resource-Discovery-Specifications.md](Resource-Discovery-Specifications.md) - Discovery workflow
- [2026-01-20-multi-resource-tabbed-ui.md](2026-01-20-multi-resource-tabbed-ui.md) - Multi-resource UI design
- [2026-01-19-qt-model-view-architecture.md](2026-01-19-qt-model-view-architecture.md) - Model/View patterns
- [2026-01-15-phase2-inventory-pipeline-complete.md](2026-01-15-phase2-inventory-pipeline-complete.md) - QVariant architecture
- [2026-01-16-template-file-io-design-discussion.md](2026-01-16-template-file-io-design-discussion.md) - Template file I/O

### Development Logs (Archive but Don't Port)

- Phase results documents (2026-01-*-phase*.md)
- Refactoring plans (2026-01-*-refactor*.md)
- Architecture analyses (2026-01-*-architecture*.md)

**Reason:** Valuable for understanding evolution but not needed in OpenSCAD

---

## 10. Summary & Next Steps

### What's Ready to Port ✅

1. **Resource Discovery Pipeline** - Fully functional, well-tested
2. **Inventory System** - Templates & Examples working with QVariant
3. **Template Parser** - JSON read/write with legacy conversion
4. **Core Data Models** - ResourceItem, ResourceTemplate, ResourceScript

### What Needs Work ⚠️

1. **Model/View Implementation** - Rewrite for QVariant + Qt patterns
2. **UI Components** - Adapt MainWindow for OpenSCAD
3. **Editor Integration** - Design and implement
4. **Testing** - Adapt to OpenSCAD's test framework

### What to Delete ❌

1. **Old Preferences System** - 10 files in src/gui/
2. **ResourceLocationManager** - Removed, replaced by automatic discovery
3. **Test Harnesses** - Keep during dev, delete before merge

### Immediate Action Items

1. **Answer Open Questions** (Section 8)
   - UI style preference
   - Editor integration approach
   - Qt version compatibility

2. **Create Phase 1 Plan**
   - Copy discovery + inventory code
   - Update application name
   - Test in OpenSCAD build environment

3. **Create Model/View Prototype**
   - Implement TemplateTreeModel
   - Test standalone before UI integration

---

## Appendix A: File Count Summary

### Production Code
- **Ready to port:** ~30 files (discovery + inventory + parsing)
- **Needs rewrite:** ~5 files (model/view + mainwindow)
- **Delete:** ~10 files (old preferences system)

### Test Code
- **Unit tests (GoogleTest):** ~15 files (adapt to OpenSCAD)
- **Standalone tests:** ~10 files (keep test_sa_path_discovery)
- **UI test harnesses:** 3 files (keep during dev)

### Documentation
- **Architecture docs:** ~40 markdown files
- **Keep for port:** ~10 files
- **Archive:** ~30 files (development logs)

---

## Appendix B: FIXME/TODO Markers

Search results show several FIXME comments in disabled code:

**In resourcelocationwidget.cpp (DISABLED):**
- Line 98: `// FIXME : need a correct way to set the checkbox`
- Line 102: `// FIXME : need to fix this carefully`
- Line 169: `// FIXME : better to use .contains() on the QList?`
- Line 201: `// FIXME : make a correct check to gray out a disabled path`

**In userTab.cpp (DISABLED):**
- Line 55: `// FIXME : check that this works correctly`
- Line 70: `// FIXME : use correct Qt method to get separator and get rid of #IFDEF block`
- Line 77: `// FIXME : need a correct way to check for resource folders`

**In mainwindow.hpp:**
- Line 91: `void refreshInventory(); // TODO: Implement after save/delete work`

**Decision:** All FIXME/TODO in disabled code will be deleted. MainWindow TODO is known incomplete work.

---

## Document Revision History

| Date | Author | Changes |
|------|--------|---------|
| 2026-01-26 | GitHub Copilot | Initial analysis created |

