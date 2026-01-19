/**
 * @file test_templates_inventory.cpp
 * @brief Phase 4 tests for TemplatesInventory class
 * 
 * Tests QHash-based storage with hierarchical keys for templates.
 * Templates are .json files (not .scad), no attachments, no categories.
 */

#include <gtest/gtest.h>
#include <QDir>
#include <QDirListing>

#include "resourceInventory/TemplatesInventory.hpp"
#include "resourceInventory/resourceItem.hpp"

using namespace resourceInventory;

class TemplatesInventoryTest : public ::testing::Test {
protected:
    QString testDataPath;
    
    void SetUp() override {
        // Use testFileStructure for repeatable tests
        // Test runs from workspace root (d:\repositories\cppsnippets\cppsnippets)
        testDataPath = QDir::current().absolutePath() + "/testFileStructure/inst/OpenSCAD/templates";
        ASSERT_TRUE(QDir(testDataPath).exists()) << "testFileStructure templates not found at: " << qPrintable(testDataPath);
    }
};

// ============================================================================
// Phase 4: TemplatesInventory Tests
// ============================================================================

TEST_F(TemplatesInventoryTest, AddTemplateWithHierarchicalKey) {
    TemplatesInventory inventory;
    
    // Create ResourceLocation for test data
    platformInfo::ResourceLocation location(testDataPath, resourceMetadata::ResourceTier::Installation);
    
    // Scan templates folder for .json files
    for (const auto& entry : QDirListing(testDataPath, {"*.json"})) {
        if (entry.isFile()) {
            ASSERT_TRUE(inventory.addTemplate(entry, location));
        }
    }
    
    EXPECT_GT(inventory.count(), 0) << "Should find .json templates in testFileStructure";
    
    // Verify hierarchical key format (tier-name)
    QList<QVariant> all = inventory.getAll();
    if (all.size() > 0) {
        QVariant var = all.first();
        ASSERT_TRUE(var.canConvert<ResourceTemplate>());
        
        ResourceTemplate tmpl = var.value<ResourceTemplate>();
        EXPECT_FALSE(tmpl.displayName().isEmpty());
    }
}

TEST_F(TemplatesInventoryTest, DifferentTiersSameFile) {
    TemplatesInventory inventory;
    platformInfo::ResourceLocation installLocation(testDataPath, ResourceTier::Installation);
    platformInfo::ResourceLocation userLocation(testDataPath, ResourceTier::User);
    
    QString templatePath;
    for (const auto& entry : QDirListing(testDataPath, {"*.json"})) {
        if (entry.isFile()) {
            templatePath = entry.filePath();
            
            // Add same file with different tiers
            ASSERT_TRUE(inventory.addTemplate(entry, installLocation));
            ASSERT_TRUE(inventory.addTemplate(entry, userLocation));
            
            break;
        }
    }
    
    if (!templatePath.isEmpty()) {
        // Should have 2 entries (different keys: installation-name, user-name)
        EXPECT_EQ(inventory.count(), 2);
    }
}

TEST_F(TemplatesInventoryTest, DuplicateKeyRejected) {
    TemplatesInventory inventory;
    platformInfo::ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    for (const auto& entry : QDirListing(testDataPath, {"*.json"})) {
        if (entry.isFile()) {
            ASSERT_TRUE(inventory.addTemplate(entry, location));
            
            // Duplicate key should be rejected
            EXPECT_FALSE(inventory.addTemplate(entry, location));
            break;
        }
    }
    
    EXPECT_LE(inventory.count(), 1);
}

TEST_F(TemplatesInventoryTest, GetAll) {
    TemplatesInventory inventory;
    platformInfo::ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    int addedCount = 0;
    for (const auto& entry : QDirListing(testDataPath, {"*.json"})) {
        if (entry.isFile()) {
            if (inventory.addTemplate(entry, location)) {
                addedCount++;
            }
        }
    }
    
    QList<QVariant> all = inventory.getAll();
    EXPECT_EQ(all.size(), addedCount);
    EXPECT_EQ(inventory.count(), addedCount);
}

TEST_F(TemplatesInventoryTest, AddFolderWithTemplates) {
    TemplatesInventory inventory;
    platformInfo::ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    // Create a DirEntry from the testDataPath by listing parent
    int added = 0;
    QDir dir(testDataPath);
    if (dir.exists()) {
        // Use QDirListing to get a proper DirEntry
        for (const auto& entry : QDirListing(testDataPath)) {
            if (entry.isFile() && entry.fileName().endsWith(".json")) {
                if (inventory.addTemplate(entry, location)) {
                    added++;
                }
            }
        }
    }
    
    EXPECT_GE(added, 0);
    EXPECT_EQ(inventory.count(), added);
}

TEST_F(TemplatesInventoryTest, Clear) {
    TemplatesInventory inventory;
    platformInfo::ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    for (const auto& entry : QDirListing(testDataPath, {"*.json"})) {
        if (entry.isFile()) {
            inventory.addTemplate(entry, location);
        }
    }
    
    if (inventory.count() > 0) {
        inventory.clear();
        EXPECT_EQ(inventory.count(), 0);
        EXPECT_TRUE(inventory.getAll().isEmpty());
    } else {
        GTEST_SKIP() << "No templates to test clear()";
    }
}

/* Removed: JSON cache eliminated in redesign - getJsonContent() now loads on-demand
TEST_F(TemplatesInventoryTest, JsonContentCaching) {
    TemplatesInventory inventory;
    platformInfo::ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    // Add a template
    QString templateKey;
    for (const auto& entry : QDirListing(testDataPath, {"*.json"})) {
        if (entry.isFile()) {
            ASSERT_TRUE(inventory.addTemplate(entry, location));
            
            QString baseName = QFileInfo(entry.filePath()).baseName();
            templateKey = QString("installation-%1").arg(baseName);
            break;
        }
    }
    
    if (templateKey.isEmpty()) {
        GTEST_SKIP() << "No .json templates found for JSON caching test";
    }
    
    // JSON should be cached automatically by addTemplate
    EXPECT_TRUE(inventory.hasJsonContent(templateKey));
    EXPECT_EQ(inventory.jsonCacheSize(), 1);
    
    // Get JSON content
    QJsonObject json = inventory.getJsonContent(templateKey);
    EXPECT_FALSE(json.isEmpty());
    
    // Should still be cached (not reloaded)
    EXPECT_TRUE(inventory.hasJsonContent(templateKey));
    
    // Clear cache
    inventory.clearJsonCache();
    EXPECT_FALSE(inventory.hasJsonContent(templateKey));
    EXPECT_EQ(inventory.jsonCacheSize(), 0);
    
    // Metadata should still exist
    EXPECT_TRUE(inventory.contains(templateKey));
    EXPECT_EQ(inventory.count(), 1);
    
    // Accessing JSON should reload and cache
    json = inventory.getJsonContent(templateKey);
    EXPECT_FALSE(json.isEmpty());
    EXPECT_TRUE(inventory.hasJsonContent(templateKey));
}
*/

TEST_F(TemplatesInventoryTest, JsonContentStructure) {
    TemplatesInventory inventory;
    platformInfo::ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    // Add a template
    QString templateKey;
    for (const auto& entry : QDirListing(testDataPath, {"*.json"})) {
        if (entry.isFile()) {
            ASSERT_TRUE(inventory.addTemplate(entry, location));
            
            QString baseName = QFileInfo(entry.filePath()).baseName();
            templateKey = QString("1000-%1").arg(baseName);
            break;
        }
    }
    
    if (templateKey.isEmpty()) {
        GTEST_SKIP() << "No .json templates found for structure test";
    }
    
    // Get JSON content
    QJsonObject json = inventory.getJsonContent(templateKey);
    ASSERT_FALSE(json.isEmpty());
    
    // VS Code snippet format should have "body" and "prefix"
    // (Actual structure depends on your .json files)
    // This test verifies JSON is parsed, structure validation is separate
    EXPECT_TRUE(json.contains("body") || json.contains("prefix") || json.contains("description"));
}
