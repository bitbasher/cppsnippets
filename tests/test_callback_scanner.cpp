/**
 * @file test_callback_scanner.cpp
 * @brief Unit tests for callback-based ResourceScanner API (Phase 1)
 */

#include <QtTest/QtTest>
#include <QDebug>
#include "resourceScanner_test.hpp"
#include <resourceInventory/resourceItem.hpp>
#include <QTemporaryDir>
#include <QFile>
#include <QStandardItemModel>
#include <QStandardItem>

using namespace resourceInventory;
using namespace resourceMetadata;

class CallbackScannerTest : public QObject {
    Q_OBJECT

private:
    QTemporaryDir* tempDir = nullptr;
    QString basePath;
    ResourceScanner* scanner = nullptr;
    
    void createTestFile(const QString& relativePath, const QString& content) {
        QString fullPath = basePath + "/" + relativePath;
        QFile file(fullPath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write(content.toUtf8());
        file.close();
    }

private slots:
    void initTestCase() {
        qDebug() << "*** TEST STARTING - initTestCase called ***";
        // Called once before all tests
    }
    
    void init() {
        qDebug() << "*** init() called for test ***";
        // Called before each test
        tempDir = new QTemporaryDir();
        QVERIFY(tempDir->isValid());
        
        basePath = tempDir->path();
        
        // Create test template files
        createTestFile("template1.scad", "// Template 1");
        createTestFile("template2.json", R"({"body": "test"})");
        
        // Create subfolder with categorized templates
        QDir(basePath).mkdir("Category1");
        createTestFile("Category1/template3.scad", "// Template 3");
        createTestFile("Category1/template4.json", R"({"body": "test4"})");
        
        // Create nested subfolder
        QDir(basePath).mkdir("Category1/Nested");
        createTestFile("Category1/Nested/template5.scad", "// Template 5");
        
        scanner = new ResourceScanner();
    }
    
    void cleanup() {
        // Called after each test
        delete scanner;
        scanner = nullptr;
        delete tempDir;
        tempDir = nullptr;
    }
    
    void cleanupTestCase() {
        // Called once after all tests
    }

// ============================================================================
// Callback API Tests
// ============================================================================

private slots:
    void testCallbackReceivesAllItems() {
    QList<ResourceItem> captured;
    
    scanner->scanTemplates(basePath, ResourceTier::User, "Test Location",
                          [&captured](const ResourceItem& item) {
        captured.append(item);
    });
    
    QCOMPARE(captured.size(), 5);
}

void testCallbackReceivesCorrectType() {
    bool allTemplates = true;
    
    scanner->scanTemplates(basePath, ResourceTier::User, "Test Location",
                          [&allTemplates](const ResourceItem& item) {
        if (item.type() != ResourceType::Templates) {
            allTemplates = false;
        }
    });
    
    QVERIFY(allTemplates);
}

void testCallbackReceivesCorrectTier() {
    bool allUserTier = true;
    
    scanner->scanTemplates(basePath, ResourceTier::Machine, "Test Location",
                          [&allUserTier](const ResourceItem& item) {
        if (item.tier() != ResourceTier::Machine) {
            allUserTier = false;
        }
    });
    
    QVERIFY(allUserTier);
}

void testCallbackReceivesCorrectLocationKey() {
    QString expectedKey = "MyTestLocation";
    bool allCorrect = true;
    
    scanner->scanTemplates(basePath, ResourceTier::User, expectedKey,
                          [&allCorrect, &expectedKey](const ResourceItem& item) {
        if (item.sourceLocationKey() != expectedKey) {
            allCorrect = false;
        }
    });
    
    QVERIFY(allCorrect);
}

void testCallbackReceivesCategoryForSubfolderItems() {
    int topLevelCount = 0;
    int category1Count = 0;
    int nestedCount = 0;
    
    scanner->scanTemplates(basePath, ResourceTier::User, "Test Location",
                          [&](const ResourceItem& item) {
        QString category = item.category();
        if (category.isEmpty()) {
            topLevelCount++;
        } else if (category == "Category1") {
            category1Count++;
        } else if (category == "Category1/Nested") {
            nestedCount++;
        }
    });
    
    QCOMPARE(topLevelCount, 2);   // template1.scad, template2.json
    QCOMPARE(category1Count, 2);  // template3.scad, template4.json
    QCOMPARE(nestedCount, 1);     // template5.scad
}

void testCallbackHandlesEmptyDirectory() {
    QTemporaryDir emptyDir;
    QVERIFY(emptyDir.isValid());
    
    int callbackCount = 0;
    scanner->scanTemplates(emptyDir.path(), ResourceTier::User, "Empty",
                          [&callbackCount](const ResourceItem& item) {
        Q_UNUSED(item);
        callbackCount++;
    });
    
    QCOMPARE(callbackCount, 0);
}

void testCallbackHandlesNonExistentDirectory() {
    int callbackCount = 0;
    scanner->scanTemplates("/nonexistent/path", ResourceTier::User, "Missing",
                          [&callbackCount](const ResourceItem& item) {
        Q_UNUSED(item);
        callbackCount++;
    });
    
    QCOMPARE(callbackCount, 0);
}

void testNullCallbackDoesNotCrash() {
    // Should not crash with null callback - QTest doesn't have EXPECT_NO_THROW
    // Just call it and if it crashes, test fails
    scanner->scanTemplates(basePath, ResourceTier::User, "Test", nullptr);
    QVERIFY(true); // If we get here, didn't crash
}

// ============================================================================
// Convenience Wrapper Tests
// ============================================================================

void testScanToListReturnsAllItems() {
    QList<ResourceItem> results = scanner->scanTemplatesToList(
        basePath, ResourceTier::User, "Test Location");
    
    QCOMPARE(results.size(), 5);
}

void testScanToListMatchesCallbackResults() {
    QList<ResourceItem> fromList = scanner->scanTemplatesToList(
        basePath, ResourceTier::User, "Test Location");
    
    QList<ResourceItem> fromCallback;
    scanner->scanTemplates(basePath, ResourceTier::User, "Test Location",
                          [&fromCallback](const ResourceItem& item) {
        fromCallback.append(item);
    });
    
    QCOMPARE(fromList.size(), fromCallback.size());
    
    // Compare names (order should be same)
    for (int i = 0; i < fromList.size(); ++i) {
        QCOMPARE(fromList[i].name(), fromCallback[i].name());
        QCOMPARE(fromList[i].type(), fromCallback[i].type());
        QCOMPARE(fromList[i].tier(), fromCallback[i].tier());
    }
}

void testScanToModelPopulatesModel() {
    QStandardItemModel model;
    
    scanner->scanTemplatesToModel(basePath, ResourceTier::User, "Test Location", &model);
    
    QCOMPARE(model.rowCount(), 5);
}

void testScanToModelStoresMetadata() {
    QStandardItemModel model;
    
    scanner->scanTemplatesToModel(basePath, ResourceTier::Machine, "MyLocation", &model);
    
    QVERIFY(model.rowCount() > 0);
    
    // Custom roles for verification
    enum ItemRole {
        TypeRole = Qt::UserRole + 1,
        TierRole = Qt::UserRole + 2,
        PathRole = Qt::UserRole + 3,
        CategoryRole = Qt::UserRole + 4,
        AccessRole = Qt::UserRole + 5,
        LocationKeyRole = Qt::UserRole + 6
    };
    
    // Check first item has metadata
    QStandardItem* firstItem = model.item(0);
    QVERIFY(firstItem != nullptr);
    
    int type = firstItem->data(TypeRole).toInt();
    QCOMPARE(type, static_cast<int>(ResourceType::Templates));
    
    int tier = firstItem->data(TierRole).toInt();
    QCOMPARE(tier, static_cast<int>(ResourceTier::Machine));
    
    QString locationKey = firstItem->data(LocationKeyRole).toString();
    QCOMPARE(locationKey, QString("MyLocation"));
    
    QString path = firstItem->data(PathRole).toString();
    QVERIFY(!path.isEmpty());
}

void testScanToModelHandlesNullModel() {
    // Should not crash with null model
    scanner->scanTemplatesToModel(basePath, ResourceTier::User, "Test", nullptr);
    QVERIFY(true); // If we get here, didn't crash
}

void testScanToModelHandlesEmptyDirectory() {
    QTemporaryDir emptyDir;
    QVERIFY(emptyDir.isValid());
    
    QStandardItemModel model;
    scanner->scanTemplatesToModel(emptyDir.path(), ResourceTier::User, "Empty", &model);
    
    QCOMPARE(model.rowCount(), 0);
}

// ============================================================================
// Integration Tests
// ============================================================================

void testMultipleScansToSameModel() {
    QStandardItemModel model;
    
    // Scan from first location
    scanner->scanTemplatesToModel(basePath, ResourceTier::User, "Location1", &model);
    int firstCount = model.rowCount();
    
    // Create second location with different templates
    QTemporaryDir tempDir2;
    QVERIFY(tempDir2.isValid());
    QString basePath2 = tempDir2.path();
    
    QFile file(basePath2 + "/another.scad");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("// Another template");
    file.close();
    
    // Scan from second location (should append)
    scanner->scanTemplatesToModel(basePath2, ResourceTier::Machine, "Location2", &model);
    
    QCOMPARE(model.rowCount(), firstCount + 1);
}

void testCallbackCanFilterItems() {
    // Only count .scad files
    int scadCount = 0;
    
    scanner->scanTemplates(basePath, ResourceTier::User, "Test Location",
                          [&scadCount](const ResourceItem& item) {
        if (item.sourcePath().endsWith(".scad")) {
            scadCount++;
        }
    });
    
    QCOMPARE(scadCount, 3);  // template1.scad, template3.scad, template5.scad
}

void testCallbackCanLogEachItem() {
    QStringList log;
    
    scanner->scanTemplates(basePath, ResourceTier::User, "Test Location",
                          [&log](const ResourceItem& item) {
        log << QString("Found: %1 in %2").arg(item.name(), item.category());
    });
    
    QCOMPARE(log.size(), 5);
    QVERIFY(log[0].startsWith("Found: "));
}
};

// ============================================================================
// Main
// ============================================================================

QTEST_MAIN(CallbackScannerTest)
#include "test_callback_scanner.moc"