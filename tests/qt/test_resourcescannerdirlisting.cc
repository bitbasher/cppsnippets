/**
 * @file tst_resourcescannerdirlisting.cpp
 * @brief Qt Test unit tests for ResourceScannerDirListing
 * 
 * Tests the QDirListing-based scanner implementation using Qt Test framework.
 * This is a proof-of-concept demonstrating Qt Test with the new scanner.
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>

#include "resInventory/resourceScannerDirListing.h"

using namespace resInventory;

/**
 * @brief Test class for ResourceScannerDirListing
 * 
 * Uses Qt Test framework with:
 * - QSignalSpy for signal verification
 * - Data-driven tests with _data() methods
 * - Test fixture via testFileStructure/
 */
class TestResourceScannerDirListing : public QObject
{
    Q_OBJECT

private:
    QString m_testDataPath;     ///< Path to testFileStructure
    QString m_installPath;       ///< Path to inst/OpenSCAD
    
private slots:
    // ========================================================================
    // Test fixture setup/teardown
    // ========================================================================
    
    void initTestCase()
    {
        // Find testFileStructure relative to build directory
        // The test executable runs from build/bin, testFileStructure is in project root
        QDir dir(QCoreApplication::applicationDirPath());
        
        // Try various relative paths to find testFileStructure
        QStringList searchPaths = {
            dir.absoluteFilePath("../../testFileStructure"),           // build/bin -> root
            dir.absoluteFilePath("../../../testFileStructure"),        // deeper build
            QDir::currentPath() + "/testFileStructure",                // current dir
            QString(SRCDIR) + "/testFileStructure"                     // CMake SRCDIR if defined
        };
        
        for (const QString& path : searchPaths) {
            QDir testDir(path);
            if (testDir.exists() && testDir.exists("inst/OpenSCAD")) {
                m_testDataPath = testDir.absolutePath();
                m_installPath = testDir.absoluteFilePath("inst/OpenSCAD");
                qDebug() << "Found test data at:" << m_testDataPath;
                break;
            }
        }
        
        QVERIFY2(!m_testDataPath.isEmpty(), 
                 "Could not find testFileStructure directory");
        QVERIFY2(QDir(m_installPath).exists(),
                 "Could not find inst/OpenSCAD in test data");
    }
    
    void cleanupTestCase()
    {
        // Nothing to clean up - using existing fixture
    }
    
    // ========================================================================
    // Static method tests
    // ========================================================================
    
    void testResourceSubfolder_data()
    {
        QTest::addColumn<int>("resourceType");
        QTest::addColumn<QString>("expectedSubfolder");
        
        QTest::newRow("RenderColors") << static_cast<int>(ResourceType::RenderColors) << "color-schemes/render";
        QTest::newRow("EditorColors") << static_cast<int>(ResourceType::EditorColors) << "color-schemes/editor";
        QTest::newRow("Font")         << static_cast<int>(ResourceType::Font)         << "fonts";
        QTest::newRow("Library")      << static_cast<int>(ResourceType::Library)      << "libraries";
        QTest::newRow("Example")      << static_cast<int>(ResourceType::Example)      << "examples";
        QTest::newRow("Test")         << static_cast<int>(ResourceType::Test)         << "tests";
        QTest::newRow("Template")     << static_cast<int>(ResourceType::Template)     << "templates";
        QTest::newRow("Translation")  << static_cast<int>(ResourceType::Translation)  << "locale";
        QTest::newRow("Unknown")      << static_cast<int>(ResourceType::Unknown)      << "";
    }
    
    void testResourceSubfolder()
    {
        QFETCH(int, resourceType);
        QFETCH(QString, expectedSubfolder);
        
        QString result = ResourceScannerDirListing::resourceSubfolder(
            static_cast<ResourceType>(resourceType));
        
        QCOMPARE(result, expectedSubfolder);
    }
    
    void testResourceFilters_data()
    {
        QTest::addColumn<int>("resourceType");
        QTest::addColumn<QStringList>("expectedFilters");
        
        QTest::newRow("RenderColors") << static_cast<int>(ResourceType::RenderColors) 
                                      << QStringList{"*.json"};
        QTest::newRow("Font")         << static_cast<int>(ResourceType::Font)
                                      << QStringList{"*.ttf", "*.otf", "*.woff", "*.woff2"};
        QTest::newRow("Library")      << static_cast<int>(ResourceType::Library)
                                      << QStringList{"*.scad"};
        QTest::newRow("Translation")  << static_cast<int>(ResourceType::Translation)
                                      << QStringList{"*.qm", "*.ts"};
    }
    
    void testResourceFilters()
    {
        QFETCH(int, resourceType);
        QFETCH(QStringList, expectedFilters);
        
        QStringList result = ResourceScannerDirListing::resourceFilters(
            static_cast<ResourceType>(resourceType));
        
        QCOMPARE(result, expectedFilters);
    }
    
    void testCategorizeByPath_data()
    {
        QTest::addColumn<QString>("path");
        QTest::addColumn<int>("expectedType");
        
        QTest::newRow("color-schemes render")
            << "/some/path/color-schemes/render/default.json"
            << static_cast<int>(ResourceType::RenderColors);
            
        QTest::newRow("color-schemes editor")
            << "/some/path/color-schemes/editor/dark.json"
            << static_cast<int>(ResourceType::EditorColors);
            
        QTest::newRow("fonts")
            << "/usr/share/openscad/fonts/Liberation/LiberationSans.ttf"
            << static_cast<int>(ResourceType::Font);
            
        QTest::newRow("libraries")
            << "C:/Program Files/OpenSCAD/libraries/MCAD/gears.scad"
            << static_cast<int>(ResourceType::Library);
            
        QTest::newRow("examples")
            << "/opt/openscad/examples/Basics/logo.scad"
            << static_cast<int>(ResourceType::Example);
            
        QTest::newRow("templates")
            << "/home/user/.local/share/openscad/templates/for_loop.scad"
            << static_cast<int>(ResourceType::Template);
            
        QTest::newRow("locale")
            << "/app/locale/de/openscad_de.qm"
            << static_cast<int>(ResourceType::Translation);
            
        QTest::newRow("unknown path")
            << "/random/path/file.txt"
            << static_cast<int>(ResourceType::Unknown);
    }
    
    void testCategorizeByPath()
    {
        QFETCH(QString, path);
        QFETCH(int, expectedType);
        
        ResourceType result = ResourceScannerDirListing::categorizeByPath(path);
        
        QCOMPARE(static_cast<int>(result), expectedType);
    }
    
    // ========================================================================
    // Scanner functionality tests
    // ========================================================================
    
    void testScanNonexistentPath()
    {
        ResourceScannerDirListing scanner;
        QSignalSpy errorSpy(&scanner, &ResourceScannerDirListing::scanError);
        
        int count = scanner.scanLocation(
            "/nonexistent/path/that/does/not/exist",
            ResourceTier::User,
            "test-location",
            [](const DiscoveredResource&) {});
        
        QCOMPARE(count, 0);
        QCOMPARE(errorSpy.count(), 1);
    }
    
    void testScanRenderColors()
    {
        ResourceScannerDirListing scanner;
        QVector<DiscoveredResource> results;
        
        int count = scanner.scanLocationForType(
            m_installPath,
            ResourceType::RenderColors,
            ResourceTier::Installation,
            "test-install",
            [&results](const DiscoveredResource& res) {
                results.append(res);
            });
        
        // testFileStructure has render color schemes
        QVERIFY2(count > 0, "Expected to find render color schemes");
        QCOMPARE(results.size(), count);
        
        // Verify all results are the correct type
        for (const auto& res : results) {
            QCOMPARE(res.type, ResourceType::RenderColors);
            QCOMPARE(res.tier, ResourceTier::Installation);
            QCOMPARE(res.locationKey, QString("test-install"));
            QVERIFY(res.path.endsWith(".json"));
        }
    }
    
    void testScanEditorColors()
    {
        ResourceScannerDirListing scanner;
        QVector<DiscoveredResource> results;
        
        int count = scanner.scanLocationForType(
            m_installPath,
            ResourceType::EditorColors,
            ResourceTier::Installation,
            "test-install",
            [&results](const DiscoveredResource& res) {
                results.append(res);
            });
        
        // testFileStructure has editor color schemes
        QVERIFY2(count > 0, "Expected to find editor color schemes");
        
        for (const auto& res : results) {
            QCOMPARE(res.type, ResourceType::EditorColors);
            QVERIFY(res.path.endsWith(".json"));
        }
    }
    
    void testScanTemplates()
    {
        ResourceScannerDirListing scanner;
        QVector<DiscoveredResource> results;
        
        int count = scanner.scanLocationForType(
            m_installPath,
            ResourceType::Template,
            ResourceTier::Installation,
            "test-install",
            [&results](const DiscoveredResource& res) {
                results.append(res);
            });
        
        // testFileStructure has templates (recursive scan)
        QVERIFY2(count > 0, "Expected to find templates");
        
        for (const auto& res : results) {
            QCOMPARE(res.type, ResourceType::Template);
            QVERIFY2(res.path.endsWith(".json"), 
                     qPrintable(QString("Templates must be .json files, got: %1").arg(res.path)));
        }
    }
    
    void testTemplatesIgnoreNonJsonFiles()
    {
        // Test that .scad, .jpeg and other non-JSON files in templates folder are silently ignored
        // testFileStructure/inst/OpenSCAD/templates/ contains:
        //   - inst_template_basic_cube.json (valid)
        //   - inst_template_sphere_param.json (valid)
        //   - fakemodel.scad (should be ignored)
        //   - fakephoto.jpeg (should be ignored)
        
        ResourceScannerDirListing scanner;
        QSignalSpy errorSpy(&scanner, &ResourceScannerDirListing::scanError);
        QVector<DiscoveredResource> results;
        
        int count = scanner.scanLocationForType(
            m_installPath,
            ResourceType::Template,
            ResourceTier::Installation,
            "test-install",
            [&results](const DiscoveredResource& res) {
                results.append(res);
            });
        
        // Should find only the 2 JSON templates, not the .scad or .jpeg files
        QCOMPARE(count, 2);
        QCOMPARE(results.size(), 2);
        
        // No errors should have been emitted for ignored files
        QCOMPARE(errorSpy.count(), 0);
        
        // Verify none of the results are .scad or .jpeg
        for (const auto& res : results) {
            QVERIFY2(!res.path.endsWith(".scad"), 
                     qPrintable(QString("Should not scan .scad in templates: %1").arg(res.path)));
            QVERIFY2(!res.path.endsWith(".jpeg"), 
                     qPrintable(QString("Should not scan .jpeg in templates: %1").arg(res.path)));
            QVERIFY(res.path.endsWith(".json"));
        }
    }
    
    void testCollectAll()
    {
        ResourceScannerDirListing scanner;
        
        QVector<DiscoveredResource> results = scanner.collectAll(
            m_installPath,
            ResourceTier::Installation,
            "test-install");
        
        // Should find multiple resource types
        QVERIFY2(!results.isEmpty(), "Expected to find some resources");
        
        // Check for variety of types
        QSet<ResourceType> foundTypes;
        for (const auto& res : results) {
            foundTypes.insert(res.type);
        }
        
        qDebug() << "Found resource types:" << foundTypes.size();
        QVERIFY2(foundTypes.size() >= 2, 
                 "Expected to find at least 2 different resource types");
    }
    
    // ========================================================================
    // Signal tests (demonstrating Qt Test's QSignalSpy)
    // ========================================================================
    
    void testSignalsEmitted()
    {
        ResourceScannerDirListing scanner;
        
        QSignalSpy startedSpy(&scanner, &ResourceScannerDirListing::scanStarted);
        QSignalSpy foundSpy(&scanner, &ResourceScannerDirListing::resourceFound);
        QSignalSpy completedSpy(&scanner, &ResourceScannerDirListing::scanCompleted);
        
        QVERIFY(startedSpy.isValid());
        QVERIFY(foundSpy.isValid());
        QVERIFY(completedSpy.isValid());
        
        scanner.scanLocationForType(
            m_installPath,
            ResourceType::RenderColors,
            ResourceTier::Installation,
            "test-install",
            [](const DiscoveredResource&) {});
        
        // Verify signals were emitted
        QCOMPARE(startedSpy.count(), 1);
        QCOMPARE(completedSpy.count(), 1);
        QVERIFY(foundSpy.count() > 0);  // At least some resources found
        
        // Verify signal parameters
        QList<QVariant> startedArgs = startedSpy.takeFirst();
        QVERIFY(startedArgs.at(0).toString().contains("color-schemes"));
        QCOMPARE(startedArgs.at(1).toInt(), static_cast<int>(ResourceType::RenderColors));
        
        QList<QVariant> completedArgs = completedSpy.takeFirst();
        QVERIFY(completedArgs.at(0).toString().contains("color-schemes"));
        QVERIFY(completedArgs.at(1).toInt() > 0);  // count > 0
    }
    
    // ========================================================================
    // Callback streaming test
    // ========================================================================
    
    void testStreamingCallback()
    {
        ResourceScannerDirListing scanner;
        
        int callbackCount = 0;
        QStringList foundNames;
        
        scanner.scanLocationForType(
            m_installPath,
            ResourceType::RenderColors,
            ResourceTier::Installation,
            "streaming-test",
            [&](const DiscoveredResource& res) {
                ++callbackCount;
                foundNames.append(res.name);
                
                // Verify each resource as it streams
                QVERIFY(!res.path.isEmpty());
                QVERIFY(!res.name.isEmpty());
                QCOMPARE(res.tier, ResourceTier::Installation);
            });
        
        QVERIFY(callbackCount > 0);
        qDebug() << "Streamed" << callbackCount << "resources:" << foundNames;
    }
    
    // ========================================================================
    // DiscoveredResource struct tests
    // ========================================================================
    
    void testDiscoveredResourceDefaults()
    {
        DiscoveredResource res;
        
        QVERIFY(res.path.isEmpty());
        QVERIFY(res.name.isEmpty());
        QVERIFY(res.category.isEmpty());
        QVERIFY(res.locationKey.isEmpty());
        QCOMPARE(res.type, ResourceType::Unknown);
        QCOMPARE(res.tier, ResourceTier::User);
        QCOMPARE(res.size, 0);
        QVERIFY(!res.lastModified.isValid());
    }
    
    void testResourceMetadata()
    {
        ResourceScannerDirListing scanner;
        QVector<DiscoveredResource> results;
        
        scanner.scanLocationForType(
            m_installPath,
            ResourceType::RenderColors,
            ResourceTier::Installation,
            "metadata-test",
            [&results](const DiscoveredResource& res) {
                results.append(res);
            });
        
        QVERIFY(!results.isEmpty());
        
        // Verify metadata is populated
        const auto& first = results.first();
        QVERIFY(!first.path.isEmpty());
        QVERIFY(!first.name.isEmpty());
        QVERIFY(first.lastModified.isValid());
        QVERIFY(first.size > 0);  // JSON files have content
    }
};

// Register metatype for signal spy
Q_DECLARE_METATYPE(DiscoveredResource)

QTEST_MAIN(TestResourceScannerDirListing)
#include "test_resourcescannerdirlisting.moc"
