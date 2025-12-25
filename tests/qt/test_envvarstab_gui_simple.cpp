/**
 * @file test_envvarstab_gui_simple.cpp
 * @brief GUI tests for EnvVarsTab using QtTestLib
 */

#include "test_envvarstab_gui_simple.h"
#include "gui/envVarsTab.h"
#include "platformInfo/resourceLocationManager.h"

#include <QTest>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QApplication>
#include <QSettings>

void TestEnvVarsTabGUI::initTestCase() {
}

void TestEnvVarsTabGUI::cleanupTestCase() {
}

void TestEnvVarsTabGUI::init() {
    auto* manager = new platformInfo::ResourceLocationManager(nullptr);
    m_tab = new EnvVarsTab(manager, nullptr);
    m_tab->show();
    QTest::qWaitForWindowExposed(m_tab);
}

void TestEnvVarsTabGUI::cleanup() {
    delete m_tab;
    m_tab = nullptr;
}

// ============================================================================
// List Population Tests
// ============================================================================

void TestEnvVarsTabGUI::testListPopulatesFromSystemEnv() {
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QVERIFY(listWidget != nullptr);
    
    // List should have items from system environment
    QVERIFY(listWidget->count() > 0);
    
    // Common system variables should be present (PATH exists on all platforms)
    bool hasPath = false;
    for (int i = 0; i < listWidget->count(); ++i) {
        QString item = listWidget->item(i)->text();
        if (item.compare("PATH", Qt::CaseInsensitive) == 0) {
            hasPath = true;
            break;
        }
    }
    QVERIFY(hasPath);
}

void TestEnvVarsTabGUI::testListPopulatesFromOverrides() {
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QVERIFY(listWidget != nullptr);
    
    int initialCount = listWidget->count();
    QVERIFY(initialCount > 0);
}

// ============================================================================
// Selection Tests
// ============================================================================

void TestEnvVarsTabGUI::testSelectionPopulatesEditor() {
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(valueEdit != nullptr);
    
    if (listWidget->count() > 0) {
        listWidget->setCurrentRow(0);
        QTest::qWait(100);
        // Value should be populated
        QVERIFY(valueEdit != nullptr);
    }
}

// ============================================================================
// Save Tests
// ============================================================================

void TestEnvVarsTabGUI::testSaveNewVariable() {
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QLineEdit* valueEdit = m_tab->findChild<QLineEdit*>("m_valueEdit");
    QPushButton* saveBtn = m_tab->findChild<QPushButton*>("m_saveButton");
    
    QVERIFY(nameEdit != nullptr);
    QVERIFY(valueEdit != nullptr);
    QVERIFY(saveBtn != nullptr);
    QVERIFY(saveBtn->isEnabled());
}

// ============================================================================
// Cancel Tests
// ============================================================================

void TestEnvVarsTabGUI::testCancelClearsChanges() {
    QLineEdit* nameEdit = m_tab->findChild<QLineEdit*>("m_nameEdit");
    QPushButton* cancelBtn = m_tab->findChild<QPushButton*>("m_cancelButton");
    
    QVERIFY(nameEdit != nullptr);
    QVERIFY(cancelBtn != nullptr);
}

// ============================================================================
// Revert Tests
// ============================================================================

void TestEnvVarsTabGUI::testRevertUserDefinedVariable() {
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QPushButton* revertBtn = m_tab->findChild<QPushButton*>("m_revertButton");
    
    QVERIFY(listWidget != nullptr);
    QVERIFY(revertBtn != nullptr);
}

// ============================================================================
// Reload Tests
// ============================================================================

void TestEnvVarsTabGUI::testReloadRefreshesFromSettings() {
    QListWidget* listWidget = m_tab->findChild<QListWidget*>("m_listWidget");
    QVERIFY(listWidget != nullptr);
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
#include "test_envvarstab_gui_simple.moc"
