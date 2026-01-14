# QVariant-Based Resource Architecture Refactor

**Date:** 2026-01-14  
**Author:** GitHub Copilot (Agent) with User Direction  
**Status:** Planning  

---

## User Requirements

**Original Problem Statement:**
> "when scanning for attachments the results were being passed as ResourceScript items .. if so that would be wrong .. you need another type for attachments that tracks their filename (with its extension) and its mime-type (if that is known)
>
> we should be basing our polymorphic resource objects on QVariant yes? so it should be a QList<QVariant> .. unless i misunderstood something"

**Key Requirements:**
1. Fix object slicing problem discovered in attachment scanning diagnostic test
2. Create separate type for attachments (not ResourceScript)
3. Use QVariant for polymorphic resource storage
4. Implement proper Qt-idiomatic architecture
5. Preserve type information through callback/storage pipeline

---

## Problem Statement

### Current Architecture Issues

**1. Object Slicing Problem**

Currently discovered via diagnostic test `test_attachment_scanning.exe`:
```
Found: customizer-all
Type: 2
Category: Parametric
Is ResourceScript: NO (dynamic_cast failed)
FAIL!  : AttachmentScanTest::testParametricExampleHasAttachment() 
         customizer-all should be a ResourceScript
```

**Root Cause:**
```cpp
// Current architecture
using ItemCallback = std::function<void(const ResourceItem&)>;

// In scanner
ResourceScript script = scanScriptWithAttachments(...);
onItemFound(script);  // ❌ Object slicing here!

// In consumer
QList<ResourceItem> results;
results.append(item);  // ❌ Object slicing again!

// Derived class data (attachments) is LOST
```

**2. Attachments Misrepresented**

Attachments are currently added to `ResourceScript` via `addAttachment()`, but:
- When stored in `QList<ResourceItem>`, the `ResourceScript` subclass is sliced to `ResourceItem`
- `attachments()` method is lost
- Attachment data vanishes from the model

**3. Callback Pattern Incompatible with Inheritance**

The callback-based scanner pattern passes `const ResourceItem&`, which:
- Cannot preserve derived class information
- Causes slicing when copying to containers
- Makes polymorphism impossible

### Why This Matters

**Current Impact:**
- ✅ Resource scanning works
- ✅ Tree structure displays correctly
- ❌ Attachments never appear in UI
- ❌ ResourceScript-specific data lost
- ❌ Cannot extend with new resource types safely

**Future Impact:**
- Cannot add resource-type-specific properties
- Model/View integration will fail for complex data
- Unit tests passing but UI behavior broken (hidden bug)

---

## Proposed Solution: QVariant-Based Architecture

### Qt's QVariant System

**What is QVariant?**

From Qt Documentation:
> "A QVariant object holds a single value of a single type at a time. You can find out what type, T, the variant holds, convert it to a different type using convert(), get its value using one of the toT() functions, and check whether the type can be converted to a particular type using canConvert()."

**Key Features for Our Use Case:**

1. **Type-Safe Storage:**
   ```cpp
   QVariant variant;
   variant.setValue(myResourceScript);  // Stores FULL object
   
   if (variant.canConvert<ResourceScript>()) {
       ResourceScript script = variant.value<ResourceScript>();  // Retrieves FULL object
   }
   ```

2. **Integration with Model/View:**
   - `QAbstractItemModel::data()` returns `QVariant`
   - `QAbstractItemModel::setData()` accepts `QVariant`
   - `QStandardItem::setData()` stores `QVariant`
   - Built-in role system uses `QVariant`

3. **Custom Type Registration:**
   ```cpp
   Q_DECLARE_METATYPE(ResourceScript);
   Q_DECLARE_METATYPE(ResourceAttachment);
   ```

### Architectural Design

**1. New Type Hierarchy**

```cpp
// Base type (still exists for common interface)
class ResourceItem {
public:
    ResourceItem();
    ResourceItem(const ResourceItem& other);  // Copy constructor
    ResourceItem& operator=(const ResourceItem& other);  // Assignment
    
    ResourceType type() const;
    QString displayName() const;
    QString category() const;
    // ... common properties ...
private:
    ResourceType m_type;
    QString m_displayName;
    QString m_category;
    // No virtual functions needed!
};
Q_DECLARE_METATYPE(ResourceItem);

// Script type with attachments
class ResourceScript {
public:
    ResourceScript();
    ResourceScript(const ResourceScript& other);
    ResourceScript& operator=(const ResourceScript& other);
    
    // All ResourceItem properties
    ResourceType type() const { return ResourceType::Examples; }
    QString displayName() const { return m_displayName; }
    QString category() const { return m_category; }
    
    // Script-specific properties
    QString scriptPath() const { return m_scriptPath; }
    QStringList attachments() const { return m_attachments; }
    void addAttachment(const QString& path) { m_attachments.append(path); }
    bool hasAttachments() const { return !m_attachments.isEmpty(); }
    
private:
    QString m_displayName;
    QString m_category;
    QString m_scriptPath;
    QStringList m_attachments;  // Preserved!
};
Q_DECLARE_METATYPE(ResourceScript);

// NEW: Attachment type
struct ResourceAttachment {
    QString filePath;
    QString fileName;
    QString extension;
    QString mimeType;
    QString parentScriptPath;  // Which script does this belong to?
    
    ResourceType type() const { return ResourceType::Attachment; }
    QString displayName() const { return fileName; }
};
Q_DECLARE_METATYPE(ResourceAttachment);
```

**Design Rationale:**
- No inheritance = no virtual functions = no slicing
- Each type stands alone with its own properties
- Common interface through same method names (duck typing)
- QVariant handles type discrimination

**2. Storage Change**

```cpp
// OLD (slices objects)
QList<ResourceItem> results;

// NEW (preserves full type)
QList<QVariant> results;
```

**3. Callback Signature Change**

```cpp
// OLD (causes slicing)
using ItemCallback = std::function<void(const ResourceItem&)>;

// NEW (preserves type)
using ItemCallback = std::function<void(const QVariant&)>;
```

**4. Type Checking Pattern**

```cpp
void processResource(const QVariant& var) {
    // Check what type we have
    if (var.canConvert<ResourceScript>()) {
        ResourceScript script = var.value<ResourceScript>();
        qDebug() << "Script:" << script.displayName();
        
        if (script.hasAttachments()) {
            qDebug() << "Attachments:" << script.attachments();
        }
    }
    else if (var.canConvert<ResourceAttachment>()) {
        ResourceAttachment att = var.value<ResourceAttachment>();
        qDebug() << "Attachment:" << att.fileName 
                 << "Type:" << att.mimeType;
    }
    else if (var.canConvert<ResourceItem>()) {
        ResourceItem item = var.value<ResourceItem>();
        qDebug() << "Generic:" << item.displayName();
    }
}
```

### Qt Integration Points

**1. Model/View Integration**

Models already use QVariant:
```cpp
QVariant MyModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        QVariant var = m_data.at(index.row());
        
        if (var.canConvert<ResourceScript>()) {
            return var.value<ResourceScript>().displayName();
        }
        // ... handle other types ...
    }
    else if (role == Qt::UserRole) {
        // Return the full QVariant for custom roles
        return m_data.at(index.row());
    }
    return QVariant();
}
```

**2. QStandardItemModel Usage**

```cpp
void addItemToModel(QStandardItemModel* model, const QVariant& var) {
    auto* item = new QStandardItem();
    
    if (var.canConvert<ResourceScript>()) {
        ResourceScript script = var.value<ResourceScript>();
        item->setText(script.displayName());
        item->setData(var, Qt::UserRole);  // Store full object
        
        // Add attachment subitems
        if (script.hasAttachments()) {
            for (const QString& attPath : script.attachments()) {
                ResourceAttachment att;
                att.filePath = attPath;
                att.fileName = QFileInfo(attPath).fileName();
                att.extension = QFileInfo(attPath).suffix();
                att.mimeType = detectMimeType(attPath);
                
                auto* attItem = new QStandardItem();
                attItem->setText(att.fileName);
                attItem->setData(QVariant::fromValue(att), Qt::UserRole);
                item->appendRow(attItem);
            }
        }
    }
    
    model->appendRow(item);
}
```

**3. Scanner Implementation**

```cpp
void ResourceScanner::scanExamples(
    const QString& basePath,
    ResourceTier tier,
    const QString& locationKey,
    ItemCallback onItemFound)
{
    QDir dir(basePath);
    if (!dir.exists()) return;
    
    // Scan scripts
    QFileInfoList files = dir.entryInfoList({"*.scad"}, QDir::Files);
    for (const QFileInfo& fi : files) {
        ResourceScript script = scanScriptWithAttachments(fi.absoluteFilePath(), ...);
        
        // Pass as QVariant - no slicing!
        QVariant var = QVariant::fromValue(script);
        onItemFound(var);
        
        // OPTIONAL: Also emit attachments as separate items
        if (script.hasAttachments()) {
            for (const QString& attPath : script.attachments()) {
                ResourceAttachment att;
                att.filePath = attPath;
                att.fileName = QFileInfo(attPath).fileName();
                att.extension = QFileInfo(attPath).suffix();
                att.mimeType = detectMimeType(attPath);
                att.parentScriptPath = script.scriptPath();
                
                QVariant attVar = QVariant::fromValue(att);
                onItemFound(attVar);
            }
        }
    }
}
```

---

## Multi-Phase Implementation Plan

### Phase 1: Type System Foundation
**Goal:** Register types with Qt meta-object system, add Q_DECLARE_METATYPE

**Changes:**
1. Add `Q_DECLARE_METATYPE(ResourceItem)` after class declaration
2. Add `Q_DECLARE_METATYPE(ResourceScript)` after class declaration
3. Create new `ResourceAttachment` struct
4. Add `Q_DECLARE_METATYPE(ResourceAttachment)` 
5. Ensure all types have proper copy constructors and assignment operators
6. Add `ResourceType::Attachment` to enum

**Files Modified:**
- `src/resourceInventory/resourceItem.hpp`
- `src/resourceInventory/resourceItem.cpp`
- `src/resourceMetadata/ResourceTypeInfo.hpp`

**Testing:**
```cpp
TEST(QVariantRegistration, ResourceItemCanBeStored) {
    ResourceItem item(ResourceType::Examples, "test");
    QVariant var = QVariant::fromValue(item);
    ASSERT_TRUE(var.canConvert<ResourceItem>());
    
    ResourceItem retrieved = var.value<ResourceItem>();
    ASSERT_EQ(retrieved.displayName(), "test");
}

TEST(QVariantRegistration, ResourceScriptPreservesAttachments) {
    ResourceScript script;
    script.addAttachment("/path/to/file.json");
    
    QVariant var = QVariant::fromValue(script);
    ASSERT_TRUE(var.canConvert<ResourceScript>());
    
    ResourceScript retrieved = var.value<ResourceScript>();
    ASSERT_TRUE(retrieved.hasAttachments());
    ASSERT_EQ(retrieved.attachments().size(), 1);
}
```

**Risk:** Low - additive changes only

---

### Phase 2: Scanner Callback Conversion
**Goal:** Change scanner callbacks to use QVariant

**Changes:**
1. Change `ItemCallback` type from `std::function<void(const ResourceItem&)>` to `std::function<void(const QVariant&)>`
2. Update `scanExamples()` to emit `QVariant::fromValue(script)`
3. Update `scanTemplates()` to emit `QVariant::fromValue(item)`
4. Update `scanGroup()` to emit `QVariant::fromValue(script)`
5. Update ALL scanner methods to use new callback signature

**Files Modified:**
- `src/resourceScanning/resourceScanner.hpp`
- `src/resourceScanning/resourceScanner.cpp`
- `tests/resourceScanner_minimal.cpp`

**Testing Strategy:**
- Run all existing scanner tests
- Verify tests still pass (they should, as consumers can still extract ResourceItem)
- Add new tests that verify QVariant contains correct type

**Risk:** Medium - affects all scanner code

---

### Phase 3: Consumer/Storage Updates
**Goal:** Change storage containers from `QList<ResourceItem>` to `QList<QVariant>`

**Changes:**
1. `scanExamplesToList()` returns `QList<QVariant>`
2. `scanTemplatesToList()` returns `QList<QVariant>`
3. Update lambda captures: `[&results](const QVariant& var) { results.append(var); }`
4. Update ALL wrapper methods

**Files Modified:**
- `src/resourceScanning/resourceScanner.cpp`
- All consumer code that receives lists

**Testing:**
- Verify tree printing test now shows attachments
- Run attachment scanning diagnostic again - should PASS

**Risk:** Medium-High - affects data flow

---

### Phase 4: Model Integration
**Goal:** Update QStandardItemModel integration to use QVariant storage

**Changes:**
1. `addItemToModel()` takes `const QVariant&` instead of `const ResourceItem&`
2. Store full QVariant in `Qt::UserRole` of QStandardItem
3. Use `var.canConvert<Type>()` pattern for type discrimination
4. Add attachment subitems when script has attachments

**Files Modified:**
- `src/resourceScanning/resourceScanner.cpp` (`addItemToModel()` function)
- Model consumers

**Testing:**
- Verify tree view displays attachments as child nodes
- Verify item data retrieval works correctly
- Test with QTreeView and QTableView

**Risk:** Medium - affects UI

---

### Phase 5: Attachment Emission
**Goal:** Scanner emits attachments as separate ResourceAttachment items

**Changes:**
1. Implement `detectMimeType()` helper
2. Update scanners to emit both script AND attachment items
3. Add attachment type handling to model integration
4. Update tree tests to verify attachment items appear

**Files Modified:**
- `src/resourceScanning/resourceScanner.cpp`
- `tests/test_examples_tree.cpp`

**Testing:**
- Verify customizer-all.scad has customizer-all.json attachment visible
- Verify attachment has correct filename, extension, mime-type
- Verify parent-child relationship in tree

**Risk:** Low - additive feature

---

### Phase 6: ResourceItem Hierarchy Cleanup (OPTIONAL)
**Goal:** Remove inheritance, make ResourceItem non-polymorphic

**Rationale:**
- If using QVariant, we don't need virtual functions
- Can remove vtable overhead
- Simpler design without inheritance

**Changes:**
1. Remove `virtual` from ResourceItem methods
2. Remove inheritance relationship between ResourceScript and ResourceItem
3. Keep same method signatures for interface compatibility (duck typing)
4. Remove dynamic_cast usage (use QVariant instead)

**Risk:** Low - cleanup phase

---

## Decision Log

### Why QVariant Instead of Smart Pointers?

**Considered Alternatives:**
1. `QList<std::shared_ptr<ResourceItem>>`
2. `QList<QSharedPointer<ResourceItem>>`
3. `QList<ResourceItem*>` with manual memory management

**Why QVariant is Better:**
- ✅ Native Qt integration (Model/View already uses it)
- ✅ Value semantics (automatic memory management)
- ✅ No heap allocations for small types
- ✅ Built-in type checking (`canConvert()`, `value<T>()`)
- ✅ Serialization support (QDataStream)
- ✅ Works with Qt signals/slots
- ✅ No pointer lifetime management

**Smart Pointer Downsides:**
- ❌ Still requires inheritance/virtual functions
- ❌ Heap allocation overhead
- ❌ Reference counting overhead
- ❌ Doesn't solve the fundamental slicing problem in callbacks
- ❌ Less idiomatic for Qt

### Why Separate ResourceAttachment Type?

**Current Problem:**
Attachments were being added to ResourceScript, but this conflates two concepts:
- The script file itself
- Files related to the script

**Better Design:**
- `ResourceScript` represents the .scad file
- `ResourceAttachment` represents related files (.json, .png, etc.)
- Clear parent-child relationship
- Each type has appropriate properties

**Benefits:**
- ✅ Clearer semantics
- ✅ Can emit attachments as separate tree nodes
- ✅ Can have attachment-specific operations (preview image, edit JSON)
- ✅ Easier to query "all attachments" separately from "all scripts"

### Why Not Keep Inheritance with QVariant?

**Question:** Could we use `QVariant` to store `ResourceItem*` pointers?

**Answer:** Possible, but defeats the purpose:
```cpp
// This works but is NOT recommended
ResourceScript* script = new ResourceScript();
QVariant var = QVariant::fromValue<ResourceItem*>(script);
```

**Problems:**
- Still need virtual functions
- Still need manual memory management
- Still need to worry about object lifetime
- Loses QVariant value semantics

**Better approach:** Store objects by value in QVariant

---

## Technology Discoveries

### Qt Meta-Object System

**Q_DECLARE_METATYPE(Type):**
- Registers `Type` with Qt's meta-object system
- Enables storage in QVariant
- Enables use in signals/slots
- Enables queued connections
- Must be outside any namespace (or use full qualification)

**Requirements for Custom Types:**
- Must have public default constructor
- Must have public copy constructor
- Must have public destructor
- Must have public assignment operator

**Placement:**
```cpp
// In header file, AFTER class declaration
class MyType {
    // ...
};

Q_DECLARE_METATYPE(MyType);  // ← Here, not inside class
```

### QVariant::fromValue() vs setValue()

**fromValue() - Creates new QVariant:**
```cpp
ResourceScript script;
QVariant var = QVariant::fromValue(script);
```

**setValue() - Modifies existing QVariant:**
```cpp
QVariant var;
var.setValue(script);
```

Both work, `fromValue()` is more explicit for creation.

### Type Checking Patterns

**Safe Extraction:**
```cpp
if (variant.canConvert<MyType>()) {
    MyType obj = variant.value<MyType>();
    // Use obj safely
}
```

**Type Discrimination:**
```cpp
if (variant.userType() == qMetaTypeId<ResourceScript>()) {
    // It's definitely a ResourceScript
}
```

**Duck Typing Approach:**
```cpp
// Since our types have same method names, we could:
auto getText = [](const QVariant& var) -> QString {
    if (var.canConvert<ResourceScript>()) {
        return var.value<ResourceScript>().displayName();
    } else if (var.canConvert<ResourceAttachment>()) {
        return var.value<ResourceAttachment>().displayName();
    } else if (var.canConvert<ResourceItem>()) {
        return var.value<ResourceItem>().displayName();
    }
    return QString();
};
```

---

## Limitations & Constraints

### Performance Considerations

**QVariant Overhead:**
- Small types (< sizeof(void*) * 2) stored inline
- Larger types heap allocated
- Copy-on-write for large types
- Generally negligible overhead for our use case (100s of items, not millions)

**Measurement Plan:**
- Profile scanner performance before/after
- Measure model population time
- Verify tree rendering speed
- Target: < 5% regression acceptable

### Backward Compatibility

**Breaking Changes:**
- Callback signature changes (compile-time break)
- Storage type changes (compile-time break)
- Return type changes from scanner wrappers (compile-time break)

**No Runtime Compatibility Needed:**
- This is application code, not a library
- Can change everything at once

**Migration Strategy:**
- All changes in one branch
- All tests updated together
- No gradual migration needed

### Qt Version Requirements

**Minimum Qt Version:**
- Using Qt 6.10.1 (current)
- Q_DECLARE_METATYPE available since Qt 5.0
- QVariant::fromValue() available since Qt 4.5
- All features well-established ✅

---

## Testing Strategy

### Unit Tests

**Phase 1 Tests:**
- QVariant registration
- Type round-trip (store → retrieve → verify)
- Copy constructor correctness
- Attachment struct creation

**Phase 2 Tests:**
- Scanner callback receives QVariant
- Correct type stored in QVariant
- All scanner methods updated

**Phase 3 Tests:**
- List storage preserves types
- Attachment scanning works
- test_attachment_scanning.cpp PASSES ✅

**Phase 4 Tests:**
- Model stores QVariant correctly
- Qt::UserRole retrieval works
- Tree structure correct

**Phase 5 Tests:**
- Attachments appear as child nodes
- MIME type detection works
- Parent-child relationships correct

### Integration Tests

**Tree Printing Test:**
- Should now show attachments indented 2 levels
- Verify output format matches expected

**Scanner Tests:**
- All 92 existing tests should still pass
- Add new tests for QVariant behavior

**Model Tests:**
- Verify QStandardItemModel integration
- Test with QTreeView
- Test with QTableView

### Regression Testing

**Critical Paths:**
1. Resource discovery still finds all resources
2. Tree hierarchy still correct
3. Templates/Examples/Tests distinction preserved
4. Category (Group) handling still works

---

## Success Criteria

**Phase 1 Complete When:**
- ✅ All types registered with Q_DECLARE_METATYPE
- ✅ Copy constructors tested
- ✅ QVariant round-trip tests pass
- ✅ Build successful

**Phase 2 Complete When:**
- ✅ All scanner callbacks use QVariant signature
- ✅ All existing scanner tests pass
- ✅ Type preservation verified

**Phase 3 Complete When:**
- ✅ Storage uses `QList<QVariant>`
- ✅ test_attachment_scanning.cpp PASSES
- ✅ No object slicing occurs

**Phase 4 Complete When:**
- ✅ Model integration uses QVariant
- ✅ Tree view displays correctly
- ✅ Data retrieval works

**Phase 5 Complete When:**
- ✅ Attachments visible in tree output
- ✅ customizer-all.json appears under customizer-all.scad
- ✅ MIME types correct

**Overall Success:**
- ✅ All 92+ tests passing
- ✅ Attachments display in UI
- ✅ No object slicing
- ✅ Qt-idiomatic architecture
- ✅ < 5% performance regression
- ✅ Documentation complete

---

## Rollback Plan

**If Phase Fails:**
1. Revert to last known-good commit (tagged)
2. Review results document for root cause
3. Adjust plan based on findings
4. Retry or pivot to alternative

**Each phase has rollback point:**
- Phase 1: Revert type registrations
- Phase 2: Revert callback signatures
- Phase 3: Revert storage types
- Phase 4: Revert model changes
- Phase 5: Revert attachment emission

**Git Tags:**
- `qvariant-phase1-complete`
- `qvariant-phase2-complete`
- etc.

---

## Open Questions

1. **Should attachments be emitted as separate items or only stored in parent script?**
   - Separate: Better for filtering, searching
   - Stored only: Simpler tree structure
   - **Decision:** Emit separately for phase 5, can be toggled

2. **How to handle MIME type detection?**
   - QMimeDatabase (Qt built-in)
   - Extension-based lookup
   - **Decision:** QMimeDatabase with fallback to extension

3. **Should ResourceItem base class remain?**
   - Keep for common interface
   - Remove for pure duck typing
   - **Decision:** Keep initially, remove in optional Phase 6

4. **Performance impact acceptable?**
   - Profile before/after
   - **Decision:** Measure in Phase 3, pivot if > 10% regression

---

## Next Steps

**Immediate:**
1. User reviews this planning document
2. User approves approach OR suggests changes
3. Create Phase 1 implementation
4. Run Phase 1 tests
5. Create Phase 1 results document
6. Get user approval for Phase 2

**Do NOT proceed with implementation until user approves this plan.**

---

## References

- [Qt Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html)
- [QVariant Class Documentation](https://doc.qt.io/qt-6/qvariant.html)
- [Q_DECLARE_METATYPE Documentation](https://doc.qt.io/qt-6/qmetatype.html#Q_DECLARE_METATYPE)
- [Creating Custom Qt Types](https://doc.qt.io/qt-6/custom-types.html)
- Project: `doc/2025-12-28-resource-location-enablement-refactoring.md` (prior refactor example)
