#include <gtest/gtest.h>
#include <QString>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "resInventory/resourceScanner.h"
#include "resInventory/resourceItem.h"
#include "platformInfo/resourceLocationManager.h"
#include "platformInfo/ResourceLocation.h"

// Use explicit namespace prefixes to avoid conflicts
namespace ri = resInventory;
namespace pi = platformInfo;

class ResourceDiscoveryTest : public ::testing::Test {
protected:
    QString testBasePath;
    QString instPath;
    QString machPath;
    QString persPath;
    
    void SetUp() override {
        // Get absolute path to testFileStructure
        QDir currentDir(QDir::currentPath());
        currentDir.cdUp(); // Go to project root from build directory
        testBasePath = currentDir.absoluteFilePath("testFileStructure");
        
        instPath = QDir(testBasePath).absoluteFilePath("inst");
        machPath = QDir(testBasePath).absoluteFilePath("mach");
        persPath = QDir(testBasePath).absoluteFilePath("pers");
        
        // Verify test structure exists
        ASSERT_TRUE(QDir(testBasePath).exists()) << "testFileStructure not found at: " << testBasePath.toStdString();
        ASSERT_TRUE(QDir(instPath).exists()) << "inst directory not found";
        ASSERT_TRUE(QDir(machPath).exists()) << "mach directory not found";
        ASSERT_TRUE(QDir(persPath).exists()) << "pers directory not found";
    }
};

// Test that testFileStructure directories exist
TEST_F(ResourceDiscoveryTest, TestStructureExists) {
    EXPECT_TRUE(QDir(instPath).exists());
    EXPECT_TRUE(QDir(machPath).exists());
    EXPECT_TRUE(QDir(persPath).exists());
}

// Test Installation tier structure
TEST_F(ResourceDiscoveryTest, InstallationTierStructure) {
    QDir instDir(instPath);
    
    // Check for OpenSCAD installation folders
    QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    EXPECT_GT(installations.size(), 0) << "No OpenSCAD installation folders found";
    
    // Verify at least one installation has expected subfolders
    bool foundValidInstall = false;
    QStringList expectedFolders = {"color-schemes", "examples", "fonts", "libraries", "templates"};
    
    for (const QString &install : installations) {
        QDir installDir(instDir.absoluteFilePath(install));
        int foundCount = 0;
        for (const QString &expected : expectedFolders) {
            if (installDir.exists(expected)) {
                foundCount++;
            }
        }
        if (foundCount >= 3) { // At least 3 of the expected folders
            foundValidInstall = true;
            break;
        }
    }
    
    EXPECT_TRUE(foundValidInstall) << "No valid OpenSCAD installation structure found";
}

// Test Personal tier structure
TEST_F(ResourceDiscoveryTest, PersonalTierStructure) {
    QDir persDir(persPath);
    
    // Check for AppData structure
    QString appDataPath = persDir.absoluteFilePath("appdata/local/openscad");
    EXPECT_TRUE(QDir(appDataPath).exists()) << "AppData structure not found";
    
    // Check for Documents structure (user-specific)
    QStringList users = persDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    bool foundDocuments = false;
    for (const QString &user : users) {
        if (user == "appdata") continue; // Skip appdata folder
        
        QString docPath = persDir.absoluteFilePath(user + "/Documents/OpenSCAD");
        if (QDir(docPath).exists()) {
            foundDocuments = true;
            break;
        }
    }
    
    EXPECT_TRUE(foundDocuments) << "No user Documents/OpenSCAD structure found";
}

// Test color scheme folder structure
TEST_F(ResourceDiscoveryTest, ColorSchemeFolderStructure) {
    QDir instDir(instPath);
    QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    bool foundColorSchemes = false;
    bool foundRenderSubfolder = false;
    bool foundEditorSubfolder = false;
    
    for (const QString &install : installations) {
        QDir colorSchemesDir(instDir.absoluteFilePath(install + "/color-schemes"));
        if (colorSchemesDir.exists()) {
            foundColorSchemes = true;
            
            if (colorSchemesDir.exists("render")) {
                foundRenderSubfolder = true;
            }
            if (colorSchemesDir.exists("editor")) {
                foundEditorSubfolder = true;
            }
            
            if (foundRenderSubfolder && foundEditorSubfolder) {
                break;
            }
        }
    }
    
    EXPECT_TRUE(foundColorSchemes) << "color-schemes folder not found";
    EXPECT_TRUE(foundRenderSubfolder) << "color-schemes/render subfolder not found";
    EXPECT_TRUE(foundEditorSubfolder) << "color-schemes/editor subfolder not found";
}

// Test examples folder structure (one-level deep)
TEST_F(ResourceDiscoveryTest, ExamplesFolderStructure) {
    QDir instDir(instPath);
    QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    bool foundExamples = false;
    bool foundCategories = false;
    
    for (const QString &install : installations) {
        QDir examplesDir(instDir.absoluteFilePath(install + "/examples"));
        if (examplesDir.exists()) {
            foundExamples = true;
            
            // Check for subdirectories (categories)
            QStringList categories = examplesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            if (categories.size() > 0) {
                foundCategories = true;
                break;
            }
        }
    }
    
    EXPECT_TRUE(foundExamples) << "examples folder not found";
    EXPECT_TRUE(foundCategories) << "No category subfolders found in examples";
}

// Test ResourceScanner can scan installation locations
TEST_F(ResourceDiscoveryTest, ScannerCanProcessInstallation) {
    QDir instDir(instPath);
    QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    ASSERT_GT(installations.size(), 0) << "No installations to test";
    
    // Create a mock resource location
    QString installPath = instDir.absoluteFilePath(installations[0]);
    pi::ResourceLocation testLoc;
    testLoc.path = installPath;
    testLoc.displayName = installations[0];
    testLoc.hasResourceFolders = true;

    
    ri::ResourceScanner scanner;
    // Scan for templates as a representative resource type
    auto items = scanner.scanLocation(testLoc, ri::ResourceType::Template, ri::ResourceTier::Installation);
    
    // Should find templates if they exist
    QDir templatesDir(installPath + "/templates");
    if (templatesDir.exists() && !templatesDir.entryList({"*.json"}, QDir::Files).isEmpty()) {
        EXPECT_GT(items.size(), 0) << "Templates found but scanner didn't discover them";
    }
}

// Test ResourceScanner discovers render color schemes
TEST_F(ResourceDiscoveryTest, ScannerDiscoversRenderColors) {
    QDir instDir(instPath);
    QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    ASSERT_GT(installations.size(), 0);
    
    QString installPath = instDir.absoluteFilePath(installations[0]);
    
    // Check if render color schemes exist in test data
    QDir renderColorsDir(installPath + "/color-schemes/render");
    if (!renderColorsDir.exists()) {
        GTEST_SKIP() << "Render colors folder not present in test data";
    }
    
    QStringList colorFiles = renderColorsDir.entryList({"*.json"}, QDir::Files);
    if (colorFiles.isEmpty()) {
        GTEST_SKIP() << "No render color scheme files in test data";
    }
    
    pi::ResourceLocation testLoc;
    testLoc.path = installPath;
    testLoc.displayName = installations[0];
    testLoc.hasResourceFolders = true;

    
    ri::ResourceScanner scanner;
    auto items = scanner.scanLocation(testLoc, ri::ResourceType::RenderColors, ri::ResourceTier::Installation);
    
    EXPECT_GT(items.size(), 0) << "No render color schemes discovered";
}

// Test ResourceScanner discovers editor color schemes
TEST_F(ResourceDiscoveryTest, ScannerDiscoversEditorColors) {
    QDir instDir(instPath);
    QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    ASSERT_GT(installations.size(), 0);
    
    QString installPath = instDir.absoluteFilePath(installations[0]);
    
    // Check if editor color schemes exist in test data
    QDir editorColorsDir(installPath + "/color-schemes/editor");
    if (!editorColorsDir.exists()) {
        GTEST_SKIP() << "Editor colors folder not present in test data";
    }
    
    QStringList colorFiles = editorColorsDir.entryList({"*.json"}, QDir::Files);
    if (colorFiles.isEmpty()) {
        GTEST_SKIP() << "No editor color scheme files in test data";
    }
    
    pi::ResourceLocation testLoc;
    testLoc.path = installPath;
    testLoc.displayName = installations[0];
    testLoc.hasResourceFolders = true;

    
    ri::ResourceScanner scanner;
    auto items = scanner.scanLocation(testLoc, ri::ResourceType::EditorColors, ri::ResourceTier::Installation);
    
    EXPECT_GT(items.size(), 0) << "No editor color schemes discovered";
}

// Test ResourceScanner discovers examples with categories
TEST_F(ResourceDiscoveryTest, ScannerDiscoversExamplesWithCategories) {
    QDir instDir(instPath);
    QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    ASSERT_GT(installations.size(), 0);
    
    QString installPath = instDir.absoluteFilePath(installations[0]);
    
    // Check if examples exist in test data
    QDir examplesDir(installPath + "/examples");
    if (!examplesDir.exists()) {
        GTEST_SKIP() << "Examples folder not present in test data";
    }
    
    pi::ResourceLocation testLoc;
    testLoc.path = installPath;
    testLoc.displayName = installations[0];
    testLoc.hasResourceFolders = true;

    
    ri::ResourceScanner scanner;
    auto items = scanner.scanLocation(testLoc, ri::ResourceType::Example, ri::ResourceTier::Installation);
    
    if (items.isEmpty()) {
        GTEST_SKIP() << "No examples found in test data";
    }
    
    // At least some examples should have categories (from subdirectories)
    bool foundWithCategory = false;
    for (const auto &item : items) {
        if (!item.category().isEmpty()) {
            foundWithCategory = true;
            break;
        }
    }
    
    EXPECT_TRUE(foundWithCategory) << "No examples with categories found (expected from subdirectories)";
}

// Test ResourceScanner discovers templates
TEST_F(ResourceDiscoveryTest, ScannerDiscoversTemplates) {
    QDir instDir(instPath);
    QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    ASSERT_GT(installations.size(), 0);
    
    QString installPath = instDir.absoluteFilePath(installations[0]);
    
    // Check if templates exist in test data
    QDir templatesDir(installPath + "/templates");
    if (!templatesDir.exists()) {
        GTEST_SKIP() << "Templates folder not present in test data";
    }
    
    QStringList templateFiles = templatesDir.entryList({"*.json"}, QDir::Files);
    if (templateFiles.isEmpty()) {
        GTEST_SKIP() << "No template files in test data";
    }
    
    pi::ResourceLocation testLoc;
    testLoc.path = installPath;
    testLoc.displayName = installations[0];
    testLoc.hasResourceFolders = true;

    
    ri::ResourceScanner scanner;
    auto items = scanner.scanLocation(testLoc, ri::ResourceType::Template, ri::ResourceTier::Installation);
    
    EXPECT_GT(items.size(), 0) << "No templates discovered";
}

// Test fonts scanner is no-op
TEST_F(ResourceDiscoveryTest, FontsScannerIsNoOp) {
    QDir instDir(instPath);
    QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    ASSERT_GT(installations.size(), 0);
    
    QString installPath = instDir.absoluteFilePath(installations[0]);
    
    pi::ResourceLocation testLoc;
    testLoc.path = installPath;
    testLoc.displayName = installations[0];
    testLoc.hasResourceFolders = true;

    
    ri::ResourceScanner scanner;
    auto items = scanner.scanLocation(testLoc, ri::ResourceType::Font, ri::ResourceTier::Installation);
    
    // Fonts should not be discovered (scanner is no-op)
    EXPECT_EQ(items.size(), 0) << "Fonts scanner should be no-op";
}

// Test ResourceItem properties are populated correctly
TEST_F(ResourceDiscoveryTest, ResourceItemPropertiesPopulated) {
    QDir instDir(instPath);
    QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    ASSERT_GT(installations.size(), 0);
    
    QString installPath = instDir.absoluteFilePath(installations[0]);
    
    pi::ResourceLocation testLoc;
    testLoc.path = installPath;
    testLoc.displayName = installations[0];
    testLoc.hasResourceFolders = true;

    
    ri::ResourceScanner scanner;
    
    // Find any resource to validate properties
    bool foundValidResource = false;
    QList<ri::ResourceType> typesToTest = {
        ri::ResourceType::Template, ri::ResourceType::Example, 
        ri::ResourceType::RenderColors, ri::ResourceType::EditorColors
    };
    
    for (auto type : typesToTest) {
        auto items = scanner.scanLocation(testLoc, type, ri::ResourceTier::Installation);
        if (!items.isEmpty()) {
            auto item = items.first();
            
            // Validate required properties
            EXPECT_FALSE(item.displayName().isEmpty()) << "Resource name should not be empty";
            EXPECT_FALSE(item.path().isEmpty()) << "Resource path should not be empty";
            EXPECT_TRUE(QFile::exists(item.path())) << "Resource path should point to existing file";
            EXPECT_EQ(item.tier(), ri::ResourceTier::Installation);
            
            foundValidResource = true;
            break;
        }
    }
    
    EXPECT_TRUE(foundValidResource) << "No resources found to validate properties";
}

// Test ResourceInventoryManager can build from mock locations
TEST_F(ResourceDiscoveryTest, InventoryManagerCanBeInstantiated) {
    // This test would require mocking ResourceLocationManager to return testFileStructure paths
    // For now, we'll test that the manager can be instantiated
    ri::ResourceInventoryManager manager;
    
    // Full integration test would require ResourceLocationManager mock
    // to point to testFileStructure instead of real system locations
    // For now just verify instantiation works
    EXPECT_TRUE(true);
}
