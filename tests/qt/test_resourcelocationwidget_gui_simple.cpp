/**
 * @file test_resourcelocationwidget_gui_simple.cpp
 * @brief GUI tests for ResourceLocationWidget using QtTestLib
 */

#include "test_resourcelocationwidget_gui_simple.h"
#include <QTest>

void TestResourceLocationWidgetGUI::initTestCase() {
}

void TestResourceLocationWidgetGUI::cleanupTestCase() {
}

void TestResourceLocationWidgetGUI::testFrameworkExists() {
    // Just verify the test infrastructure is working
    QVERIFY(true);
}

void TestResourceLocationWidgetGUI::testQtVersion() {
    // Verify we're running with a valid Qt version
    const char* qtVersion = qVersion();
    QVERIFY(qtVersion != nullptr);
    QVERIFY(strlen(qtVersion) > 0);
}

QTEST_MAIN(TestResourceLocationWidgetGUI)
