/**
 * @file test_envvarstab_gui.h
 * @brief GUI tests for EnvVarsTab using QtTestLib
 * 
 * Tests cover:
 * - Environment variable list population
 * - Edit, save, cancel operations
 * - Copy variable functionality
 * - Revert to system values
 * - Delete user-defined variables
 */

#pragma once

#include <QObject>
#include <QTest>

class EnvVarsTab;

class TestEnvVarsTabGUI : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // List population tests
    void testListPopulatesFromSystemEnv();
    void testListPopulatesFromOverrides();
    void testListDoesNotDuplicate();
    void testListIsSorted();
    
    // Selection and display tests
    void testSelectionPopulatesEditor();
    void testEditorShowsCorrectValue();
    void testPreviewUpdates();
    void testPreviewHandlesEmptyValue();
    
    // Save tests
    void testSaveNewVariable();
    void testSaveModifiedVariable();
    void testSavePersistsToSettings();
    void testSaveValidatesName();
    void testSaveRejectsDuplicateName();
    
    // Cancel tests
    void testCancelClearsChanges();
    void testCancelReloadsOriginalValue();
    
    // Copy tests
    void testCopyAddsVariableWithSuffix();
    void testCopyPreservesValue();
    void testCopyHandlesMultipleCopies();
    
    // Revert tests
    void testRevertUserDefinedVariable();
    void testRevertOverriddenSystemVariable();
    void testRevertPromptAppears();
    
    // Reload tests
    void testReloadRefreshesFromSettings();
    void testReloadClearsChanges();
    
    // Button state tests
    void testButtonStatesOnStartup();
    void testCopyButtonDisabledWithoutSelection();
    void testRevertButtonDisabledForNewVariable();

private:
    EnvVarsTab* m_tab = nullptr;
    
    // Helper functions
    bool variableExists(const QString& name);
    QString getVariableValue(const QString& name);
    void selectVariable(const QString& name);
};
