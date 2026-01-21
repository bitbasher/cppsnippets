# Multi-Resource Inventory with Tabbed UI

**Date:** January 20, 2026  
**Author:** User + AI Assistant  
**Status:** Planning  
**Branch:** resScannerOption1

---

## User Requirements

### Primary Goal
Extend the application to handle multiple resource types (Templates and Examples) with a unified editing interface.

### Specific Requirements
1. Support both Templates (.json) and Examples (.scad) in the same application
2. Use tabbed interface on left side for resource type selection
3. Preserve current Templates UI design (tree + small editor panel)
4. Add Examples UI with full-height tree (no bottom panel)
5. Right-side Scintilla editor handles both JSON and OpenSCAD files
6. Single scan of filesystem populates both inventories
7. Same window, no separate applications

---

## Problem Statement

### Current State
- Application only handles Templates (JSON snippet definitions)
- Single inventory model: `TemplatesInventory`
- Left side: Template tree + small editor panel (vertical split)
- Right side: Scintilla editor (full height)
- `main.cpp` creates and returns single inventory

### What Needs to Change
1. **Data Layer:** Need `ExamplesInventory` for .scad script resources
2. **Discovery Layer:** `main.cpp::resourceManager()` must scan and return multiple inventories
3. **UI Layer:** Left panel becomes tabbed widget with different layouts per tab
4. **Editor Layer:** Scintilla must handle JSON (templates) and OpenSCAD (examples) syntax
5. **State Management:** Track which resource type is active for save operations

---

## Proposed Solution

### Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│ Main Window (Existing Horizontal Splitter)         │
│ ┌─────────────────────┬─────────────────────────┐  │
│ │ LEFT: QTabWidget    │ RIGHT: Scintilla       │  │
│ │ ┌─────┬─────┐       │ (Unchanged)            │  │
│ │ │Tmpls│Exmpl│       │                        │  │
│ │ └─────┴─────┘       │ - Handles .json        │  │
│ │                     │ - Handles .scad        │  │
│ │ [Active Tab UI]     │ - Single instance      │  │
│ │                     │ - Switch lexer on      │  │
│ │                     │   tab change           │  │
│ └─────────────────────┴─────────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

### Tab Layouts

**Templates Tab (Preserves Current Design):**
```
┌─────────────────────┐
│ Template Tree       │
│ (QTreeView)         │
├─────────────────────┤
│ Template Editor     │
│ (QTextEdit, small)  │
└─────────────────────┘
```

**Examples Tab (New, Simpler):**
```
┌─────────────────────┐
│                     │
│ Example Tree        │
│ (QTreeView)         │
│ (Full height)       │
│                     │
│                     │
└─────────────────────┘
```

### Data Flow

```
main.cpp::resourceManager()
    ↓
Discovers all ResourceLocations
    ↓
Creates TemplatesInventory + ExamplesInventory
    ↓
Calls scanLocations() on each
    ↓
Returns ResourceInventories struct {templates, examples}
    ↓
MainWindow receives both inventories
    ↓
Creates tabbed UI with both trees
    ↓
Connects selections to Scintilla editor
```

---

## Multi-Phase Plan

### Phase 1: Create ExamplesInventory Class
**Goal:** Establish data model for Examples resources  
**Risk:** Low - cloning proven TemplatesInventory pattern  

**Changes:**
- Create `src/resourceInventory/ExamplesInventory.hpp`
- Create `src/resourceInventory/ExamplesInventory.cpp`
- Inherit from QAbstractItemModel (same as TemplatesInventory)
- Implement scanning methods: `addExample()`, `addFolder()`, `scanLocation()`, `scanLocations()`
- Store ResourceScript objects instead of ResourceTemplate
- Handle `.scad` file extension instead of `.json`
- Use "examples" subfolder from ResourceTypeInfo

**Files Created:**
- `src/resourceInventory/ExamplesInventory.hpp`
- `src/resourceInventory/ExamplesInventory.cpp`

**Files Modified:**
- `src/resourceInventory/CMakeLists.txt` (add new files to build)

**Success Criteria:**
- ✅ ExamplesInventory compiles
- ✅ Can scan example folders
- ✅ Stores ResourceScript items
- ✅ Implements QAbstractItemModel interface
- ✅ Unit tests pass (if added)

---

### Phase 2: Update main.cpp for Multiple Inventories
**Goal:** Resource discovery returns both Templates and Examples  
**Risk:** Low - simple struct return type  

**Changes:**
- Create `ResourceInventories` struct in main.cpp:
  ```cpp
  struct ResourceInventories {
      resourceInventory::TemplatesInventory* templates;
      resourceInventory::ExamplesInventory* examples;
  };
  ```
- Update `resourceManager()` signature to return struct
- Create both inventory instances
- Call `scanLocations()` on both (single filesystem scan)
- Update `main()` to handle struct return

**Files Modified:**
- `src/app/main.cpp`

**Success Criteria:**
- ✅ Builds successfully
- ✅ Both inventories populated
- ✅ Counts reported in qDebug output
- ✅ MainWindow receives both inventories

---

### Phase 3: Refactor MainWindow Constructor
**Goal:** Accept multiple inventories, prepare for tabbed UI  
**Risk:** Low - parameter addition only  

**Changes:**
- Update MainWindow constructor:
  ```cpp
  MainWindow(TemplatesInventory* templates,
             ExamplesInventory* examples,
             QWidget* parent = nullptr);
  ```
- Store both inventories as member variables:
  ```cpp
  resourceInventory::TemplatesInventory* m_templatesInventory;
  resourceInventory::ExamplesInventory* m_examplesInventory;
  ```
- Update main.cpp to pass both inventories
- No UI changes yet - still using only templates

**Files Modified:**
- `src/app/mainwindow.hpp`
- `src/app/mainwindow.cpp`
- `src/app/main.cpp` (pass both inventories)

**Success Criteria:**
- ✅ Builds successfully
- ✅ Application runs unchanged
- ✅ Both inventories accessible in MainWindow
- ✅ Templates display still works

---

### Phase 4: Implement Tabbed Left Panel
**Goal:** Convert left panel to QTabWidget with two tabs  
**Risk:** Medium - UI restructuring affects existing code  

**Changes:**
- Add member variables to MainWindow:
  ```cpp
  QTabWidget* m_resourceTabs;
  QTreeView* m_templateTree;  // Was direct child of splitter
  QTreeView* m_exampleTree;   // NEW
  QTextEdit* m_templateEditorPanel;  // The small bottom panel
  ```

- Restructure left panel layout:
  1. Create QTabWidget
  2. **Tab 0 "Templates":**
     - Create QWidget container
     - Add QSplitter (vertical) inside container
     - Top: m_templateTree (connected to m_templatesInventory)
     - Bottom: m_templateEditorPanel (small, max height 150px)
  3. **Tab 1 "Examples":**
     - Add m_exampleTree directly (no splitter, full height)
     - Connect to m_examplesInventory
     - **Configure tree model:**
       - Two-level hierarchy: loose examples + category folders
       - Strip `.scad` extension from display names
       - Merge categories across all tiers (Installation/Machine/User)
     - No attachments display
     - No tier badges or filters
  4. Replace left panel of main splitter with m_resourceTabs

**Files Modified:**
- `src/app/mainwindow.hpp` (add member variables)
- `src/app/mainwindow.cpp` (restructure UI creation)
- `src/resourceInventory/ExamplesInventory.hpp` (may need display name method)
- `src/resourceInventory/ExamplesInventory.cpp` (may need category grouping)

**Success Criteria:**
- ✅ Builds successfully
- ✅ Left panel shows tabs
- ✅ Templates tab displays as before (tree + small panel)
- ✅ Examples tab shows two-level tree (loose + categories)
- ✅ Example names display without .scad extension
- ✅ Categories merge examples from all tiers
- ✅ Clicking templates loads in Scintilla
- ✅ Clicking examples loads in Scintilla
- ✅ Tab switching works

---

### Phase 5: Add Scintilla Syntax Switching
**Goal:** Scintilla uses correct lexer for active resource type  
**Risk:** Low - lexer switching is standard Scintilla feature  

**Changes:**
- Add slot: `void MainWindow::onResourceTabChanged(int index)`
- Add helper methods:
  ```cpp
  void setupScintillaForJson();
  void setupScintillaForScad();
  ```
- When tab changes:
  - Track active resource type
  - Switch Scintilla lexer
  - Clear editor content
- When template selected:
  - Load JSON content
  - Ensure JSON lexer active
- When example selected:
  - Load .scad content
  - Ensure OpenSCAD/C++ lexer active

**Files Modified:**
- `src/app/mainwindow.hpp` (add slots and helpers)
- `src/app/mainwindow.cpp` (implement lexer switching)

**Success Criteria:**
- ✅ JSON files syntax-highlighted as JSON
- ✅ .scad files syntax-highlighted as OpenSCAD/C++
- ✅ Switching tabs clears editor
- ✅ Correct lexer after tab switch

---

### Phase 6: Handle Save Operations
**Goal:** Save button respects active resource type and tier writability  
**Risk:** Low - simple if/switch on active type  

**Changes:**
- Add state tracking:
  ```cpp
  enum class ActiveResourceType { Templates, Examples };
  ActiveResourceType m_activeType = ActiveResourceType::Templates;
  QString m_currentResourceKey;  // ID of loaded resource
  ResourceTier m_currentResourceTier;  // Track source tier
  ```
- Update `onResourceTabChanged()` to set `m_activeType`
- Update `onSaveClicked()`:
  ```cpp
  if (m_activeType == ActiveResourceType::Templates) {
      // Save to m_templatesInventory->writeJsonContent()
  } else if (m_activeType == ActiveResourceType::Examples) {
      if (m_currentResourceTier == ResourceTier::User) {
          // Can overwrite original or prompt for Save As
      } else {
          // Must save to user-writable location
          // Prompt for location in User examples folder
      }
  }
  ```
- **Save Policy:**
  - User-tier resources: Allow overwrite or Save As
  - Install/Machine tier resources: Force Save As to user location
  - Update inventory immediately after save (no restart needed)

**Files Modified:**
- `src/app/mainwindow.hpp` (add state tracking)
- `src/app/mainwindow.cpp` (update save logic)

**Success Criteria:**
- ✅ Saving templates updates .json files
- ✅ Saving examples updates .scad files
- ✅ User-tier examples can overwrite
- ✅ Install/Machine tier examples force Save As
- ✅ Inventory updates after save
- ✅ No cross-contamination between types
- ✅ Save disabled when nothing selected

---

### Phase 7: Testing and Refinement
**Goal:** Verify all functionality works correctly  
**Risk:** Low - validation phase  

**Tasks:**
- Test template selection and editing
- Test example selection and editing
- Test tab switching behavior
- Test syntax highlighting for both types
- Test save operations for both types
- Verify memory management (no leaks)
- Check UI layout on resize
- Test with empty inventories
- Test with missing folders

**Success Criteria:**
- ✅ All manual tests pass
- ✅ No crashes or warnings
- ✅ UI responsive and intuitive
- ✅ No memory leaks (valgrind or similar)

---

## Decision Log

### Decision 1: Single Application vs Separate Apps
**Date:** 2026-01-20  
**Options Considered:**
- A: Single app, two windows
- B: Separate applications (ScadTemplates, ScadExamples)
- C: Single app, fully tabbed interface
- **D: Single app, tabbed left selection, shared editor** ← CHOSEN

**Rationale:**
- User proposed Option D after seeing Options A-C
- Eliminates editor duplication
- Provides unified UX (always same editor)
- Simpler state management
- Single Scintilla instance handles both file types
- More scalable for future resource types

---

### Decision 2: Left Panel Layout
**Date:** 2026-01-20  
**Options Considered:**
- A: Both tabs use same layout (tree + small panel)
- **B: Templates keeps current design, Examples uses full-height tree** ← CHOSEN

**Rationale:**
- Templates need the small editor panel (likely for snippet metadata/preview)
- Examples don't need bottom panel - .scad content displays in main Scintilla
- Maximizes vertical space for example tree (may have categories/hierarchy)
- Preserves existing Templates workflow (no user retraining)

---

### Decision 3: Data Structure for Multiple Inventories
**Date:** 2026-01-20  
**Options Considered:**
- A: Return QHash<ResourceType, QAbstractItemModel*>
- **B: Return simple struct with named pointers** ← CHOSEN
- C: Create InventoryManager class

**Rationale:**
- Struct is explicit and type-safe
- Easy to extend with more types
- Avoids dynamic_cast when retrieving inventories
- Clear ownership (main.cpp creates, MainWindow uses)
- No over-engineering for two inventories

---

### Decision 4: Syntax Highlighting Strategy
**Date:** 2026-01-20  
**Options Considered:**
- A: Detect file type on load
- **B: Switch lexer on tab change + verify on selection** ← CHOSEN
- C: Auto-detect from file extension

**Rationale:**
- Tab change gives clear signal of intent
- Ensures correct lexer always active for tab context
- Selection verification catches edge cases
- Prevents confusion if user switches tabs then types

---

## Implementation Notes

### ExamplesInventory Differences from TemplatesInventory

| Aspect | TemplatesInventory | ExamplesInventory |
|--------|-------------------|-------------------|
| File extension | `.json` | `.scad` |
| Item type | `ResourceTemplate` | `ResourceScript` |
| Folder name | `templates/` | `examples/` |
| Structure | Flat files | Two-level: loose + category folders |
| Display name | Full filename | Strip `.scad` extension |
| Category handling | N/A | Merge same-named categories across tiers |
| Tier visibility | May show tier badges | Tiers hidden from user |
| Metadata | JSON parsing | OpenSCAD comments + attachments |
| Validation | JSON schema | OpenSCAD syntax (future) |
| Attachments | None | Tracked but not displayed in UI |

### ResourceScript Class Status
**Question:** Does ResourceScript already exist?  
**Action:** Verify in Phase 1 planning. If not, may need to enhance resourceItem.hpp.

### Scintilla Lexer for OpenSCAD
**Note:** OpenSCAD syntax is C-like, so `QsciLexerCPP` should work.  
**Future:** Could create custom OpenSCAD lexer for better highlighting.

---

## File Changes Summary

### Phase 1: ExamplesInventory
- ✅ **NEW:** `src/resourceInventory/ExamplesInventory.hpp`
- ✅ **NEW:** `src/resourceInventory/ExamplesInventory.cpp`
- ⚠️ **MODIFY:** `src/resourceInventory/CMakeLists.txt`

### Phase 2: main.cpp
- ⚠️ **MODIFY:** `src/app/main.cpp`

### Phase 3: MainWindow Constructor
- ⚠️ **MODIFY:** `src/app/mainwindow.hpp`
- ⚠️ **MODIFY:** `src/app/mainwindow.cpp`

### Phase 4-6: Tabbed UI
- ⚠️ **MODIFY:** `src/app/mainwindow.hpp` (add members, slots)
- ⚠️ **MODIFY:** `src/app/mainwindow.cpp` (restructure layout, add logic)

---

## Risks and Mitigations

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| ExamplesInventory differs significantly from templates | Low | Medium | Clone and adapt proven TemplatesInventory pattern |
| UI restructuring breaks existing Templates workflow | Low | High | Test Templates tab thoroughly before proceeding |
| Scintilla lexer switching causes crashes | Low | Medium | Verify lexer cleanup, test switch repeatedly |
| Save logic corrupts wrong file type | Low | High | Use strong typing for m_activeType, add assertions |
| Memory leak from multiple inventories | Low | Medium | Use Qt parent-child ownership, verify with tools |

---

## Success Criteria (Overall)

### Functional Requirements
- ✅ Templates and Examples both selectable via tabs
- ✅ Templates display and edit correctly (no regression)
- ✅ Examples display and edit correctly
- ✅ Syntax highlighting correct for both types
- ✅ Save operations work for both types
- ✅ No crashes or data corruption

### Non-Functional Requirements
- ✅ No performance degradation
- ✅ Memory usage reasonable (< 10% increase)
- ✅ UI responsive on all operations
- ✅ Code follows project conventions

### User Experience
- ✅ Intuitive tab switching
- ✅ Clear indication of active resource type
- ✅ Consistent editor behavior
- ✅ No unexpected state changes

---

## Next Steps

1. **User Approval:** Review this plan and approve before implementation
2. **Phase 1 Start:** Create ExamplesInventory class skeleton
3. **Incremental Commits:** One phase per commit with documentation
4. **Known Good Checkpoints:** Tag after each successful phase
5. **Results Docs:** Create phase results documents as work progresses

---

## Examples Tab Requirements (Clarified 2026-01-21)

### Loading and Discovery
- **Policy:** ExamplesInventory must be fully populated **before** GUI starts
- **Rationale:** Example list must be immediately available for "Open" operations
- **No Rescan:** Application never rescans at runtime (user must restart app if filesystem changes)
- **Exception:** User-edited examples saved to writable locations will update inventory immediately

### Display Structure
- **Two-Level Tree:**
  - Root level: Loose examples (no category) + Category folders
  - Category level: Examples within that category
- **Example:**
  ```
  anExample
  anotherExample
  📁 category1
     cat1Example1
     cat1Example2
  📁 category2
     cat2Example1
  ```

### File Extensions
- **Strip .scad Extension:** Display names never show `.scad` (redundant - all examples are .scad)
- **Example:** File `myExample.scad` displays as `myExample`

### Category Handling
- **Cross-Tier Grouping:** Categories with same name from all tiers (Installation/Machine/User) merge into single category folder
- **Example:** 
  - `C:\Program Files\OpenSCAD\examples\category1\` (Install tier)
  - `C:\ProgramData\OpenSCAD\examples\category1\` (Machine tier)
  - Both appear under one `📁 category1` branch
- **Tier Transparency:** User doesn't see tier information - all examples of same category grouped together

### Selection and Editing
- **Double-Click:** Opens example in right-side Scintilla editor directly (no intermediate panel)
- **Edit Button:** Same as double-click - loads into main editor
- **No Attachments Display:** Attachment tracking exists (for future OpenSCAD integration) but not shown in UI
- **Save Behavior:**
  - User-tier examples: Can overwrite original or save as new file in writable location
  - Install/Machine tier examples: Must save to user-writable location (prompt for location)

### What's NOT Needed
- ❌ No rescan/refresh button
- ❌ No attachments list or count display
- ❌ No tier filtering or tier badges
- ❌ No attachment viewer
- ❌ No status bar updates for scanning

---

## Open Questions

~~1. **ResourceScript Class:**~~ ✅ RESOLVED
   - Exists in `resourceItem.hpp`
   - Handles .scad file metadata via `ResourceScript` class
   - Supports category via `category()` method

~~2. **Example Categories:**~~ ✅ RESOLVED (see Examples Tab Requirements above)
   - Two-level tree: loose examples + category folders
   - Categories merge across all tiers
   - No tier visibility in UI

3. **Template Editor Panel:**
   - What is currently displayed in the small bottom panel for templates?
   - Snippet preview? Metadata? Description?
   - Needed for understanding current workflow

4. **Future Resource Types:**
   - Fonts, Shaders, Tests - do they need similar treatment?
   - Should architecture accommodate easy addition of new tabs?
   - Generic "ResourceTab" base class worth considering?

---

## References

- [2026-01-19-qt-model-view-architecture.md](2026-01-19-qt-model-view-architecture.md) - Model/View architecture decisions
- [2025-12-22-Implementation-Status-Report.md](2025-12-22-Implementation-Status-Report.md) - Current system status
- Qt Documentation: QTabWidget, QTreeView, QSplitter
- Scintilla Documentation: Lexer switching, QsciLexer classes

---

## Revision History

| Date | Author | Changes |
|------|--------|---------|
| 2026-01-20 | AI + User | Initial planning document created |
| 2026-01-21 | AI + User | Added Examples Tab Requirements section with detailed policies for loading, display structure, category handling, and save behavior. Updated Phase 4 and Phase 6 to reflect clarified requirements. Resolved Open Questions 1 and 2. |
