/**
 * @file tst_resourcestore.cpp
 * @brief Qt Test unit tests for ResourceStore
 * 
 * Tests the typed resource storage container using Qt Test framework.
 */

#include <QtTest/QtTest>
#include <QSignalSpy>

#include "resInventory/resourceStore.h"
#include "resInventory/resourceScannerDirListing.h"

using namespace resInventory;

/**
 * @brief Test class for ResourceStore
 */
class TestResourceStore : public QObject
{
    Q_OBJECT

private:
    QString m_testDataPath;
    QString m_installPath;
    
    /// Helper to create a test resource
    DiscoveredResource makeResource(const QString& name, 
                                     ResourceType type,
                                     ResourceTier tier,
                                     const QString& location = "test-loc",
                                     const QString& category = QString())
    {
        DiscoveredResource res;
        res.path = QString("/test/path/%1").arg(name);
        res.name = name;
        res.type = type;
        res.tier = tier;
        res.locationKey = location;
        res.category = category;
        res.size = 1024;
        res.lastModified = QDateTime::currentDateTime();
        return res;
    }
    
private slots:
    void initTestCase()
    {
        // Find testFileStructure
        QDir dir(QCoreApplication::applicationDirPath());
        QStringList searchPaths = {
            dir.absoluteFilePath("../../testFileStructure"),
            dir.absoluteFilePath("../../../testFileStructure"),
            QDir::currentPath() + "/testFileStructure",
            QString(SRCDIR) + "/testFileStructure"
        };
        
        for (const QString& path : searchPaths) {
            QDir testDir(path);
            if (testDir.exists() && testDir.exists("inst/OpenSCAD")) {
                m_testDataPath = testDir.absolutePath();
                m_installPath = testDir.absoluteFilePath("inst/OpenSCAD");
                break;
            }
        }
        
        QVERIFY2(!m_testDataPath.isEmpty(), "Could not find testFileStructure");
    }
    
    // ========================================================================
    // Basic operations
    // ========================================================================
    
    void testEmptyStore()
    {
        ResourceStore store;
        
        QVERIFY(store.isEmpty());
        QCOMPARE(store.totalCount(), 0);
        QVERIFY(store.availableTypes().isEmpty());
    }
    
    void testAddSingleResource()
    {
        ResourceStore store;
        QSignalSpy addedSpy(&store, &ResourceStore::resourceAdded);
        
        auto res = makeResource("test.json", ResourceType::Template, ResourceTier::User);
        store.addResource(res);
        
        QVERIFY(!store.isEmpty());
        QCOMPARE(store.totalCount(), 1);
        QCOMPARE(store.countByType(ResourceType::Template), 1);
        QVERIFY(store.hasType(ResourceType::Template));
        QVERIFY(!store.hasType(ResourceType::Font));
        
        // Signal emitted
        QCOMPARE(addedSpy.count(), 1);
    }
    
    void testAddMultipleResources()
    {
        ResourceStore store;
        
        QVector<DiscoveredResource> resources = {
            makeResource("a.json", ResourceType::Template, ResourceTier::User),
            makeResource("b.json", ResourceType::Template, ResourceTier::Installation),
            makeResource("c.ttf", ResourceType::Font, ResourceTier::User)
        };
        
        store.addResources(resources);
        
        QCOMPARE(store.totalCount(), 3);
        QCOMPARE(store.countByType(ResourceType::Template), 2);
        QCOMPARE(store.countByType(ResourceType::Font), 1);
        
        auto types = store.availableTypes();
        QCOMPARE(types.size(), 2);
        QVERIFY(types.contains(ResourceType::Template));
        QVERIFY(types.contains(ResourceType::Font));
    }
    
    // ========================================================================
    // Querying
    // ========================================================================
    
    void testResourcesOfType()
    {
        ResourceStore store;
        
        store.addResource(makeResource("a.json", ResourceType::Template, ResourceTier::User));
        store.addResource(makeResource("b.json", ResourceType::Template, ResourceTier::Installation));
        store.addResource(makeResource("c.ttf", ResourceType::Font, ResourceTier::User));
        
        auto templates = store.resourcesOfType(ResourceType::Template);
        QCOMPARE(templates.size(), 2);
        
        auto fonts = store.resourcesOfType(ResourceType::Font);
        QCOMPARE(fonts.size(), 1);
        
        auto colors = store.resourcesOfType(ResourceType::RenderColors);
        QVERIFY(colors.isEmpty());
    }
    
    void testResourcesOfTypeAndTier()
    {
        ResourceStore store;
        
        store.addResource(makeResource("user.json", ResourceType::Template, ResourceTier::User));
        store.addResource(makeResource("inst.json", ResourceType::Template, ResourceTier::Installation));
        store.addResource(makeResource("mach.json", ResourceType::Template, ResourceTier::Machine));
        
        auto userTemplates = store.resourcesOfType(ResourceType::Template, ResourceTier::User);
        QCOMPARE(userTemplates.size(), 1);
        QCOMPARE(userTemplates[0].name, QString("user.json"));
        
        auto instTemplates = store.resourcesOfType(ResourceType::Template, ResourceTier::Installation);
        QCOMPARE(instTemplates.size(), 1);
        QCOMPARE(instTemplates[0].name, QString("inst.json"));
    }
    
    void testResourcesByLocation()
    {
        ResourceStore store;
        
        store.addResource(makeResource("a.json", ResourceType::Template, ResourceTier::User, "loc-A"));
        store.addResource(makeResource("b.json", ResourceType::Template, ResourceTier::User, "loc-B"));
        store.addResource(makeResource("c.json", ResourceType::Template, ResourceTier::User, "loc-A"));
        
        auto locA = store.resourcesByLocation(ResourceType::Template, "loc-A");
        QCOMPARE(locA.size(), 2);
        
        auto locB = store.resourcesByLocation(ResourceType::Template, "loc-B");
        QCOMPARE(locB.size(), 1);
        
        auto locC = store.resourcesByLocation(ResourceType::Template, "loc-C");
        QVERIFY(locC.isEmpty());
    }
    
    void testResourcesByCategory()
    {
        ResourceStore store;
        
        store.addResource(makeResource("a.scad", ResourceType::Example, ResourceTier::Installation, "loc", "Basics"));
        store.addResource(makeResource("b.scad", ResourceType::Example, ResourceTier::Installation, "loc", "Advanced"));
        store.addResource(makeResource("c.scad", ResourceType::Example, ResourceTier::Installation, "loc", "Basics"));
        
        auto basics = store.resourcesByCategory(ResourceType::Example, "Basics");
        QCOMPARE(basics.size(), 2);
        
        auto advanced = store.resourcesByCategory(ResourceType::Example, "Advanced");
        QCOMPARE(advanced.size(), 1);
    }
    
    void testFindByPath()
    {
        ResourceStore store;
        
        auto res = makeResource("findme.json", ResourceType::Template, ResourceTier::User);
        store.addResource(res);
        store.addResource(makeResource("other.json", ResourceType::Template, ResourceTier::User));
        
        const auto* found = store.findByPath(res.path);
        QVERIFY(found != nullptr);
        QCOMPARE(found->name, QString("findme.json"));
        
        const auto* notFound = store.findByPath("/nonexistent/path");
        QVERIFY(notFound == nullptr);
    }
    
    void testAllResources()
    {
        ResourceStore store;
        
        store.addResource(makeResource("a.json", ResourceType::Template, ResourceTier::User));
        store.addResource(makeResource("b.ttf", ResourceType::Font, ResourceTier::User));
        store.addResource(makeResource("c.json", ResourceType::RenderColors, ResourceTier::Installation));
        
        auto all = store.allResources();
        QCOMPARE(all.size(), 3);
    }
    
    // ========================================================================
    // Counts and metadata
    // ========================================================================
    
    void testCountByTypeAndTier()
    {
        ResourceStore store;
        
        store.addResource(makeResource("a.json", ResourceType::Template, ResourceTier::User));
        store.addResource(makeResource("b.json", ResourceType::Template, ResourceTier::User));
        store.addResource(makeResource("c.json", ResourceType::Template, ResourceTier::Installation));
        
        QCOMPARE(store.countByTypeAndTier(ResourceType::Template, ResourceTier::User), 2);
        QCOMPARE(store.countByTypeAndTier(ResourceType::Template, ResourceTier::Installation), 1);
        QCOMPARE(store.countByTypeAndTier(ResourceType::Template, ResourceTier::Machine), 0);
    }
    
    void testCategoriesForType()
    {
        ResourceStore store;
        
        store.addResource(makeResource("a.scad", ResourceType::Example, ResourceTier::Installation, "loc", "Basics"));
        store.addResource(makeResource("b.scad", ResourceType::Example, ResourceTier::Installation, "loc", "Advanced"));
        store.addResource(makeResource("c.scad", ResourceType::Example, ResourceTier::Installation, "loc", "Basics"));
        store.addResource(makeResource("d.scad", ResourceType::Example, ResourceTier::Installation, "loc", ""));  // No category
        
        auto categories = store.categoriesForType(ResourceType::Example);
        QCOMPARE(categories.size(), 2);
        QVERIFY(categories.contains("Basics"));
        QVERIFY(categories.contains("Advanced"));
    }
    
    void testLocationsForType()
    {
        ResourceStore store;
        
        store.addResource(makeResource("a.json", ResourceType::Template, ResourceTier::User, "user-docs"));
        store.addResource(makeResource("b.json", ResourceType::Template, ResourceTier::Installation, "install"));
        store.addResource(makeResource("c.json", ResourceType::Template, ResourceTier::User, "user-docs"));
        
        auto locations = store.locationsForType(ResourceType::Template);
        QCOMPARE(locations.size(), 2);
        QVERIFY(locations.contains("user-docs"));
        QVERIFY(locations.contains("install"));
    }
    
    // ========================================================================
    // Modification
    // ========================================================================
    
    void testClear()
    {
        ResourceStore store;
        QSignalSpy clearedSpy(&store, &ResourceStore::resourcesCleared);
        
        store.addResource(makeResource("a.json", ResourceType::Template, ResourceTier::User));
        store.addResource(makeResource("b.ttf", ResourceType::Font, ResourceTier::User));
        
        QCOMPARE(store.totalCount(), 2);
        
        store.clear();
        
        QVERIFY(store.isEmpty());
        QCOMPARE(store.totalCount(), 0);
        QCOMPARE(clearedSpy.count(), 1);
        
        // Signal parameter is Unknown for full clear
        QCOMPARE(clearedSpy.takeFirst().at(0).value<ResourceType>(), ResourceType::Unknown);
    }
    
    void testClearType()
    {
        ResourceStore store;
        QSignalSpy clearedSpy(&store, &ResourceStore::resourcesCleared);
        
        store.addResource(makeResource("a.json", ResourceType::Template, ResourceTier::User));
        store.addResource(makeResource("b.json", ResourceType::Template, ResourceTier::User));
        store.addResource(makeResource("c.ttf", ResourceType::Font, ResourceTier::User));
        
        store.clearType(ResourceType::Template);
        
        QCOMPARE(store.countByType(ResourceType::Template), 0);
        QCOMPARE(store.countByType(ResourceType::Font), 1);
        QCOMPARE(store.totalCount(), 1);
        
        QCOMPARE(clearedSpy.count(), 1);
        QCOMPARE(clearedSpy.takeFirst().at(0).value<ResourceType>(), ResourceType::Template);
    }
    
    void testClearTier()
    {
        ResourceStore store;
        
        store.addResource(makeResource("user.json", ResourceType::Template, ResourceTier::User));
        store.addResource(makeResource("inst.json", ResourceType::Template, ResourceTier::Installation));
        store.addResource(makeResource("user.ttf", ResourceType::Font, ResourceTier::User));
        
        store.clearTier(ResourceTier::User);
        
        QCOMPARE(store.totalCount(), 1);
        QCOMPARE(store.countByTypeAndTier(ResourceType::Template, ResourceTier::User), 0);
        QCOMPARE(store.countByTypeAndTier(ResourceType::Template, ResourceTier::Installation), 1);
    }
    
    void testRemoveByPath()
    {
        ResourceStore store;
        QSignalSpy removedSpy(&store, &ResourceStore::resourceRemoved);
        
        auto res = makeResource("removeme.json", ResourceType::Template, ResourceTier::User);
        store.addResource(res);
        store.addResource(makeResource("keepme.json", ResourceType::Template, ResourceTier::User));
        
        QCOMPARE(store.totalCount(), 2);
        
        bool removed = store.removeByPath(res.path);
        QVERIFY(removed);
        QCOMPARE(store.totalCount(), 1);
        QVERIFY(store.findByPath(res.path) == nullptr);
        
        QCOMPARE(removedSpy.count(), 1);
        QCOMPARE(removedSpy.takeFirst().at(0).toString(), res.path);
        
        // Try to remove non-existent
        bool notRemoved = store.removeByPath("/nonexistent");
        QVERIFY(!notRemoved);
    }
    
    // ========================================================================
    // Integration with scanner
    // ========================================================================
    
    void testScanAndStore()
    {
        ResourceStore store;
        ResourceScannerDirListing scanner;
        
        int count = store.scanAndStore(scanner, m_installPath, 
                                        ResourceTier::Installation, "test-install");
        
        QVERIFY2(count > 0, "Expected to scan some resources");
        QCOMPARE(store.totalCount(), count);
        
        // Should have found multiple types
        auto types = store.availableTypes();
        QVERIFY2(types.size() >= 2, "Expected at least 2 resource types");
    }
    
    void testScanTypeAndStore()
    {
        ResourceStore store;
        ResourceScannerDirListing scanner;
        
        int count = store.scanTypeAndStore(scanner, m_installPath,
                                            ResourceType::RenderColors,
                                            ResourceTier::Installation, "test-install");
        
        QVERIFY2(count > 0, "Expected to find render colors");
        QCOMPARE(store.countByType(ResourceType::RenderColors), count);
        
        // Only RenderColors should be present
        QCOMPARE(store.availableTypes().size(), 1);
        QVERIFY(store.hasType(ResourceType::RenderColors));
    }
    
    void testMultipleScansMerge()
    {
        ResourceStore store;
        ResourceScannerDirListing scanner;
        
        // Scan render colors
        store.scanTypeAndStore(scanner, m_installPath,
                               ResourceType::RenderColors,
                               ResourceTier::Installation, "install");
        
        int afterFirst = store.totalCount();
        
        // Scan templates - should add to existing
        store.scanTypeAndStore(scanner, m_installPath,
                               ResourceType::Template,
                               ResourceTier::Installation, "install");
        
        int afterSecond = store.totalCount();
        
        QVERIFY(afterSecond > afterFirst);
        QVERIFY(store.hasType(ResourceType::RenderColors));
        QVERIFY(store.hasType(ResourceType::Template));
    }
};

// Register metatype for signal spy
Q_DECLARE_METATYPE(ResourceType)

QTEST_MAIN(TestResourceStore)
#include "test_resourcestore.moc"
