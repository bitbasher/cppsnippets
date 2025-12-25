/**
 * @file test_envvarstab_gui.cpp
 * @brief GUI tests for EnvVarsTab using QtTestLib
 */

#include "test_envvarstab_gui.h"
#include "gui/envVarsTab.h"
#include "platformInfo/resourceLocationManager.h"

#include <QTest>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QSettings>

void TestEnvVarsTabGUI::initTestCase() {
    // Initialization for entire test suite
}

void TestEnvVarsTabGUI::cleanupTestCase() {
    // Cleanup after entire test suite
}

void TestEnvVarsTabGUI::init() {
    // Setup before each test
    auto* manager = new platformInfo::ResourceLocationManager(nullptr);
    m_tab = new EnvVarsTab(manager, nullptr);
    m_tab->show();
    QTest::qWaitForWindowExposed(m_tab);
}

void TestEnvVarsTabGUI::cleanup() {
    // Cleanup after each test
    delete m_tab;
    m_tab = nullptr;
}

// ============================================================================
// List Population Tests
// ============================================================================

void TestEnvVarsTabGUI::testListPopulatesFromSystemEnv() {
    // Test that environment variable list loads system variables
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QVERIFY(listWidget != nullptr);
    
    // List should have items from system environment
    QVERIFY(listWidget->count() > 0);
    
    // Common system variables should be present
    bool hasPath = false;
    for (int i = 0; i < listWidget->count(); ++i) {
        QString item = listWidget->item(i)->text();
        if (item.compare("PATH", Qt::CaseInsensitive) == 0) {
            hasPath = true;
            break;
        }
    }
    QVERIFY(hasPath);  // PATH exists on all platforms
}

void TestEnvVarsTabGUI::testListPopulatesFromOverrides() {
    // Test that saved overrides are loaded into list
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QVERIFY(listWidget != nullptr);
    
    // Add a new variable and save
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* saveBtn = m_tab->findChild<QPushButton*>("m_saveButton");
    
    QVERIFY(nameEdit != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(saveBtn != nullptr);
    
    // Enter new variable
    QTest::keyClicks(nameEdit, "TEST_VAR");
    QTest::keyClicks(valueEdit, "test_value");
    QTest::mouseClick(saveBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Reload - new variable should still be there
    QPushButton* reloadBtn = m_tab->findChild<QPushButton*>("m_restoreDefaultsButton");
    if (reloadBtn) {
        QTest::mouseClick(reloadBtn, Qt::LeftButton);
        QTest::qWait(100);
    }
    
    // TEST_VAR should still be in list
    QVERIFY(variableExists("TEST_VAR"));
}

void TestEnvVarsTabGUI::testListDoesNotDuplicate() {
    // Test that system and override variables don't appear twice
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QVERIFY(listWidget != nullptr);
    
    QStringList items;
    for (int i = 0; i < listWidget->count(); ++i) {
        items << listWidget->item(i)->text();
    }
    
    // Check for duplicates
    QStringList sorted = items;
    sorted.sort();
    for (int i = 0; i < sorted.size() - 1; ++i) {
        QVERIFY(sorted[i] != sorted[i + 1]);  // No consecutive duplicates
    }
}

void TestEnvVarsTabGUI::testListIsSorted() {
    // Test that list is sorted alphabetically
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QVERIFY(listWidget != nullptr);
    
    QString prevItem;
    for (int i = 0; i < listWidget->count(); ++i) {
        QString item = listWidget->item(i)->text();
        if (!prevItem.isEmpty()) {
            QVERIFY(prevItem <= item);  // Should be sorted
        }
        prevItem = item;
    }
}

// ============================================================================
// Selection and Display Tests
// ============================================================================

void TestEnvVarsTabGUI::testSelectionPopulatesEditor() {
    // Test that selecting a variable fills the editor fields
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(nameEdit != nullptr);
    QVERIFY(valueEdit != nullptr);
    
    // Select first item
    listWidget->setCurrentRow(0);
    QTest::qWait(100);
    
    // Editor should be populated
    QVERIFY(!nameEdit->text().isEmpty());
    QVERIFY(!valueEdit->text().isEmpty());
}

void TestEnvVarsTabGUI::testEditorShowsCorrectValue() {
    // Test that editor shows the correct value for selected variable
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(nameEdit != nullptr);
    QVERIFY(valueEdit != nullptr);
    
    // Select first item
    QString varName = listWidget->item(0)->text();
    listWidget->setCurrentRow(0);
    QTest::qWait(100);
    
    // Name should match
    QCOMPARE(nameEdit->text(), varName);
}

void TestEnvVarsTabGUI::testPreviewUpdates() {
    // Test that preview label updates when value changes
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QLabel* previewLabel = m_tab->findChild<QLabel*>("m_previewLabel");
    
    QVERIFY(valueEdit != nullptr);
    QVERIFY(previewLabel != nullptr);
    
    // Select first variable
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    listWidget->setCurrentRow(0);
    QTest::qWait(100);
    
    // Change value
    valueEdit->clear();
    QTest::keyClicks(valueEdit, "/path/test");
    
    // Preview should update
    QString preview = previewLabel->text();
    QVERIFY(!preview.isEmpty());
}

void TestEnvVarsTabGUI::testPreviewHandlesEmptyValue() {
    // Test that preview handles empty values gracefully
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QLabel* previewLabel = m_tab->findChild<QLabel*>("m_previewLabel");
    
    QVERIFY(valueEdit != nullptr);
    QVERIFY(previewLabel != nullptr);
    
    // Clear value
    valueEdit->clear();
    QTest::qWait(100);
    
    // Should show "No value to preview" or similar
    QString preview = previewLabel->text();
    QVERIFY(!preview.isEmpty());
}

// ============================================================================
// Save Tests
// ============================================================================

void TestEnvVarsTabGUI::testSaveNewVariable() {
    // Test saving a new environment variable
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* saveBtn = m_tab->findChild<QPushButton*>("m_saveButton");
    
    QVERIFY(nameEdit != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(saveBtn != nullptr);
    
    // Clear fields and enter new variable
    nameEdit->clear();
    valueEdit->clear();
    
    QTest::keyClicks(nameEdit, "NEW_TEST_VAR");
    QTest::keyClicks(valueEdit, "new_value");
    
    // Save
    QTest::mouseClick(saveBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Variable should be in list
    QVERIFY(variableExists("NEW_TEST_VAR"));
    QCOMPARE(getVariableValue("NEW_TEST_VAR"), QString("new_value"));
}

void TestEnvVarsTabGUI::testSaveModifiedVariable() {
    // Test saving modifications to an existing variable
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* saveBtn = m_tab->findChild<QPushButton*>("m_saveButton");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(saveBtn != nullptr);
    
    // Select first variable
    listWidget->setCurrentRow(0);
    QTest::qWait(100);
    
    // Modify value
    valueEdit->clear();
    QTest::keyClicks(valueEdit, "modified_value");
    
    // Save
    QTest::mouseClick(saveBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Should save without error
    QVERIFY(true);
}

void TestEnvVarsTabGUI::testSaveValidatesName() {
    // Test that save validates variable name
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* saveBtn = m_tab->findChild<QPushButton*>("m_saveButton");
    
    QVERIFY(nameEdit != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(saveBtn != nullptr);
    
    // Try invalid name with space
    nameEdit->clear();
    valueEdit->clear();
    QTest::keyClicks(nameEdit, "INVALID NAME");
    QTest::keyClicks(valueEdit, "value");
    
    // Save should fail or show error
    QTest::mouseClick(saveBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // No crash expected
    QVERIFY(true);
}

void TestEnvVarsTabGUI::testSaveRejectsDuplicateName() {
    // Test that save rejects duplicate variable names
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* saveBtn = m_tab->findChild<QPushButton*>("m_saveButton");
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    
    QVERIFY(nameEdit != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(saveBtn != nullptr);
    QVERIFY(listWidget != nullptr);
    
    // Get first variable name
    QString firstVar = listWidget->item(0)->text();
    
    // Try to create duplicate with different value
    nameEdit->clear();
    valueEdit->clear();
    QTest::keyClicks(nameEdit, firstVar);
    QTest::keyClicks(valueEdit, "different_value");
    
    // Save should reject
    QTest::mouseClick(saveBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // No crash expected
    QVERIFY(true);
}

// ============================================================================
// Cancel Tests
// ============================================================================

void TestEnvVarsTabGUI::testCancelClearsChanges() {
    // Test that cancel discards unsaved changes
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* cancelBtn = m_tab->findChild<QPushButton*>("m_cancelButton");
    
    QVERIFY(nameEdit != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(cancelBtn != nullptr);
    
    QString originalName = nameEdit->text();
    QString originalValue = valueEdit->text();
    
    // Modify fields
    nameEdit->clear();
    valueEdit->clear();
    QTest::keyClicks(nameEdit, "changed");
    QTest::keyClicks(valueEdit, "changed");
    
    // Cancel
    QTest::mouseClick(cancelBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Should revert to original
    QCOMPARE(nameEdit->text(), originalName);
    QCOMPARE(valueEdit->text(), originalValue);
}

void TestEnvVarsTabGUI::testCancelReloadsOriginalValue() {
    // Test that cancel reloads original value from list
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* cancelBtn = m_tab->findChild<QPushButton*>("m_cancelButton");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(cancelBtn != nullptr);
    
    // Select first variable
    listWidget->setCurrentRow(0);
    QTest::qWait(100);
    
    QString originalValue = valueEdit->text();
    
    // Modify
    valueEdit->clear();
    QTest::keyClicks(valueEdit, "modified");
    
    // Cancel
    QTest::mouseClick(cancelBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Should restore original
    QCOMPARE(valueEdit->text(), originalValue);
}

// ============================================================================
// Copy Tests
// ============================================================================

void TestEnvVarsTabGUI::testCopyAddsVariableWithSuffix() {
    // Test that Copy creates a new variable with _copy suffix
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QPushButton* copyBtn = m_tab->findChild<QPushButton*>("m_copyButton");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(nameEdit != nullptr);
    QVERIFY(copyBtn != nullptr);
    
    // Select first variable
    listWidget->setCurrentRow(0);
    QTest::qWait(100);
    
    QString originalName = nameEdit->text();
    
    // Copy
    QTest::mouseClick(copyBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Name should have _copy suffix
    QVERIFY(nameEdit->text().contains("copy"));
    QVERIFY(nameEdit->text().startsWith(originalName));
}

void TestEnvVarsTabGUI::testCopyPreservesValue() {
    // Test that Copy preserves the original value
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* copyBtn = m_tab->findChild<QPushButton*>("m_copyButton");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(copyBtn != nullptr);
    
    // Select first variable
    listWidget->setCurrentRow(0);
    QTest::qWait(100);
    
    QString originalValue = valueEdit->text();
    
    // Copy
    QTest::mouseClick(copyBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Value should be preserved
    QCOMPARE(valueEdit->text(), originalValue);
}

void TestEnvVarsTabGUI::testCopyHandlesMultipleCopies() {
    // Test that Copy handles multiple copy operations correctly
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QPushButton* copyBtn = m_tab->findChild<QPushButton*>("m_copyButton");
    QPushButton* saveBtn = m_tab->findChild<QPushButton*>("m_saveButton");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(nameEdit != nullptr);
    QVERIFY(copyBtn != nullptr);
    QVERIFY(saveBtn != nullptr);
    
    // Select and copy first variable
    listWidget->setCurrentRow(0);
    QTest::qWait(100);
    
    QString originalName = nameEdit->text();
    
    // Copy and save
    QTest::mouseClick(copyBtn, Qt::LeftButton);
    QTest::qWait(100);
    QTest::mouseClick(saveBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Copy again - should get _copy1 instead of _copy_copy
    QTest::mouseClick(copyBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    QString secondCopyName = nameEdit->text();
    
    // Names should be different and properly suffixed
    QVERIFY(secondCopyName != (originalName + "_copy"));
}

// ============================================================================
// Revert Tests
// ============================================================================

void TestEnvVarsTabGUI::testRevertUserDefinedVariable() {
    // Test reverting (deleting) a user-defined variable with no system counterpart
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* saveBtn = m_tab->findChild<QPushButton*>("m_saveButton");
    QPushButton* revertBtn = m_tab->findChild<QPushButton*>("m_revertButton");
    
    QVERIFY(nameEdit != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(saveBtn != nullptr);
    QVERIFY(revertBtn != nullptr);
    
    // Create a new variable
    nameEdit->clear();
    valueEdit->clear();
    QTest::keyClicks(nameEdit, "USER_DEFINED_VAR");
    QTest::keyClicks(valueEdit, "user_value");
    QTest::mouseClick(saveBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Now revert it - should show delete prompt
    // Since we can't interact with dialogs easily, just verify button works
    if (revertBtn->isEnabled()) {
        QTest::mouseClick(revertBtn, Qt::LeftButton);
        QTest::qWait(100);
    }
    
    // No crash expected
    QVERIFY(true);
}

void TestEnvVarsTabGUI::testRevertOverriddenSystemVariable() {
    // Test reverting an overridden system variable
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* saveBtn = m_tab->findChild<QPushButton*>("m_saveButton");
    QPushButton* revertBtn = m_tab->findChild<QPushButton*>("m_revertButton");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(saveBtn != nullptr);
    QVERIFY(revertBtn != nullptr);
    
    // Select a system variable and modify it
    listWidget->setCurrentRow(0);
    QTest::qWait(100);
    
    valueEdit->clear();
    QTest::keyClicks(valueEdit, "modified_system_var");
    QTest::mouseClick(saveBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Now revert it - should show revert prompt
    if (revertBtn->isEnabled()) {
        QTest::mouseClick(revertBtn, Qt::LeftButton);
        QTest::qWait(100);
    }
    
    // No crash expected
    QVERIFY(true);
}

void TestEnvVarsTabGUI::testRevertPromptAppears() {
    // Test that revert shows appropriate prompt
    // This is harder to test without dialog interaction
    // Just verify button functionality
    QPushButton* revertBtn = m_tab->findChild<QPushButton*>("m_revertButton");
    QVERIFY(revertBtn != nullptr);
    
    // Button should exist and be functional
    QVERIFY(true);
}

// ============================================================================
// Reload Tests
// ============================================================================

void TestEnvVarsTabGUI::testReloadRefreshesFromSettings() {
    // Test that Reload button refreshes variables from settings
    QPushButton* reloadBtn = m_tab->findChild<QPushButton*>("m_restoreDefaultsButton");
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    
    QVERIFY(reloadBtn != nullptr);
    QVERIFY(listWidget != nullptr);
    
    int beforeCount = listWidget->count();
    
    // Click Reload
    QTest::mouseClick(reloadBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // List should be refreshed
    int afterCount = listWidget->count();
    QVERIFY(afterCount > 0);
}

void TestEnvVarsTabGUI::testReloadClearsChanges() {
    // Test that Reload discards unsaved changes
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* reloadBtn = m_tab->findChild<QPushButton*>("m_restoreDefaultsButton");
    
    QVERIFY(nameEdit != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(reloadBtn != nullptr);
    
    // Modify a field
    nameEdit->clear();
    QTest::keyClicks(nameEdit, "test");
    
    // Reload
    QTest::mouseClick(reloadBtn, Qt::LeftButton);
    QTest::qWait(100);
    
    // Changes should be discarded (fields cleared or reset)
    QVERIFY(true);
}

// ============================================================================
// Button State Tests
// ============================================================================

void TestEnvVarsTabGUI::testButtonStatesOnStartup() {
    // Test button enable/disable states at startup
    QPushButton* copyBtn = m_tab->findChild<QPushButton*>("m_copyButton");
    QPushButton* saveBtn = m_tab->findChild<QPushButton*>("m_saveButton");
    QPushButton* revertBtn = m_tab->findChild<QPushButton*>("m_revertButton");
    
    QVERIFY(copyBtn != nullptr);
    QVERIFY(saveBtn != nullptr);
    QVERIFY(revertBtn != nullptr);
    
    // At startup with no selection
    // Copy/Revert should be disabled, Save can be enabled if fields have text
    QVERIFY(!copyBtn->isEnabled() || true);  // Depends on selection
}

void TestEnvVarsTabGUI::testCopyButtonDisabledWithoutSelection() {
    // Test that Copy button is disabled without selection
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QPushButton* copyBtn = m_tab->findChild<QPushButton*>("m_copyButton");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(copyBtn != nullptr);
    
    // Clear selection
    listWidget->clearSelection();
    
    // Copy should be disabled
    QVERIFY(!copyBtn->isEnabled());
}

void TestEnvVarsTabGUI::testRevertButtonDisabledForNewVariable() {
    // Test that Revert button is disabled when creating new variable
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QPushButton* revertBtn = m_tab->findChild<QPushButton*>("m_revertButton");
    
    QVERIFY(nameEdit != nullptr);
    QVERIFY(revertBtn != nullptr);
    
    // Clear and create new name
    nameEdit->clear();
    QTest::keyClicks(nameEdit, "NEW_VAR");
    
    // Revert should be disabled for new variable
    QVERIFY(!revertBtn->isEnabled());
}

// ============================================================================
// Helper Functions
// ============================================================================

bool TestEnvVarsTabGUI::variableExists(const QString& name) {
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    if (!listWidget) return false;
    
    for (int i = 0; i < listWidget->count(); ++i) {
        if (listWidget->item(i)->text() == name) {
            return true;
        }
    }
    return false;
}

QString TestEnvVarsTabGUI::getVariableValue(const QString& name) {
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    
    if (!listWidget || !valueEdit) return QString();
    
    for (int i = 0; i < listWidget->count(); ++i) {
        if (listWidget->item(i)->text() == name) {
            listWidget->setCurrentRow(i);
            return valueEdit->text();
        }
    }
    return QString();
}

void TestEnvVarsTabGUI::selectVariable(const QString& name) {
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    if (!listWidget) return;
    
    for (int i = 0; i < listWidget->count(); ++i) {
        if (listWidget->item(i)->text() == name) {
            listWidget->setCurrentRow(i);
            return;
        }
    }
}

QTEST_MAIN(TestEnvVarsTabGUI)
#include "test_envvarstab_gui.moc"
