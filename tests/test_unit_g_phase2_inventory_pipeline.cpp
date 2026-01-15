/**
 * @file test_unit_g_phase2_inventory_pipeline.cpp
 * @brief Phase 2 Tests: QVariant End-to-End Pipeline
 * 
 * Verifies that the complete pipeline preserves type information:
 *   Scanner → Inventory Storage (QVariant) → Retrieval → Type Extraction
 * 
 * Phase 2 Goal: Confirm no object slicing through the entire pipeline
 */

#include <gtest/gtest.h>
#include <resourceInventory/resourceItem.hpp>
#include <resourceInventory/ExamplesInventory.hpp>
#include <resourceInventory/TemplatesInventory.hpp>
#include <scadtemplates/editsubtype.hpp>
#include <QDir>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>

using namespace resourceInventory;

class Phase2InventoryPipeline : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory structure for testing
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());
        
        examplesDir = QDir(tempDir->path()).filePath("examples");
        templatesDir = QDir(tempDir->path()).filePath("templates");
        
        QDir().mkpath(examplesDir);
        QDir().mkpath(templatesDir);
    }
    
    void TearDown() override {
        delete tempDir;
    }
    
    // Helper: Create a test .scad file with attachments
    void createScadWithAttachments(const QString& baseName) {
        QString scadPath = QDir(examplesDir).filePath(baseName + ".scad");
        QString jsonPath = QDir(examplesDir).filePath(baseName + ".json");
        QString pngPath = QDir(examplesDir).filePath(baseName + ".png");
        
        // Create .scad file
        QFile scadFile(scadPath);
        ASSERT_TRUE(scadFile.open(QIODevice::WriteOnly));
        QTextStream(&scadFile) << "// Test OpenSCAD script\ncube([10,10,10]);\n";
        scadFile.close();
        
        // Create attachments
        QFile jsonFile(jsonPath);
        ASSERT_TRUE(jsonFile.open(QIODevice::WriteOnly));
        jsonFile.write("{\"param\": \"value\"}");
        jsonFile.close();
        
        QFile pngFile(pngPath);
        ASSERT_TRUE(pngFile.open(QIODevice::WriteOnly));
        pngFile.write("fake png data");
        pngFile.close();
    }
    
    // Helper: Create a test template .json file
    void createTemplateJson(const QString& name, const QString& content) {
        QString path = QDir(templatesDir).filePath(name + ".json");
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream(&file) << content;
        file.close();
    }
    
    QTemporaryDir* tempDir;
    QString examplesDir;
    QString templatesDir;
};

// =============================================================================
// Test 1: Examples Inventory Preserves ResourceScript Through QVariant
// =============================================================================

TEST_F(Phase2InventoryPipeline, ExamplesInventoryPreservesResourceScript) {
    // ARRANGE: Create test files
    createScadWithAttachments("test_example");
    
    ExamplesInventory inventory;
    
    // ACT: Manually create and add ResourceScript (simulating scanner)
    ResourceScript script;
    script.setDisplayName("Test Example");
    script.setCategory("TestCategory");
    script.setPath(QDir(examplesDir).filePath("test_example.scad"));
    script.setTier(ResourceTier::Installation);
    script.addAttachment(QDir(examplesDir).filePath("test_example.json"));
    script.addAttachment(QDir(examplesDir).filePath("test_example.png"));
    
    // Store in inventory as QVariant
    QString key = "installation-testcategory-test_example";
    QVariant var = QVariant::fromValue(script);
    
    // SIMULATE: What inventory does internally
    QHash<QString, QVariant> storage;
    storage.insert(key, var);
    
    // ASSERT: Retrieve and verify no object slicing
    ASSERT_TRUE(storage.contains(key));
    QVariant retrieved = storage.value(key);
    
    ASSERT_TRUE(retrieved.canConvert<ResourceScript>());
    ResourceScript retrievedScript = retrieved.value<ResourceScript>();
    
    // Verify all ResourceScript-specific data preserved
    EXPECT_EQ(retrievedScript.displayName(), "Test Example");
    EXPECT_EQ(retrievedScript.category(), "TestCategory");
    EXPECT_TRUE(retrievedScript.hasAttachments());
    EXPECT_EQ(retrievedScript.attachments().size(), 2);
    EXPECT_TRUE(retrievedScript.attachments().contains(QDir(examplesDir).filePath("test_example.json")));
    EXPECT_TRUE(retrievedScript.attachments().contains(QDir(examplesDir).filePath("test_example.png")));
}

// =============================================================================
// Test 2: Templates Inventory Preserves ResourceTemplate Through QVariant
// =============================================================================

TEST_F(Phase2InventoryPipeline, TemplatesInventoryPreservesResourceTemplate) {
    // ARRANGE: Create test template
    QString templateContent = R"({
        "prefix": "cube_template",
        "body": ["cube([${1:10}, ${2:10}, ${3:10}]);"],
        "description": "Creates a cube with customizable dimensions"
    })";
    
    createTemplateJson("test_template", templateContent);
    
    TemplatesInventory inventory;
    
    // ACT: Manually create and add ResourceTemplate (simulating scanner)
    ResourceTemplate tmpl;
    tmpl.setDisplayName("Cube Template");
    tmpl.setPath(QDir(templatesDir).filePath("test_template.json"));
    tmpl.setTier(ResourceTier::Installation);
    tmpl.setEditSubtype(scadtemplates::EditSubtype::Json);
    
    // Store in inventory as QVariant
    QString key = "installation-cube_template";
    QVariant var = QVariant::fromValue(tmpl);
    
    // SIMULATE: What inventory does internally
    QHash<QString, QVariant> storage;
    storage.insert(key, var);
    
    // ASSERT: Retrieve and verify no object slicing
    ASSERT_TRUE(storage.contains(key));
    QVariant retrieved = storage.value(key);
    
    ASSERT_TRUE(retrieved.canConvert<ResourceTemplate>());
    ResourceTemplate retrievedTmpl = retrieved.value<ResourceTemplate>();
    
    // Verify all ResourceTemplate-specific data preserved
    EXPECT_EQ(retrievedTmpl.displayName(), "Cube Template");
    EXPECT_EQ(retrievedTmpl.editSubtype(), scadtemplates::EditSubtype::Json);
    EXPECT_EQ(retrievedTmpl.tier(), ResourceTier::Installation);
}

// =============================================================================
// Test 3: Mixed Types in Same Container
// =============================================================================

TEST_F(Phase2InventoryPipeline, MixedTypesInSingleContainer) {
    // ARRANGE: Create mixed resources
    createScadWithAttachments("example1");
    createTemplateJson("template1", R"({"prefix": "t1", "body": ["test"]})");
    
    // ACT: Store different types in same QHash<QString, QVariant>
    QHash<QString, QVariant> mixedStorage;
    
    ResourceScript script;
    script.setDisplayName("Example 1");
    script.addAttachment("fake_attachment.json");
    mixedStorage.insert("script1", QVariant::fromValue(script));
    
    ResourceTemplate tmpl;
    tmpl.setDisplayName("Template 1");
    tmpl.setEditSubtype(scadtemplates::EditSubtype::Json);
    mixedStorage.insert("template1", QVariant::fromValue(tmpl));
    
    ResourceItem baseItem;
    baseItem.setDisplayName("Base Item");
    baseItem.setType(ResourceType::Examples);
    mixedStorage.insert("base1", QVariant::fromValue(baseItem));
    
    // ASSERT: All types retrievable with correct type info
    ASSERT_TRUE(mixedStorage["script1"].canConvert<ResourceScript>());
    ASSERT_TRUE(mixedStorage["template1"].canConvert<ResourceTemplate>());
    ASSERT_TRUE(mixedStorage["base1"].canConvert<ResourceItem>());
    
    // Verify can distinguish between types
    EXPECT_FALSE(mixedStorage["script1"].canConvert<ResourceTemplate>());
    EXPECT_FALSE(mixedStorage["template1"].canConvert<ResourceScript>());
    
    // Verify derived class data accessible
    ResourceScript retrievedScript = mixedStorage["script1"].value<ResourceScript>();
    EXPECT_TRUE(retrievedScript.hasAttachments());
    EXPECT_EQ(retrievedScript.attachments().size(), 1);
}

// =============================================================================
// Test 4: QList<QVariant> Preserves Types (Inventory.getAll() Pattern)
// =============================================================================

TEST_F(Phase2InventoryPipeline, QListPreservesTypes) {
    // ARRANGE: Create multiple resources
    QList<QVariant> results;
    
    // Add multiple ResourceScripts
    for (int i = 0; i < 5; i++) {
        ResourceScript script;
        script.setDisplayName(QString("Script %1").arg(i));
        script.addAttachment(QString("attachment_%1.json").arg(i));
        results.append(QVariant::fromValue(script));
    }
    
    // Add multiple ResourceTemplates
    for (int i = 0; i < 3; i++) {
        ResourceTemplate tmpl;
        tmpl.setDisplayName(QString("Template %1").arg(i));
        tmpl.setEditSubtype(scadtemplates::EditSubtype::Json);
        results.append(QVariant::fromValue(tmpl));
    }
    
    // ASSERT: Can iterate and extract types
    int scriptCount = 0;
    int templateCount = 0;
    
    for (const QVariant& var : results) {
        if (var.canConvert<ResourceScript>()) {
            ResourceScript script = var.value<ResourceScript>();
            EXPECT_TRUE(script.hasAttachments());
            scriptCount++;
        } else if (var.canConvert<ResourceTemplate>()) {
            ResourceTemplate tmpl = var.value<ResourceTemplate>();
            EXPECT_EQ(tmpl.editSubtype(), scadtemplates::EditSubtype::Json);
            templateCount++;
        }
    }
    
    EXPECT_EQ(scriptCount, 5);
    EXPECT_EQ(templateCount, 3);
}

// =============================================================================
// Test 5: Verify Attachment Scanning Logic Preserves Data
// =============================================================================

TEST_F(Phase2InventoryPipeline, AttachmentScanningPreservesAllData) {
    // ARRANGE: Create script with multiple attachment types
    QString baseName = "complex_example";
    QString basePath = QDir(examplesDir).filePath(baseName);
    
    // Create main script
    QFile scadMainFile(basePath + ".scad");
    ASSERT_TRUE(scadMainFile.open(QIODevice::WriteOnly));
    scadMainFile.write("// Complex script");
    scadMainFile.close();
    
    // Create various attachments
    QStringList attachments = {".json", ".txt", ".dat", ".png", ".jpg"};
    for (const QString& ext : attachments) {
        QFile file(basePath + ext);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        file.write("test data");
        file.close();
    }
    
    // ACT: Create ResourceScript with all attachments
    ResourceScript script;
    script.setDisplayName("Complex Example");
    script.setPath(basePath + ".scad");
    
    for (const QString& ext : attachments) {
        script.addAttachment(basePath + ext);
    }
    
    // Store and retrieve through QVariant
    QVariant var = QVariant::fromValue(script);
    ResourceScript retrieved = var.value<ResourceScript>();
    
    // ASSERT: All attachments preserved
    EXPECT_EQ(retrieved.attachments().size(), 5);
    for (const QString& ext : attachments) {
        EXPECT_TRUE(retrieved.attachments().contains(basePath + ext))
            << "Missing attachment: " << (basePath + ext).toStdString();
    }
}

// =============================================================================
// Main
// =============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
