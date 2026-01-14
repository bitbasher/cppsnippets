/**
 * @file test_scanexamples.cpp
 * @brief Unit tests for scanExamples callback-based API
 */

#include <QtTest/QtTest>
#include "resourceScanner_test.hpp"
#include <QStandardItemModel>
#include <QDir>

using namespace resourceInventory;
using namespace platformInfo;

class ScanExamplesTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Use absolute path to testFileStructure
        m_testDataRoot = "D:/repositories/cppsnippets/cppsnippets/testFileStructure";
        m_scanner = new ResourceScanner();
    }
    
    void cleanupTestCase() {
        delete m_scanner;
    }
    
    void callbackReceivesAllExamples() {
        QString examplesPath = m_testDataRoot + "/pers/Jeff/Documents/OpenSCAD/Examples";
        QList<ResourceItem> captured;
        
        m_scanner->scanExamples(examplesPath, ResourceTier::User, "Test Location",
            [&captured](const ResourceItem& item) {
                captured.append(item);
            });
        
        // Should find: somethingNew.scad (top-level)
        //              + trialrun.scad (Docs_Support/)
        //              + customizer-all.scad (Parametric/)
        // Note: ListComprehensions is nested (2 levels deep) so listprocessor.scad is out of scope
        // Note: May also find templates and tests if those subfolders exist
        QVERIFY(captured.size() >= 3);
        
        // Verify Examples are tagged correctly (may have Templates/Tests mixed in)
        int exampleCount = 0;
        int templateCount = 0;
        int testCount = 0;
        for (const auto& item : captured) {
            if (item.type() == ResourceType::Examples) {
                exampleCount++;
                QCOMPARE(item.tier(), ResourceTier::User);
            }
            else if (item.type() == ResourceType::Templates) {
                templateCount++;
            }
            else if (item.type() == ResourceType::Tests) {
                testCount++;
            }
        }
        
        QVERIFY(exampleCount >= 3);  // Should have at least 3 examples
    }

    void examplesHaveCategories() {
        QString examplesPath = m_testDataRoot + "/pers/Jeff/Documents/OpenSCAD/Examples";
        QList<ResourceItem> captured;
        
        m_scanner->scanExamples(examplesPath, ResourceTier::User, "Test Location",
            [&captured](const ResourceItem& item) {
                captured.append(item);
            });
        
        // Find an example from Docs_Support category
        auto it = std::find_if(captured.begin(), captured.end(), [](const ResourceItem& item) {
            return item.category() == "Docs_Support";
        });
        
        QVERIFY(it != captured.end());
    }

    void scanExamplesToListWorks() {
        QString examplesPath = m_testDataRoot + "/pers/Jeff/Documents/OpenSCAD/Examples";
        
        QList<ResourceItem> examples = m_scanner->scanExamplesToList(
            examplesPath, ResourceTier::User, "Test Location");
        
        QVERIFY(examples.size() >= 3);
    }

    void scanExamplesToModelPopulatesCorrectly() {
        QString examplesPath = m_testDataRoot + "/pers/Jeff/Documents/OpenSCAD/Examples";
        QStandardItemModel model;
        
        m_scanner->scanExamplesToModel(examplesPath, ResourceTier::User, "Test Location", &model);
        
        QVERIFY(model.rowCount() >= 3);
        
        // Verify first item has correct type in custom role
        if (model.rowCount() > 0) {
            QStandardItem* firstItem = model.item(0);
            QVERIFY(firstItem != nullptr);
            
            int typeRole = static_cast<int>(Qt::UserRole) + 1;
            int typeValue = firstItem->data(typeRole).toInt();
            QCOMPARE(typeValue, static_cast<int>(ResourceType::Examples));
        }
    }

    void emptyPathReturnsNoExamples() {
        QList<ResourceItem> captured;
        
        m_scanner->scanExamples("/nonexistent/path", ResourceTier::User, "Test",
            [&captured](const ResourceItem& item) {
                captured.append(item);
            });
        
        QCOMPARE(captured.size(), 0);
    }

private:
    QString m_testDataRoot;
    ResourceScanner* m_scanner;
};

QTEST_MAIN(ScanExamplesTest)
#include "test_scanexamples.moc"
