/**
 * @file test_mainwindow_gui.h
 * @brief GUI tests for MainWindow using QtTestLib
 * 
 * Tests cover:
 * - Template search functionality
 * - Button state management
 * - Template selection and editor population
 * - Save destination tracking
 */

#pragma once

#include <QObject>
#include <QTest>

class MainWindow;

class TestMainWindowGUI : public QObject {
    Q_OBJECT

private slots:
    // Setup/teardown
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // Search tests
    void testSearchFindTemplate();
    void testSearchWrapAround();
    void testSearchNoMatch();
    void testSearchCaseSensitivity();
    void testSearchWithSpaces();
    void testSearchClearSelection();
    
    // Button state tests
    void testButtonStateOnStartup();
    void testButtonStateOnUserTemplateSelection();
    void testButtonStateOnMachineTemplateSelection();
    void testButtonStateOnInstallationTemplateSelection();
    void testButtonStateOnEdit();
    void testButtonStateOnCancel();
    
    // New template tests
    void testNewTemplateInitialization();
    void testNewTemplateSaveDestination();
    void testNewTemplateFieldsReadOnly();
    
    // Copy template tests
    void testCopyTemplateName();
    void testCopyTemplateContent();
    void testCopyTemplateSaveDestination();
    
    // Editor tests
    void testEditorPopulationFromSelection();
    void testEditorDescription();
    void testEditorVersion();

private:
    MainWindow* m_window = nullptr;
    
    // Helper functions
    QModelIndex findTemplateByName(const QString& name);
    bool isButtonEnabled(const QString& buttonName);
    void selectTemplateByName(const QString& name);
};
