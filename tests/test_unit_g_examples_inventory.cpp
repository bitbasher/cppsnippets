/**
 * @file test_examples_inventory.cpp
 * @brief Phase 2 tests for ExamplesInventory class
 * 
 * Tests QHash-based storage with hierarchical keys (tier-category-name),
 * attachment scanning, and category management.
 * Uses testFileStructure for repeatable tests.
 */

#include <gtest/gtest.h>
#include <QDir>
#include <QDirListing>

#include "platformInfo/ResourceLocation.hpp"
#include "resourceInventory/ExamplesInventory.hpp"
#include "resourceInventory/resourceItem.hpp"

using namespace resourceInventory;
using namespace platformInfo;

class ExamplesInventoryTest : public ::testing::Test {
protected:
    QString testDataPath;
    
    void SetUp() override {
        // Use existing testFileStructure for repeatable tests
        // Tests run from build/ directory, testFileStructure is at workspace root
        QDir current = QDir::current();
        current.cdUp(); // Go to workspace root
        testDataPath = current.absolutePath() + "/testFileStructure/inst/OpenSCAD/examples";
        ASSERT_TRUE(QDir(testDataPath).exists()) << "testFileStructure not found at: " << testDataPath.toStdString();
    }
};

// ============================================================================
// Phase 2: ExamplesInventory Tests
// ============================================================================

TEST_F(ExamplesInventoryTest, AddExampleWithHierarchicalKey) {
    ExamplesInventory inventory;
    ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    // Scan BasicShapes folder
    QString shapesPath = testDataPath + "/BasicShapes";
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            ASSERT_TRUE(inventory.addExample(entry, location, "BasicShapes"));
        }
    }
    
    EXPECT_GT(inventory.count(), 0); // Should find at least cube.scad
    
    // Verify we can retrieve examples
    QList<QVariant> all = inventory.getAll();
    if (all.size() > 0) {
        QVariant var = all.first();
        ASSERT_TRUE(var.canConvert<ResourceScript>());
        
        ResourceScript script = var.value<ResourceScript>();
        EXPECT_EQ(script.category(), "BasicShapes");
        EXPECT_FALSE(script.displayName().isEmpty());
    }
}

TEST_F(ExamplesInventoryTest, DifferentTiersSameFile) {
    ExamplesInventory inventory;
    ResourceLocation installLocation(testDataPath, ResourceTier::Installation);
    ResourceLocation userLocation(testDataPath, ResourceTier::User);
    
    QString shapesPath = testDataPath + "/BasicShapes";
    
    // Add same file twice with different tiers (simulating installation + user copy)
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            ASSERT_TRUE(inventory.addExample(entry, installLocation, "BasicShapes"));
            ASSERT_TRUE(inventory.addExample(entry, userLocation, "MyShapes")); // Different tier+category = different key
        }
    }
    
    EXPECT_EQ(inventory.count(), 2);
}

TEST_F(ExamplesInventoryTest, DuplicateKeyRejected) {
    ExamplesInventory inventory;
    ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    QString shapesPath = testDataPath + "/BasicShapes";
    
    int addCount = 0;
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            if (inventory.addExample(entry, location, "BasicShapes")) addCount++;
            if (inventory.addExample(entry, location, "BasicShapes")) addCount++; // Same key = rejected
        }
    }
    
    EXPECT_EQ(addCount, 1);
    EXPECT_EQ(inventory.count(), 1);
}

TEST_F(ExamplesInventoryTest, GetAll) {
    ExamplesInventory inventory;
    ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    QString shapesPath = testDataPath + "/BasicShapes";
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            inventory.addExample(entry, location, "BasicShapes");
        }
    }
    
    QList<QVariant> all = inventory.getAll();
    EXPECT_GT(all.size(), 0);
    
    for (const QVariant& var : all) {
        EXPECT_TRUE(var.canConvert<ResourceScript>());
    }
}

TEST_F(ExamplesInventoryTest, GetByCategory) {
    ExamplesInventory inventory;
    ResourceLocation installLocation(testDataPath, ResourceTier::Installation);
    ResourceLocation userLocation(testDataPath, ResourceTier::User);
    
    // Add with different categories
    QString shapesPath = testDataPath + "/BasicShapes";
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            inventory.addExample(entry, installLocation, "BasicShapes");
            inventory.addExample(entry, userLocation, "Advanced"); // Different tier allows same file
        }
    }
    
    QList<QVariant> basics = inventory.getByCategory("BasicShapes");
    EXPECT_EQ(basics.size(), 1);
    
    QList<QVariant> advanced = inventory.getByCategory("Advanced");
    EXPECT_EQ(advanced.size(), 1);
}

TEST_F(ExamplesInventoryTest, GetCategories) {
    ExamplesInventory inventory;
    ResourceLocation installLocation(testDataPath, ResourceTier::Installation);
    ResourceLocation userLocation(testDataPath, ResourceTier::User);
    ResourceLocation machineLocation(testDataPath, ResourceTier::Machine);
    
    QString shapesPath = testDataPath + "/BasicShapes";
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            inventory.addExample(entry, installLocation, "Basics");
            inventory.addExample(entry, userLocation, "Parametric");
            inventory.addExample(entry, machineLocation, "Advanced");
        }
    }
    
    QStringList categories = inventory.getCategories();
    EXPECT_EQ(categories.size(), 3);
    EXPECT_TRUE(categories.contains("Basics"));
    EXPECT_TRUE(categories.contains("Parametric"));
    EXPECT_TRUE(categories.contains("Advanced"));
}

TEST_F(ExamplesInventoryTest, AddFolderWithScripts) {
    ExamplesInventory inventory;
    ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    // Scan for BasicShapes folder
    QString shapesPath = testDataPath + "/BasicShapes";
    int added = inventory.addFolder(shapesPath, location, "BasicShapes");
    
    EXPECT_GT(added, 0); // Should find at least cube.scad
    EXPECT_GT(inventory.count(), 0);
}

TEST_F(ExamplesInventoryTest, Clear) {
    ExamplesInventory inventory;
    ResourceLocation location(testDataPath, ResourceTier::Installation);
    
    QString shapesPath = testDataPath + "/BasicShapes";
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            inventory.addExample(entry, location, "BasicShapes");
        }
    }
    
    EXPECT_GT(inventory.count(), 0);
    
    inventory.clear();
    EXPECT_EQ(inventory.count(), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
