# Revised Resource Discovery Architecture - Analysis & Decisions

**Date:** January 6, 2026  
**Status:** Planning Analysis  
**Author:** GitHub Copilot (Claude Sonnet 4.5)  
**Based On:** User's revised `2026-01-05-resource-discovery-architecture.md`

---

## Executive Summary

The architecture has been **significantly simplified** from the original 5-stage pipeline to a **3-stage pipeline** with clearer responsibilities. Key changes eliminate complexity while maintaining functionality.

### Major Architectural Changes

1. **Stage 4 (ResourceScanner) ELIMINATED** - folded into LocationScanner
2. **ResourceDiscoveryCoordinator SCRAPPED** - unnecessary complexity
3. **QDirIterator preferred** over QDirListing (simpler, proven)
4. **LocationScanner now handles resource scanning** directly
5. **No enable/disable feature** - KISS principle
6. **ResourceRegistry SCRAPPED** - not needed
7. **Option A selected** - no caching of discovery paths

### New 3-Stage Pipeline

```
Stage 1: ResourcePaths (✅ COMPLETE)
    └─> QList<PathElement> discoveryPaths
    
Stage 2: DiscoveryScanner (⏳ TO BUILD)
    └─> QList<ResourceLocation> validatedLocations
    
Stage 3: LocationScanner (⏳ TO BUILD)
    └─> Resource Inventory Structures:
        ├─> QTree<ResourceItem*> s_examples
        ├─> QTree<ResourceItem*> s_tests  
        ├─> QTree<ResourceItem*> s_libraries
        ├─> QList<ResourceItem*> s_templates
        ├─> QList<ResourceItem*> s_editorColors
        ├─> QList<ResourceItem*> s_renderColors
        ├─> QList<ResourceItem*> s_fonts
        └─> QList<ResourceItem*> s_etc
```

---

## Critical Decisions & Rationale

### Decision 1: Use QDirIterator, Not QDirListing

**Rationale from User:**
> "Use Qt based search features, nameliy QDirIterator"

**Why This Matters:**
- QDirIterator is proven, mature API (available since Qt 4)
- QDirListing is new (Qt 6.8+), less tested in production
- QDirIterator meets all our needs for recursive scanning
- No need to adopt bleeding-edge API for this use case

**Action:** Phase 2 implementation uses `QDirIterator` exclusively

---

### Decision 2: No Duplicate Path Detection

**User Guidance:**
> ">> FYI .. there is no such thing as a duplicate path. The only possibility is that a user may input a discovery path in that is already on the list .. but as we do not have the GUI to take that input we do not need to worry about it."

**Implications:**
- DiscoveryScanner does NOT need deduplication logic
- Simpler implementation
- If future GUI allows user paths, handle deduplication at input time, not scan time

**Action:** Remove deduplication code from Phase 2 design

---

### Decision 3: ResourceLocation Only for Validated Paths

**User Guidance:**
> "FYI: resourceLocation objects are only instantiated when a given path actually exists and has at least one resource folder in it. There is no need for memeber booleans for existance or content"

**Implications:**
- ResourceLocation objects represent CONFIRMED resource locations
- No `exists` boolean needed (they ALL exist by definition)
- No `hasResourceFolders` boolean needed (they ALL have resources)
- DiscoveryScanner must validate BOTH existence AND content

**Critical Change:** DiscoveryScanner has two responsibilities:
1. Check path exists
2. Check path contains at least one resource folder

**Action:** DiscoveryScanner returns only populated locations

---

### Decision 4: LocationScanner Handles Resource Scanning

**User Guidance:**
> "I no longer see a need for this 'layer' of processing .. the LocationScanner becomes the sorting engine for resources"

**Rationale:**
- Eliminates a processing layer (ResourceScanner)
- LocationScanner directly populates inventory structures
- Cleaner data flow: Locations → Resources in one step

**Challenge:** LocationScanner becomes more complex but more cohesive

**Action:** Phase 2B (LocationScanner) includes resource scanning logic

---

### Decision 5: No Coordinator Class

**User Guidance:**
> ">> FYI Sorry but no. USE Option A. More complexity than it is worth. The discovery scanner will be relatively simple and with clearly defined inputs and outputs"

**Rationale:**
- Simple function calls suffice: `ResourcePaths → DiscoveryScanner → LocationScanner`
- No need to cache intermediate results
- Testing is straightforward with mock inputs
- KISS principle

**Action:** No ResourceDiscoveryCoordinator class created

---

### Decision 6: Separate Storage by Resource Type

**User Guidance (from pipeline diagram):**
```
└─> QTree<resExamples>     s_examples
└─> QTree<resTests>        s_tests
└─> QTree<resLibraries>    s_libraries
└─> QList<resTemplates>    s_templates
... etc
```

**Rationale:**
- Hierarchical resources (examples, tests, libraries) use `QTree`
- Flat resources (templates, fonts, colors) use `QList`
- Matches GUI display requirements naturally
- Type-safe storage (no need for QVariant casting at retrieval time)

**Question for User:** Where do these storage structures live?
- Option A: Global statics (like `s_examples` naming suggests)
- Option B: Encapsulated in a ResourceInventory singleton
- Option C: Passed as output from LocationScanner, managed by caller

**My Recommendation:** Option B - singleton with accessor methods

---

### Decision 7: Templates Are Flat, No Nesting

**User Clarification:**
> "Templates have NOT nexting << clarification"

**From Resource-Discovery-Specifications.md:**
- Templates folder structure is FLAT
- Only `.json` files, NO `.scad` files
- No category subfolders

**Action:** Template scanner does non-recursive scan

---

### Decision 8: Examples Use Groups for Categories

**User Guidance:**
> "In this app the Group resources will handle this case naturally, taking the category name from the name of the subfolders. The only functional change is in the ExampleScanner code to form lists of .scad files for each group resource instead of a tree."

**Clarification Needed:**
- Are examples stored as a flat list with `category` metadata?
- Or stored in a tree structure where nodes are Groups?

**Current Understanding:**
```cpp
// Option A: Flat with metadata
QList<ResourceItem*> examples;
example->setCategory("Basics");

// Option B: Tree structure
QTreeWidget root;
├─ GroupNode("Basics")
│  └─ ExampleItem("sphere.scad")
└─ GroupNode("Advanced")
   └─ ExampleItem("mesh.scad")
```

**Question:** Which structure does `QTree<resExamples>` represent?

---

### Decision 9: NewResources Folder for Drag-Drop

**User Guidance:**
> "This is to be a folder in the User Tier that must be made by the user in one of the paths in the default list OR in a user defined path. This folder must have the name 'newresources' and have read-write access for the user."

**Requirements:**
- Folder name: `newresources` (lowercase, specific name)
- Tier: User tier only
- Access: Read-write required
- Purpose: Target for drag-drop operations

**Implementation Questions:**
1. Does DiscoveryScanner detect this folder specially?
2. Does LocationScanner create it if missing?
3. Or is it only checked when drag-drop occurs?

**My Recommendation:** 
- Phase 2 detects it if present
- Phase 5 (GUI) creates it on first drag-drop
- For now, just document the requirement

---

### Decision 10: No Enable/Disable Location Feature

**User Guidance:**
> "_Significaqnt change_: we will NOT implement the Enabling feature"

**Removed:**
- `isEnabled` field from ResourceLocation
- User toggle in preferences
- QSettings persistence of enabled state

**Rationale:**
- Overly complex for current project scope
- Can add later if needed

**Action:** Remove from Phase 2 design

---

## Open Questions Requiring User Answers

### Q1: Namespace Organization

**Current State:**
- `platformInfo` - ResourcePaths
- `resInventory` - ResourceItem, existing ResourceScanner
- `resourceMgmt` - proposed for DiscoveryScanner
- `locationMgmt` - proposed for LocationScanner

**Question:** Should we consolidate namespaces?

**Options:**
- **A:** Keep as-is (3 separate namespaces)
- **B:** Move new classes to `resInventory` (consolidate with existing)
- **C:** Create single `resourceSystem` namespace and migrate everything

**My Recommendation:** Option B - add to `resInventory` namespace

---

### Q2: ResourceLocation Data Structure

**User says:** Only instantiate when path exists AND has resources

**Current ResourceLocation fields (from existing code):**
```cpp
struct ResourceLocation {
    QString path;
    ResourceTier tier;
    QString displayName;
    bool exists;        // REMOVE (always true)
    bool isWritable;    // KEEP
    bool isEnabled;     // REMOVE (feature scrapped)
    bool hasResourceFolders;  // REMOVE (always true)
};
```

**Proposed Simplified ResourceLocation:**
```cpp
struct ResourceLocation {
    QString path;
    ResourceTier tier;
    QString displayName;
    bool isWritable;
    // Optional: QList<ResourceType> detectedTypes; (for efficiency)
};
```

**Question:** Should we store `detectedTypes` to avoid re-scanning in LocationScanner?

**Options:**
- **A:** Store `QList<ResourceType> detectedTypes` in DiscoveryScanner
  - Pro: LocationScanner knows what to scan immediately
  - Con: DiscoveryScanner does more work
  
- **B:** Don't store, let LocationScanner discover types
  - Pro: DiscoveryScanner stays simple (just validates paths)
  - Con: LocationScanner checks every type at every location

**My Recommendation:** Option A - store detected types for efficiency

---

### Q3: ResourceItem Storage Location

**From pipeline diagram:** `s_examples`, `s_templates`, etc. (static naming)

**Question:** Where do these live architecturally?

**Options:**

**A: Global Statics (C-style)**
```cpp
// resourceInventory.hpp
extern QTree<ResourceItem*> s_examples;
extern QList<ResourceItem*> s_templates;
// ... etc

// resourceInventory.cpp
QTree<ResourceItem*> s_examples;
QList<ResourceItem*> s_templates;
```

**B: Singleton Class**
```cpp
class ResourceInventory {
public:
    static ResourceInventory& instance();
    
    QTree<ResourceItem*>& examples() { return m_examples; }
    QList<ResourceItem*>& templates() { return m_templates; }
    // ... etc
    
private:
    ResourceInventory() = default;
    QTree<ResourceItem*> m_examples;
    QList<ResourceItem*> m_templates;
};
```

**C: Passed as Parameters**
```cpp
// Caller creates structures
QTree<ResourceItem*> examples;
QList<ResourceItem*> templates;

// LocationScanner populates them
LocationScanner::scan(locations, examples, templates, ...);
```

**My Recommendation:** Option B - Singleton with clear ownership

---

### Q4: QTree Implementation

**User specifies:** `QTree<ResourceItem*>`

**Problem:** Qt does NOT have a `QTree` container class

**Available Qt Options:**
1. **QTreeWidget** - UI widget with data
2. **QTreeView + QStandardItemModel** - MVC pattern
3. **Custom tree structure** (std::vector of nodes with children)

**Question:** What did you mean by `QTree`?

**Options:**

**A: QTreeWidget (easiest, UI-coupled)**
```cpp
QTreeWidget* s_examples;  // Widget is also data store
```

**B: QStandardItemModel (MVC, better separation)**
```cpp
QStandardItemModel* s_examplesModel;
// Use with QTreeView in GUI
```

**C: Custom tree class**
```cpp
template<typename T>
class ResourceTree {
    struct Node {
        T* data;
        QList<Node*> children;
    };
    Node* m_root;
};
```

**My Recommendation:** Option B - QStandardItemModel for proper MVC

---

### Q5: Examples Storage Structure

**User guidance:** "form lists of .scad files for each group resource instead of a tree"

**Interpretation Conflict:**
- Pipeline shows: `QTree<resExamples>`
- User says: "form lists... for each group"

**Question:** Which structure do you want?

**Option A: Tree with Group nodes**
```
QStandardItemModel (tree)
├─ TierItem("Installation")
│  ├─ GroupItem("Basics")
│  │  ├─ ExampleItem("sphere.scad")
│  │  └─ ExampleItem("cube.scad")
│  └─ GroupItem("Advanced")
│     └─ ExampleItem("mesh.scad")
└─ TierItem("User")
   └─ ExampleItem("myExample.scad")  // No group
```

**Option B: Flat list with category field**
```
QList<ResourceItem*>
├─ Example("sphere.scad", tier=Installation, category="Basics")
├─ Example("cube.scad", tier=Installation, category="Basics")
├─ Example("mesh.scad", tier=Installation, category="Advanced")
└─ Example("myExample.scad", tier=User, category="")
```

**Then GUI builds tree display from list?**

**My Recommendation:** Option A - store as tree, matches display structure

---

### Q6: Access Field Encoding

**User guidance:**
> "determine the user's `access` and create the approriate QFilter to record that"

**Question:** How should we store access information?

**Options:**

**A: Boolean flags**
```cpp
struct ResourceLocation {
    bool isReadable;
    bool isWritable;
    bool isExecutable;
};
```

**B: Enum**
```cpp
enum class Access {
    ReadOnly,
    ReadWrite,
    ReadWriteExecute
};
```

**C: QDir::Permissions (Qt native)**
```cpp
QFileInfo info(path);
QFile::Permissions perms = info.permissions();
```

**My Recommendation:** Option A - simple booleans (readable always true, writable is what matters)

---

## Technical Clarifications

### DiscoveryScanner Responsibilities (Expanded)

**Original thought:** Just validate path exists

**User requirement:** Only instantiate ResourceLocation if path has resource folders

**New Responsibilities:**
1. Check path exists (`QDir(path).exists()`)
2. Check path is readable (`QFileInfo(path).isReadable()`)
3. **Check path contains at least one resource folder** (new!)
   - Look for: `examples/`, `fonts/`, `templates/`, `color-schemes/`, etc.
   - Use `ResourceTypeInfo` to get expected folder names
4. Determine write access (`QFileInfo(path).isWritable()`)
5. Generate display name
6. Create ResourceLocation only if path EXISTS and HAS RESOURCES

**This means DiscoveryScanner is NOT just path validation** - it does initial resource folder detection.

**Question:** Should we rename it to avoid confusion?
- Current: DiscoveryScanner (sounds like it scans everything)
- Alternative: LocationValidator
- Alternative: LocationDiscovery
- Alternative: Keep name, update docs

---

### LocationScanner Complexity Increase

**Original plan:** Just enrich locations with metadata

**New plan:** Full resource scanning and inventory population

**LocationScanner now must:**
1. For each ResourceLocation
2. For each ResourceType
3. Check if type's subfolder exists
4. If yes, scan that subfolder:
   - Get file list (recursive or flat based on type)
   - Filter by extensions
   - Find attachments for script types
   - Handle Groups (categories) for examples
   - Create ResourceItem objects
   - Add to appropriate inventory structure

**This is substantial work** - essentially what the old ResourceScanner did, plus inventory population.

**Recommendation:** Break LocationScanner into:
- **LocationScanner** - orchestrates per location
- **TypeScanners** - specialized scanners per resource type
  - `TemplateScanner::scan(path) → QList<ResourceItem*>`
  - `ExampleScanner::scan(path) → QTree<ResourceItem*>`
  - `FontScanner::scan(path) → QList<ResourceItem*>`
  - etc.

This keeps each piece testable and focused.

---

## Implementation Strategy for Phase 2

### Phase 2A: DiscoveryScanner

**Scope:**
- Input: `QList<PathElement>`
- Output: `QList<ResourceLocation>` (validated AND populated)
- Logic:
  1. For each PathElement
  2. Skip if path is empty
  3. Check path exists
  4. Check readable
  5. Look for resource folders (examples/, fonts/, etc.)
  6. If any found, create ResourceLocation
  7. Determine write access
  8. Generate display name
  9. Add to output list

**Testing:**
- Unit test with QTemporaryDir
- Create mock directory structures
- Verify only populated locations returned

**Estimated Complexity:** Medium (more than just validation)

---

### Phase 2B: LocationScanner + TypeScanners

**Scope:**
- Input: `QList<ResourceLocation>`
- Output: Populated inventory structures
  - `QTree/QList` for each resource type
- Logic:
  1. Initialize inventory structures
  2. For each ResourceLocation
  3. For each ResourceType
  4. Check if type's subfolder exists
  5. If yes, invoke appropriate TypeScanner
  6. TypeScanner returns scanned items
  7. Add items to inventory structure

**Type Scanners Needed:**
- TemplateScanner (flat .json scan)
- EditorColorScanner (flat .json in color-schemes/editor/)
- RenderColorScanner (flat .json in color-schemes/render/)
- FontScanner (flat .ttf/.otf scan)
- ExampleScanner (recursive .scad + attachments + groups)
- TestScanner (similar to examples but simpler)
- LibraryScanner (recursive .scad for libraries)

**Testing:**
- Unit test each TypeScanner independently
- Integration test LocationScanner orchestration
- Use ResourceTestFixture with realistic structures

**Estimated Complexity:** High (lots of type-specific logic)

**Recommendation:** Split into sub-phases:
- **2B.1:** LocationScanner scaffold + TemplateScanner (simplest)
- **2B.2:** Font, ColorScheme scanners (still simple)
- **2B.3:** ExampleScanner with Groups (complex)
- **2B.4:** Test and Library scanners

---

## Risk Analysis

### Risk 1: LocationScanner Scope Creep

**Problem:** Combining resource scanning with location processing

**Mitigation:**
- Use TypeScanner classes to isolate complexity
- Test each scanner independently
- Keep LocationScanner as orchestrator only

### Risk 2: QTree Ambiguity

**Problem:** Qt has no `QTree` container

**Mitigation:**
- User must clarify which tree structure to use
- Recommend QStandardItemModel for proper MVC

### Risk 3: Examples Storage Confusion

**Problem:** Conflicting guidance (tree vs list)

**Mitigation:**
- User must clarify storage vs display structure
- May need both: list for storage, tree for display

### Risk 4: Testing Hierarchical Resources

**Problem:** Examples, libraries have complex nesting

**Mitigation:**
- Build comprehensive test fixtures
- Use QTemporaryDir for isolated tests
- Test category merging across locations

---

## Recommended Next Steps

1. **User Answers Required Questions** (Q1-Q6 above)

2. **Create Phase 2A Detailed Design**
   - DiscoveryScanner class design
   - ResourceLocation final struct
   - Unit test strategy

3. **Implement Phase 2A** (small, focused)
   - Write tests first
   - Implement DiscoveryScanner
   - Verify with real and mock data

4. **Create Phase 2B Detailed Design** (after 2A complete)
   - LocationScanner orchestrator
   - TypeScanner interfaces
   - Inventory structure decisions (after Q3/Q4 answered)

5. **Implement Phase 2B in Sub-Phases**
   - Start with simplest (templates)
   - Add complexity gradually
   - Integration test after each scanner

---

## Summary of User Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Iterator API | QDirIterator | Mature, proven, sufficient |
| Coordinator | No coordinator class | KISS principle |
| Path deduplication | Not needed | No GUI input yet |
| ResourceLocation | Only for valid+populated | Simpler logic |
| Stage 4 | Eliminated | Folded into Stage 3 |
| Enable/disable | Feature scrapped | Too complex for now |
| QSettings persistence | User paths only | No location enable state |
| Storage caching | No caching | Generate on-demand |
| Resource storage | Separate by type | Type-safe, matches GUI |

---

## Questions Requiring User Input Before Phase 2A

**Critical (blocks implementation):**
- Q2: Should DiscoveryScanner store detectedTypes?
- Q3: Where do inventory structures live? (Singleton vs global vs parameter)
- Q4: What does `QTree<ResourceItem*>` mean? (QStandardItemModel?)

**Important (affects design):**
- Q1: Namespace consolidation?
- Q5: Examples storage structure (tree vs list)?
- Q6: Access encoding (booleans vs enum)?

**Once these are answered, Phase 2A detailed design can be created.**

---

## References

- [QDirIterator Documentation](https://doc.qt.io/qt-6/qdiriterator.html)
- [QStandardItemModel Documentation](https://doc.qt.io/qt-6/qstandarditemmodel.html)
- User's revised `2026-01-05-resource-discovery-architecture.md`
- Original `Resource-Discovery-Specifications.md`
