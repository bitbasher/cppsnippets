/**
 * @file test_templatetreemodel.cc
 * @brief Qt Test unit tests for TemplateTreeModel
 * 
 * Uses QAbstractItemModelTester to verify model contract compliance,
 * plus specific tests for template tree structure.
 */

#include <QtTest/QtTest>
#include <QAbstractItemModelTester>

#include "resInventory/templateTreeModel.h"
#include "resInventory/resourceStore.h"
#include "resInventory/resourceScannerDirListing.h"

using namespace resInventory;

/**
 * @brief Test class for TemplateTreeModel
 */
class TestTemplateTreeModel : public QObject
{
    Q_OBJECT

private:
    QString m_testDataPath;
    QString m_installPath;
    
    /// Helper to create a test resource
    DiscoveredResource makeTemplate(const QString& name, 
                                     ResourceTier tier,
                                     const QString& location,
                                     const QString& category = QString())
    {
        DiscoveredResource res;
        res.path = QString("%1/templates/%2").arg(location, name);
        res.name = name;
        res.type = ResourceType::Template;
        res.tier = tier;
        res.locationKey = location;
        res.category = category;
        res.size = 1024;
        res.lastModified = QDateTime::currentDateTime();
        return res;
    }
    
    /// Helper to create a library template
    DiscoveredResource makeLibraryTemplate(const QString& name,
                                            const QString& libraryName,
                                            ResourceTier tier,
                                            const QString& location)
    {
        DiscoveredResource res;
        res.path = QString("%1/libraries/%2/templates/%3").arg(location, libraryName, name);
        res.name = name;
        res.type = ResourceType::Template;
        res.tier = tier;
        res.locationKey = location;
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
    // Model contract tests using QAbstractItemModelTester
    // ========================================================================
    
    void testEmptyModelContract()
    {
        ResourceStore store;
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        // QAbstractItemModelTester will assert if model violates contract
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
        Q_UNUSED(tester);
        
        QCOMPARE(model.rowCount(), 0);
    }
    
    void testPopulatedModelContract()
    {
        ResourceStore store;
        
        // Add templates across all tiers
        store.addResource(makeTemplate("install.json", ResourceTier::Installation, "/app/OpenSCAD"));
        store.addResource(makeTemplate("machine.json", ResourceTier::Machine, "/shared/OpenSCAD"));
        store.addResource(makeTemplate("user.json", ResourceTier::User, "/home/user/OpenSCAD"));
        
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        // QAbstractItemModelTester exercises all model methods
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
        Q_UNUSED(tester);
        
        // Should have 3 tier nodes
        QCOMPARE(model.rowCount(), 3);
    }
    
    void testModelWithLibraries()
    {
        ResourceStore store;
        
        // Mix of regular and library templates
        store.addResource(makeTemplate("regular.json", ResourceTier::Installation, "/app"));
        store.addResource(makeLibraryTemplate("lib.json", "MCAD", ResourceTier::Installation, "/app"));
        store.addResource(makeLibraryTemplate("lib2.json", "BOSL", ResourceTier::Installation, "/app"));
        
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
        Q_UNUSED(tester);
        
        // One tier with 3 locations (regular + 2 libraries)
        QCOMPARE(model.rowCount(), 1);
    }
    
    // ========================================================================
    // Structure tests
    // ========================================================================
    
    void testTreeStructure()
    {
        ResourceStore store;
        store.addResource(makeTemplate("a.json", ResourceTier::Installation, "/install"));
        store.addResource(makeTemplate("b.json", ResourceTier::Installation, "/install"));
        store.addResource(makeTemplate("c.json", ResourceTier::User, "/user"));
        
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        // Level 0: Tiers (2: Installation, User)
        QCOMPARE(model.rowCount(), 2);
        
        // Find Installation tier
        QModelIndex installTier;
        for (int i = 0; i < model.rowCount(); ++i) {
            QModelIndex idx = model.index(i, 0);
            if (model.data(idx).toString() == "Installation") {
                installTier = idx;
                break;
            }
        }
        QVERIFY(installTier.isValid());
        
        // Level 1: Location nodes (1 location under Installation)
        QCOMPARE(model.rowCount(installTier), 1);
        
        QModelIndex locationNode = model.index(0, 0, installTier);
        QVERIFY(locationNode.isValid());
        
        // Level 2: Template nodes (2 templates)
        QCOMPARE(model.rowCount(locationNode), 2);
    }
    
    void testTierNodeData()
    {
        ResourceStore store;
        store.addResource(makeTemplate("test.json", ResourceTier::Installation, "/install"));
        
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        QModelIndex tierIdx = model.index(0, 0);
        
        QCOMPARE(model.data(tierIdx, Qt::DisplayRole).toString(), QString("Installation"));
        QCOMPARE(model.data(tierIdx, TemplateTreeModel::TierRole).toInt(), 
                 static_cast<int>(ResourceTier::Installation));
        QCOMPARE(model.data(tierIdx, TemplateTreeModel::NodeTypeRole).toInt(),
                 static_cast<int>(TemplateTreeNode::NodeType::Tier));
    }
    
    void testLocationNodeData()
    {
        ResourceStore store;
        store.addResource(makeTemplate("test.json", ResourceTier::Installation, "/path/to/install"));
        
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        QModelIndex tierIdx = model.index(0, 0);
        QModelIndex locIdx = model.index(0, 0, tierIdx);
        
        // Display name should be folder name
        QCOMPARE(model.data(locIdx, Qt::DisplayRole).toString(), QString("install"));
        
        // LocationKey role gives full path
        QCOMPARE(model.data(locIdx, TemplateTreeModel::LocationKeyRole).toString(), 
                 QString("/path/to/install"));
        
        // Not a library
        QCOMPARE(model.data(locIdx, TemplateTreeModel::IsLibraryRole).toBool(), false);
    }
    
    void testLibraryLocationNodeData()
    {
        ResourceStore store;
        store.addResource(makeLibraryTemplate("test.json", "MCAD", ResourceTier::Installation, "/install"));
        
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        QModelIndex tierIdx = model.index(0, 0);
        QModelIndex locIdx = model.index(0, 0, tierIdx);
        
        // Display name should be library name
        QCOMPARE(model.data(locIdx, Qt::DisplayRole).toString(), QString("MCAD"));
        
        // Is a library
        QCOMPARE(model.data(locIdx, TemplateTreeModel::IsLibraryRole).toBool(), true);
    }
    
    void testTemplateNodeData()
    {
        ResourceStore store;
        store.addResource(makeTemplate("mytemplate.json", ResourceTier::User, "/home/user", "Parametric"));
        
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        QModelIndex tierIdx = model.index(0, 0);
        QModelIndex locIdx = model.index(0, 0, tierIdx);
        QModelIndex tmplIdx = model.index(0, 0, locIdx);
        
        // Name column
        QCOMPARE(model.data(tmplIdx, Qt::DisplayRole).toString(), QString("mytemplate.json"));
        
        // Category column
        QModelIndex catIdx = model.index(0, 1, locIdx);
        QCOMPARE(model.data(catIdx, Qt::DisplayRole).toString(), QString("Parametric"));
        
        // Path column
        QModelIndex pathIdx = model.index(0, 2, locIdx);
        QVERIFY(model.data(pathIdx, Qt::DisplayRole).toString().contains("mytemplate.json"));
        
        // PathRole
        QVERIFY(model.data(tmplIdx, TemplateTreeModel::PathRole).toString().endsWith("mytemplate.json"));
    }
    
    // ========================================================================
    // Column tests
    // ========================================================================
    
    void testColumnCount()
    {
        TemplateTreeModel model;
        QCOMPARE(model.columnCount(), 3);  // Name, Category, Path
    }
    
    void testHeaderData()
    {
        TemplateTreeModel model;
        
        QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QString("Name"));
        QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), QString("Category"));
        QCOMPARE(model.headerData(2, Qt::Horizontal).toString(), QString("Path"));
    }
    
    // ========================================================================
    // Find operations
    // ========================================================================
    
    void testFindTemplatePath()
    {
        ResourceStore store;
        auto tmpl = makeTemplate("findme.json", ResourceTier::User, "/user");
        store.addResource(tmpl);
        store.addResource(makeTemplate("other.json", ResourceTier::User, "/user"));
        
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        QModelIndex found = model.findTemplatePath(tmpl.path);
        QVERIFY(found.isValid());
        QCOMPARE(model.data(found, Qt::DisplayRole).toString(), QString("findme.json"));
        
        QModelIndex notFound = model.findTemplatePath("/nonexistent/path.json");
        QVERIFY(!notFound.isValid());
    }
    
    // ========================================================================
    // Dynamic updates
    // ========================================================================
    
    void testRebuildOnStoreChange()
    {
        ResourceStore store;
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        QCOMPARE(model.rowCount(), 0);
        
        // Add resource
        store.addResource(makeTemplate("new.json", ResourceTier::Installation, "/install"));
        
        // Model should have rebuilt
        QCOMPARE(model.rowCount(), 1);
    }
    
    void testRebuildOnClear()
    {
        ResourceStore store;
        store.addResource(makeTemplate("test.json", ResourceTier::Installation, "/install"));
        
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        QCOMPARE(model.rowCount(), 1);
        
        store.clear();
        
        QCOMPARE(model.rowCount(), 0);
    }
    
    // ========================================================================
    // Integration with scanner
    // ========================================================================
    
    void testWithRealScan()
    {
        ResourceStore store;
        ResourceScannerDirListing scanner;
        
        store.scanTypeAndStore(scanner, m_installPath, ResourceType::Template,
                               ResourceTier::Installation, "test-install");
        
        TemplateTreeModel model;
        model.setResourceStore(&store);
        
        // Model tester validates everything
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
        Q_UNUSED(tester);
        
        // Should have found templates
        QVERIFY(model.rowCount() > 0);
        
        // Navigate to first template
        QModelIndex tierIdx = model.index(0, 0);
        QVERIFY(tierIdx.isValid());
        QVERIFY(model.rowCount(tierIdx) > 0);
        
        QModelIndex locIdx = model.index(0, 0, tierIdx);
        QVERIFY(locIdx.isValid());
        QVERIFY(model.rowCount(locIdx) > 0);
    }
};

QTEST_MAIN(TestTemplateTreeModel)
#include "test_templatetreemodel.moc"
