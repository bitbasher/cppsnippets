/**
 * @file test_envvarstab_gui_simple.h
 * @brief GUI tests for EnvVarsTab using QtTestLib
 * 
 * Simplified test suite focusing on core functionality
 */

#pragma once

#include <QObject>
#include <QTest>

class EnvVarsTab;

class TestEnvVarsTabGUI : public QObject {
    Q_OBJECT

private slots:
    // Setup/teardown
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // List tests
    void testListPopulatesFromSystemEnv();
    void testListPopulatesFromOverrides();
    
    // Selection tests
    void testSelectionPopulatesEditor();
    
    // Save tests
    void testSaveNewVariable();
    
    // Cancel tests
    void testCancelClearsChanges();
    
    // Revert tests
    void testRevertUserDefinedVariable();
    
    // Reload tests
    void testReloadRefreshesFromSettings();

private:
    EnvVarsTab* m_tab = nullptr;
    bool variableExists(const QString& name);
    QString getVariableValue(const QString& name);
    void selectVariable(const QString& name);
};
