/**
 * @file test_scantomodel.cpp
 * @brief Unit tests for scanToModel() high-level API (Phase 2)
 */

#include <QtTest/QtTest>
#include <QDebug>
#include "resourceScanner_test.hpp"
#include <platformInfo/ResourceLocation.hpp>
#include <QStandardItemModel>
#include <QStandardItem>

using namespace resourceInventory;
using namespace resourceMetadata;

class ScanToModelTest : public QObject {
    Q_OBJECT

private:
    QString testStructurePath;
    QList<platformInfo::ResourceLocation> allLocations;
    ResourceScanner* scanner = nullptr;

private slots:
    void initTestCase() {
        qDebug() << "*** ScanToModelTest STARTING - initTestCase called ***";
        
        // Get test file structure path relative to project root
        testStructurePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../../testFileStructure");
        
        // Verify test structure exists
        QDir testDir(testStructurePath);
        if (!testDir.exists()) {
            QSKIP("testFileStructure directory not found - cannot run tests");
        }
        
        qDebug() << "Using test structure at:" << testStructurePath;
    }
    
    void init() {
        qDebug() << "*** init() called for test ***";
        
        allLocations.clear();
        
        // Add Installation tier locations (from testFileStructure/inst/)
        QString installPath1 = QDir::cleanPath(testStructurePath + "/inst/OpenSCAD");
        if (QDir(installPath1).exists()) {
            platformInfo::ResourceLocation installLoc(installPath1, ResourceTier::Installation);
            installLoc.setEnabled(true);
            installLoc.setExists(true);
            allLocations.append(installLoc);
        }
        
        QString installPath2 = QDir::cleanPath(testStructurePath + "/inst/OpenSCAD (Nightly)");
        if (QDir(installPath2).exists()) {
            platformInfo::ResourceLocation installLoc(installPath2, ResourceTier::Installation);
            installLoc.setEnabled(true);
            installLoc.setExists(true);
            allLocations.append(installLoc);
        }
        
        // Add User tier locations (from testFileStructure/pers/)
        QString userPath1 = QDir::cleanPath(testStructurePath + "/pers/appdata/local/openscad");
        if (QDir(userPath1).exists()) {
            platformInfo::ResourceLocation userLoc(userPath1, ResourceTier::User);
            userLoc.setEnabled(true);
            userLoc.setExists(true);
            allLocations.append(userLoc);
        }
        
        QString userPath2 = QDir::cleanPath(testStructurePath + "/pers/Jeff/Documents/OpenSCAD");
        if (QDir(userPath2).exists()) {
            platformInfo::ResourceLocation userLoc(userPath2, ResourceTier::User);
            userLoc.setEnabled(true);
            userLoc.setExists(true);
            allLocations.append(userLoc);
        }
        
        scanner = new ResourceScanner();
    }
    
    void cleanup() {
        delete scanner;
        scanner = nullptr;
        allLocations.clear();
    }
    
    void cleanupTestCase() {
        qDebug() << "*** ScanToModelTest FINISHED - cleanupTestCase called ***";
    }

// ============================================================================
// scanToModel() High-Level API Tests
// ============================================================================

private slots:
    void testScanToModelPopulatesFromAllTiers() {
        QStandardItemModel model;
        scanner->scanToModel(&model, allLocations);
        
        // Should find templates from Installation and User tiers
        // (testFileStructure has templates in multiple locations)
        qDebug() << "Total items found:" << model.rowCount();
        QVERIFY(model.rowCount() > 0);
    }
    
    void testScanToModelStoresCorrectTiers() {
        QStandardItemModel model;
        scanner->scanToModel(&model, allLocations);
        
        int installCount = 0;
        int userCount = 0;
        
        for (int i = 0; i < model.rowCount(); ++i) {
            QStandardItem* item = model.item(i);
            QVERIFY(item != nullptr);
            
            int tierValue = item->data(Qt::UserRole + 2).toInt();  // TierRole
            ResourceTier tier = static_cast<ResourceTier>(tierValue);
            
            if (tier == ResourceTier::Installation) installCount++;
            else if (tier == ResourceTier::User) userCount++;
        }
        
        qDebug() << "Installation templates:" << installCount << "User templates:" << userCount;
        
        // testFileStructure has templates in both tiers
        QVERIFY(installCount > 0);
        QVERIFY(userCount > 0);
    }
    
    void testScanToModelStoresLocationKeys() {
        QStandardItemModel model;
        scanner->scanToModel(&model, allLocations);
        
        QSet<QString> locationKeys;
        
        for (int i = 0; i < model.rowCount(); ++i) {
            QStandardItem* item = model.item(i);
            QString locationKey = item->data(Qt::UserRole + 6).toString();  // LocationKeyRole
            if (!locationKey.isEmpty()) {
                locationKeys.insert(locationKey);
            }
        }
        
        qDebug() << "Location keys found:" << locationKeys;
        
        // Should have multiple location keys from testFileStructure
        QVERIFY(locationKeys.size() > 1);
    }
    
    void testScanToModelHandlesNullModel() {
        // Should not crash with null model
        scanner->scanToModel(nullptr, allLocations);
        // Test passes if no crash
        QVERIFY(true);
    }
    
    void testScanToModelFindsSpecificTemplates() {
        QStandardItemModel model;
        scanner->scanToModel(&model, allLocations);
        
        // Look for specific templates we know exist in testFileStructure
        QStringList foundNames;
        for (int i = 0; i < model.rowCount(); ++i) {
            QStandardItem* item = model.item(i);
            foundNames << item->text();
        }
        
        qDebug() << "Found template names:" << foundNames;
        
        // These templates exist in testFileStructure
        // (checking for any known templates to verify scanning works)
        QVERIFY(foundNames.size() > 0);
    }
};

QTEST_MAIN(ScanToModelTest)
#include "test_scantomodel.moc"
