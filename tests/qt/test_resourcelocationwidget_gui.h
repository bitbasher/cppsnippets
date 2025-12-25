/**
 * @file test_resourcelocationwidget_gui.h
 * @brief GUI tests for ResourceLocationWidget using QtTestLib
 * 
 * Tests cover:
 * - Location list display
 * - Add/Remove location operations
 * - Enable/Disable checkboxes
 * - Rescan button functionality
 */

#pragma once

#include <QObject>
#include <QTest>

class ResourceLocationWidget;

class TestResourceLocationWidgetGUI : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // Display tests
    void testLocationsAreDisplayed();
    void testLocationCountIsCorrect();
    void testLocationNamesAreAccurate();
    void testDisabledLocationsAreGrayed();
    
    // Checkbox tests
    void testCheckboxesForValidLocations();
    void testNoCheckboxForInvalidLocations();
    void testCheckBoxStateReflectsEnabled();
    void testToggleCheckbox();
    
    // Add location tests
    void testAddLocationSucceeds();
    void testAddLocationAppears InList();
    void testAddLocationEmitSignal();
    void testAddLocationValidatesPath();
    void testAddLocationRejectsDuplicate();
    void testAddLocationClearsInput();
    
    // Remove location tests
    void testRemoveButtonDisabledWithoutSelection();
    void testRemoveButtonEnabledWithSelection();
    void testRemoveLocationSucceeds();
    void testRemoveLocationEmitsSignal();
    
    // Rescan button tests
    void testRescanButtonExists();
    void testRescanButtonEmitsSignal();
    void testRescanButtonDisabledInReadOnlyMode();
    
    // Read-only mode tests
    void testReadOnlyDisablesInput();
    void testReadOnlyDisablesRemoveButton();
    void testReadOnlyDisablesRescanButton();
    void testReadOnlyDisablesCheckboxes();
    
    // Input widget tests
    void testInputWidgetVisible();
    void testInputWidgetCanBeHidden();
    void testPathInputAcceptsText();
    void testNameInputAcceptsText();

private:
    ResourceLocationWidget* m_widget = nullptr;
};
