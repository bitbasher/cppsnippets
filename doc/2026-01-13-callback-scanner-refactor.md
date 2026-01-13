# Callback-Based ResourceScanner Refactor

**Date:** 2026-01-13  
**Author:** User + Agent  
**Status:** Phase 1 Complete ✅ - Proceeding to Phase 2  

---

## User Requirements

"i like callbacks .. a bit harder to do blackbox testing but it is possible to do unit testing on them. lets do this with call backs"

**Goal:** Refactor ResourceScanner to use callback pattern for streaming item processing, enabling both efficient production code (direct model insertion) and testable code (captured lists).

---

## Problem Statement

**Current Architecture Issues:**
1. **ResourceInventoryManager** - Returns GUI widgets (QTreeWidget*) from business logic layer
2. **MainWindow constructor** - Performs resource scanning (wrong responsibility)
3. **Temporary lists** - Building QVector<ResourceItem> then copying to model (memory waste)
4. **Hard to test** - Business logic coupled to GUI components
5. **Inflexible** - Can't easily log, filter, or transform items during scanning

**What Works Well:**
- ✅ ResourceScanner's scan algorithms (scanTemplates, scanLibraries, etc.)
- ✅ ResourceItem DTO structure with QVariant
- ✅ ResourceLocation discovery and tier tagging
- ✅ ResourceMetadata enums (Gold Standard)

---

## Proposed Solution

### Callback Pattern Architecture

```cpp
class ResourceScanner {
public:
    using ItemCallback = std::function<void(const ResourceItem&)>;
    
    // Core scanning with callback
    void scanTemplates(const QString& path, 
                      ResourceTier tier,
                      const QString& locationKey,
                      ItemCallback onItemFound);
    
    // Convenience wrappers
    QVector<ResourceItem> scanTemplatesToList(...);
    void scanTemplatesToModel(..., QStandardItemModel* model);
    
private:
    void addItemToModel(QStandardItemModel* model, const ResourceItem& item);
};
```

### main.cpp Structure

```cpp
bool resourceManager(QStandardItemModel* model) {
    try {
        auto allLocations = platformInfo::discoverAllLocations();
        
        ResourceScanner scanner(allLocations);
        scanner.scanToModel(model);  // Streaming insertion
        
        return true;
    } catch (...) { return false; }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QStandardItemModel* inventory = new QStandardItemModel();
    if (!resourceManager(inventory)) return 1;
    
    MainWindow window(inventory);  // Receives pre-built model
    window.show();
    
    return app.exec();
}
```

---

## Multi-Phase Plan

### Phase 1: Add Callback Infrastructure ✅
**Goal:** Add callback-based scan methods without breaking existing code  
**Risk:** Low - Additive changes only  
**Status:** ✅ Complete - All tests passing (19/19)  

**Changes:**
- Add `ItemCallback` typedef to ResourceScanner
- Add `scanTemplates()` with callback parameter
- Add `scanTemplatesToList()` convenience wrapper (for tests)
- Add `scanTemplatesToModel()` convenience wrapper (for production)
- Add private `addItemToModel()` helper
- Keep existing `scanTemplates()` method temporarily

**Files Modified:**
- `src/resourceScanning/resourceScanner.hpp`
- `src/resourceScanning/resourceScanner.cpp`

**Tests:** Unit test callback-based scanTemplatesToList() returns expected items

---

### Phase 2: Add scanToModel() Public API
**Goal:** Add high-level method for main.cpp to call  
**Risk:** Low - New method, doesn't affect existing code  

**Changes:**
- Add `scanToModel(QStandardItemModel* model)` method
- Iterates all locations, all resource types
- Uses callback pattern internally

**Files Modified:**
- `src/resourceScanning/resourceScanner.hpp`
- `src/resourceScanning/resourceScanner.cpp`

**Tests:** Create test that verifies model is populated correctly

---

### Phase 3: Update main.cpp
**Goal:** Move scanning from MainWindow to main()  
**Risk:** Medium - Changes application startup flow  

**Changes:**
- Add `resourceManager()` function to main.cpp
- Call `discoverAllLocations()` from platformInfo
- Create ResourceScanner, call scanToModel()
- Update MainWindow constructor to accept `QStandardItemModel*`

**Files Modified:**
- `src/app/main.cpp`
- `src/app/mainwindow.hpp`
- `src/app/mainwindow.cpp`

**Tests:** Manual test - app should start and show resources

---

### Phase 4: Delete ResourceInventoryManager
**Goal:** Remove architecture violation  
**Risk:** Low - Should have no remaining callers  

**Changes:**
- Delete ResourceInventoryManager class entirely
- Update MainWindow to not use it

**Files Deleted:**
- Methods from `src/resourceScanning/resourceScanner.hpp` (ResourceInventoryManager section)
- Implementation from `src/resourceScanning/resourceScanner.cpp`

**Tests:** Build should succeed, no references to ResourceInventoryManager

---

### Phase 5: Clean Up Legacy Methods
**Goal:** Remove old non-callback scan methods  
**Risk:** Low - Replaced by callback versions  

**Changes:**
- Remove old `scanTemplates()` that returns QVector
- Remove `scanToTree()` methods (GUI coupling)
- Remove `scanAllTiers()` methods
- Update any remaining callers to use callback versions

**Files Modified:**
- `src/resourceScanning/resourceScanner.hpp`
- `src/resourceScanning/resourceScanner.cpp`
- Any test files using old methods

**Tests:** All existing tests should pass with callback-based methods

---

### Phase 6: Extend to All Resource Types
**Goal:** Apply callback pattern to Examples, Libraries, Fonts, etc.  
**Risk:** Medium - More complex scan logic (Libraries especially)  

**Changes:**
- Convert `scanExamples()` to callback pattern
- Convert `scanLibraries()` to callback pattern (hierarchical)
- Convert `scanFonts()` to callback pattern
- Convert `scanColorSchemes()` to callback pattern
- Convert `scanTests()` to callback pattern
- Update `scanToModel()` to call all types

**Files Modified:**
- `src/resourceScanning/resourceScanner.cpp` (all scan methods)

**Tests:** Verify each resource type callback works correctly

---

## Decision Log

**2026-01-13:** Chose callback pattern over optional capture parameter
- **Rationale:** More flexible, enables progressive UI updates, logging, filtering
- **Trade-off:** Slightly more complex to test (need lambda wrappers)
- **Benefit:** Zero memory waste in production, full testability preserved

**2026-01-13:** Keep existing scan methods during Phase 1
- **Rationale:** Allows incremental migration, reduces risk
- **Plan:** Remove in Phase 5 after all code migrated

**2026-01-13:** Use QStandardItemModel with custom roles (flat storage)
- **Rationale:** test_model_indexing.cpp proved this works well
- **Alternative:** Hierarchical tree model (rejected - too complex)
- **Benefit:** Can use QSortFilterProxyModel for multiple "indexes"

---

## Testing Strategy

### Unit Tests (Callback Verification)
```cpp
TEST(ResourceScannerTest, CallbackReceivesAllItems) {
    QVector<ResourceItem> captured;
    
    scanner.scanTemplates(testPath, tier, key, [&captured](const ResourceItem& item) {
        captured.append(item);
    });
    
    EXPECT_EQ(captured.size(), 5);
    EXPECT_EQ(captured[0].type(), ResourceType::Templates);
}
```

### Integration Tests (Model Population)
```cpp
TEST(ResourceScannerTest, PopulatesModelCorrectly) {
    QStandardItemModel model;
    
    scanner.scanTemplatesToModel(testPath, tier, key, &model);
    
    EXPECT_EQ(model.rowCount(), 5);
    EXPECT_EQ(model.item(0)->data(TypeRole).toInt(), 
              static_cast<int>(ResourceType::Templates));
}
```

### Manual Tests
- App starts without errors
- Resources visible in GUI
- Can filter by type using proxy models

---

## Rollback Plan

**If Phase N fails:**
1. Revert commits for Phase N
2. Keep earlier phases (they're additive)
3. Review what broke
4. Fix and retry OR skip phase

**Git Tags:**
- `phase-1-callbacks-added` - After Phase 1 success
- `phase-2-scantomodel` - After Phase 2 success
- `phase-3-main-refactored` - After Phase 3 success
- `phase-4-invmgr-deleted` - After Phase 4 success
- `phase-5-cleanup-done` - After Phase 5 success
- `phase-6-complete` - Full refactor complete

---

## Estimated Timeline

- **Phase 1:** 3 hours (add callbacks, test)
- **Phase 2:** 2 hours (scanToModel, test)
- **Phase 3:** 3 hours (main.cpp, MainWindow, test)
- **Phase 4:** 1 hour (delete ResourceInventoryManager)
- **Phase 5:** 2 hours (clean up old methods)
- **Phase 6:** 5 hours (extend to all resource types)

**Total:** 16 hours (2 days)

---

## Next Steps

1. ✅ Review this planning document
2. Get user approval to proceed
3. Implement Phase 1 (add callback infrastructure)
4. Commit with: `docs: Phase 1 planning complete for callback scanner refactor`
5. Proceed to Phase 1 implementation

**Ready to proceed with Phase 1?**
