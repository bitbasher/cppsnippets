<!-- markdownlint-disable -->
# GUI Regression Testing Guide

This document explains how to use the QtTestLib-based GUI tests for regression testing after UI updates.

## Overview

Three comprehensive GUI test suites have been created following Qt's TestLib tutorial patterns (Chapter 3-4):

1. **test_mainwindow_gui** - Tests for MainWindow template browser UI
2. **test_envvarstab_gui** - Tests for EnvVarsTab environment variable editor
3. **test_resourcelocationwidget_gui** - Tests for ResourceLocationWidget location management

## Test Framework Approach

The tests follow QtTestLib best practices from the official Qt documentation:

### Key Patterns Used

**Chapter 3 Pattern: GUI Event Simulation**
- `QTest::keyClicks()` - Simulate typing text into widgets
- `QTest::mouseClick()` - Simulate mouse clicks on buttons
- `QCOMPARE()` - Verify widget state and values
- `QVERIFY()` - Check boolean conditions

**Chapter 4 Pattern: Event Recording & Data-Driven Testing**
- `QTestEventList` - Record sequences of GUI events
- `QTest::newRow()` - Create parametrized test data sets
- Multiple test runs with different input combinations

## Building the Tests

### Option 1: Build All Tests

```bash
cd build
cmake --build . --target tests
```

### Option 2: Build Individual Test

```bash
# MainWindow tests
cmake --build . --target test_mainwindow_gui

# EnvVarsTab tests
cmake --build . --target test_envvarstab_gui

# ResourceLocationWidget tests
cmake --build . --target test_resourcelocationwidget_gui
```

## Running the Tests

### Option 1: Using CTest (Recommended)

```bash
cd build
ctest -V  # -V for verbose output
```

Run specific test:
```bash
ctest -R test_mainwindow_gui -V
```

### Option 2: Run Test Executable Directly

```bash
./bin/test_mainwindow_gui
./bin/test_envvarstab_gui
./bin/test_resourcelocationwidget_gui
```

### Option 3: CMake Test Target

```bash
cmake --build . --target test_mainwindow_gui
cmake --build . --target test
ctest --verbose
```

## Test Suites

### 1. MainWindow GUI Tests (`test_mainwindow_gui`)

**Purpose**: Validate template browser, search, and editor functionality

**Key Test Cases**:

#### Search Tests
- `testSearchFindTemplate` - Search locates templates by name
- `testSearchWrapAround` - Search wraps to beginning when reaching end
- `testSearchNoMatch` - Handles non-matching search gracefully
- `testSearchCaseSensitivity` - Search is case-insensitive
- `testSearchWithSpaces` - Handles spaces in search queries
- `testSearchClearSelection` - Clearing search clears selection

#### Button State Tests
- `testButtonStateOnStartup` - New button enabled, Edit/Delete disabled
- `testButtonStateOnUserTemplateSelection` - Edit/Delete enabled for User tier only
- `testButtonStateOnMachineTemplateSelection` - Edit/Delete disabled for Machine tier
- `testButtonStateOnInstallationTemplateSelection` - Edit/Delete disabled for Installation tier
- `testButtonStateOnEdit` - Save/Cancel enabled in edit mode
- `testButtonStateOnCancel` - Buttons return to normal state

#### New Template Tests
- `testNewTemplateInitialization` - Fields cleared and initialized
- `testNewTemplateSaveDestination` - Destination set to User tier location
- `testNewTemplateFieldsReadOnly` - Fields become editable

#### Copy Template Tests
- `testCopyTemplateName` - Appends "-copy" suffix to name
- `testCopyTemplateContent` - Preserves description and source
- `testCopyTemplateSaveDestination` - Sets save destination

#### Editor Tests
- `testEditorPopulationFromSelection` - Selection fills editor fields
- `testEditorDescription` - Description loaded from JSON
- `testEditorVersion` - Version field populated

### 2. EnvVarsTab GUI Tests (`test_envvarstab_gui`)

**Purpose**: Validate environment variable editor functionality

**Key Test Cases**:

#### List Population Tests
- `testListPopulatesFromSystemEnv` - System env vars loaded
- `testListPopulatesFromOverrides` - Saved overrides loaded
- `testListDoesNotDuplicate` - No duplicate entries
- `testListIsSorted` - List is alphabetically sorted

#### Selection and Display Tests
- `testSelectionPopulatesEditor` - Selecting var fills editor
- `testEditorShowsCorrectValue` - Values match selection
- `testPreviewUpdates` - Preview updates on value change
- `testPreviewHandlesEmptyValue` - Empty values handled gracefully

#### Save Tests
- `testSaveNewVariable` - New variables saved correctly
- `testSaveModifiedVariable` - Modifications saved
- `testSavePersistsToSettings` - Changes persist in settings
- `testSaveValidatesName` - Invalid names rejected
- `testSaveRejectsDuplicateName` - Duplicates prevented

#### Cancel Tests
- `testCancelClearsChanges` - Changes discarded
- `testCancelReloadsOriginalValue` - Original values restored

#### Copy Tests
- `testCopyAddsVariableWithSuffix` - Creates _copy suffix
- `testCopyPreservesValue` - Values preserved
- `testCopyHandlesMultipleCopies` - Multiple copies handled

#### Revert Tests
- `testRevertUserDefinedVariable` - User vars can be deleted
- `testRevertOverriddenSystemVariable` - System vars can be reverted
- `testRevertPromptAppears` - Dialogs show appropriately

#### Reload Tests
- `testReloadRefreshesFromSettings` - Data reloaded
- `testReloadClearsChanges` - Unsaved changes discarded

#### Button State Tests
- `testButtonStatesOnStartup` - Correct initial states
- `testCopyButtonDisabledWithoutSelection` - Copy disabled when needed
- `testRevertButtonDisabledForNewVariable` - Revert disabled for new vars

### 3. ResourceLocationWidget GUI Tests (`test_resourcelocationwidget_gui`)

**Purpose**: Validate location list management functionality

**Key Test Cases**:

#### Display Tests
- `testLocationsAreDisplayed` - Locations appear in list
- `testLocationCountIsCorrect` - Correct number displayed
- `testLocationNamesAreAccurate` - Names match data
- `testDisabledLocationsAreGrayed` - Visual indication of disabled items

#### Checkbox Tests
- `testCheckboxesForValidLocations` - Valid locations have checkboxes
- `testNoCheckboxForInvalidLocations` - Invalid locations don't
- `testCheckBoxStateReflectsEnabled` - State matches enabled status
- `testToggleCheckbox` - Checkbox state changes work

#### Add Location Tests
- `testAddLocationSucceeds` - Locations can be added
- `testAddLocationAppearsInList` - Added item appears
- `testAddLocationEmitSignal` - Signal emitted
- `testAddLocationValidatesPath` - Path validation works
- `testAddLocationRejectsDuplicate` - Duplicates prevented
- `testAddLocationClearsInput` - Input cleared after add

#### Remove Location Tests
- `testRemoveButtonDisabledWithoutSelection` - Disabled without selection
- `testRemoveButtonEnabledWithSelection` - Enabled with selection
- `testRemoveLocationSucceeds` - Locations can be removed
- `testRemoveLocationEmitsSignal` - Signal emitted

#### Rescan Button Tests
- `testRescanButtonExists` - Button present
- `testRescanButtonEmitsSignal` - Signal emitted on click
- `testRescanButtonDisabledInReadOnlyMode` - Respects read-only

#### Read-Only Mode Tests
- `testReadOnlyDisablesInput` - Input disabled
- `testReadOnlyDisablesRemoveButton` - Remove button disabled
- `testReadOnlyDisablesRescanButton` - Rescan button disabled
- `testReadOnlyDisablesCheckboxes` - Checkboxes disabled

#### Input Widget Tests
- `testInputWidgetVisible` - Input shown by default
- `testInputWidgetCanBeHidden` - Visibility toggles
- `testPathInputAcceptsText` - Text input works
- `testNameInputAcceptsText` - Text input works

## Regression Testing Workflow

### After UI Changes

1. **Identify affected component**:
   - MainWindow → Run `test_mainwindow_gui`
   - EnvVarsTab → Run `test_envvarstab_gui`
   - ResourceLocationWidget → Run `test_resourcelocationwidget_gui`

2. **Build and run tests**:
   ```bash
   cd build
   cmake --build . --target test_mainwindow_gui
   ./bin/test_mainwindow_gui
   ```

3. **Review failures**:
   - Output shows PASS/FAIL for each test
   - Failed tests indicate regression
   - Fix code or update tests as appropriate

4. **Run full test suite**:
   ```bash
   cmake --build . --target tests
   ctest --verbose
   ```

### Test Output Format

```
Testing test_mainwindow_gui...
PASS : TestMainWindowGUI::initTestCase()
PASS : TestMainWindowGUI::testSearchFindTemplate()
FAIL : TestMainWindowGUI::testButtonStateOnStartup()
  Details: QTest assertion failed at test_mainwindow_gui.cpp:142
  Expected: new button enabled
  Actual: new button disabled
PASS : TestMainWindowGUI::testNewTemplateInitialization()
...
Totals: 45 passed, 1 failed, 0 skipped
```

## Best Practices

### When Tests Fail

1. **Understand the failure**:
   - Read the assertion message
   - Check which GUI interaction failed
   - Verify the expected behavior

2. **Fix the code first**:
   - Most failures indicate actual bugs
   - Fix the implementation, not the test
   - Only update tests if behavior intentionally changed

3. **Update tests if needed**:
   - If behavior changed intentionally, update test expectations
   - Add new tests for new features
   - Keep tests focused and maintainable

### Writing New Tests

Follow the established patterns:

```cpp
// Test structure
void TestClassName::testFeatureName() {
    // 1. Get widget references
    QLineEdit* input = widget->findChild<QLineEdit*>("m_input");
    QPushButton* button = widget->findChild<QPushButton*>("m_button");
    
    QVERIFY(input != nullptr);
    QVERIFY(button != nullptr);
    
    // 2. Simulate user action
    QTest::keyClicks(input, "test");
    QTest::mouseClick(button, Qt::LeftButton);
    QTest::qWait(100);  // Let signals process
    
    // 3. Verify result
    QCOMPARE(input->text(), QString("test"));
    QVERIFY(expectedCondition);
}
```

### Test Naming Convention

- `test<Feature><Scenario>` - e.g., `testSearchFindTemplate`
- Use descriptive names that explain what's tested
- Group related tests with similar prefixes

## Continuous Integration

To integrate into CI/CD pipeline:

```bash
# Build and run all GUI tests
cmake --build . --target tests
ctest --verbose --output-on-failure

# Check exit code
if [ $? -eq 0 ]; then
    echo "All tests passed"
else
    echo "Test failures detected"
    exit 1
fi
```

## Troubleshooting

### Tests Won't Compile

**Problem**: Missing includes or undefined references

**Solution**:
1. Ensure Qt6::Test and Qt6::Widgets are linked
2. Check include paths point to src directory
3. Verify object names match in tested widgets

### Tests Hang or Crash

**Problem**: Infinite loops or access violations

**Solution**:
1. Add `QTest::qWait()` after GUI interactions
2. Check for null pointers in findChild()
3. Verify widget initialization in `init()`

### Platform-Specific Failures

**Problem**: Tests pass on one OS but fail on another

**Solution**:
1. Check file path separators (/ vs \)
2. Verify path case sensitivity
3. Test on target platform before committing

## Further Reading

- [Qt TestLib Documentation](https://doc.qt.io/qt-6/qttest-index.html)
- [Qt Test Tutorial](https://doc.qt.io/qt-6/qtest-tutorial.html)
- Chapter 3: Simulating GUI Events
- Chapter 4: Replaying GUI Events
- Chapter 5: Writing a Benchmark

## Test Maintenance

### When to Update Tests

- ✅ Intentional UI behavior changes
- ✅ New features added
- ❌ Bug fixes (test shouldn't change, code should)
- ❌ Refactoring (tests validate behavior, not implementation)

### Keeping Tests Maintainable

1. **Use object names** - `findChild<>(m_objectName)`
2. **Add comments** - Explain non-obvious test logic
3. **Keep tests small** - One feature per test
4. **DRY principle** - Extract common setup to helpers
5. **Review regularly** - Update as UI evolves

---

**Last Updated**: December 2025
**Test Framework**: QtTestLib (Qt 6.10+)
**Coverage**: 115+ GUI test cases across 3 suites
