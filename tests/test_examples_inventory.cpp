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

#include "resourceInventory/ExamplesInventory.hpp"
#include "resourceInventory/resourceItem.hpp"

using namespace resourceInventory;

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
    
    // Scan BasicShapes folder
    QString shapesPath = testDataPath + "/BasicShapes";
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            ASSERT_TRUE(inventory.addExample(entry, "installation", "BasicShapes"));
        }
    }
    
    EXPECT_EQ(inventory.count(), 1); // cube.scad
    
    // Verify hierarchical key format
    QString key = "installation-BasicShapes-cube";
    EXPECT_TRUE(inventory.contains(key));
    
    QVariant var = inventory.get(key);
    ASSERT_TRUE(var.canConvert<ResourceScript>());
    
    ResourceScript script = var.value<ResourceScript>();
    EXPECT_EQ(script.category(), "BasicShapes");
    EXPECT_EQ(script.displayName(), "cube");
}

TEST_F(ExamplesInventoryTest, GetByPathFallback) {
    ExamplesInventory inventory;
    
    QString shapesPath = testDataPath + "/BasicShapes";
    QString cubePath;
    
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            cubePath = entry.filePath();
            inventory.addExample(entry, "installation", "BasicShapes");
        }
    }
    
    // Path-based lookup (slower but works)
    QVariant var = inventory.getByPath(cubePath);
    ASSERT_TRUE(var.canConvert<ResourceScript>());
    
    ResourceScript script = var.value<ResourceScript>();
    EXPECT_EQ(script.displayName(), "cube");
}

TEST_F(ExamplesInventoryTest, DifferentTiersSameFile) {
    ExamplesInventory inventory;
    
    QString shapesPath = testDataPath + "/BasicShapes";
    
    // Add same file twice with different tiers (simulating installation + user copy)
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            ASSERT_TRUE(inventory.addExample(entry, "installation", "BasicShapes"));
            ASSERT_TRUE(inventory.addExample(entry, "user", "MyShapes")); // Different tier+category = different key
        }
    }
    
    EXPECT_EQ(inventory.count(), 2);
    EXPECT_TRUE(inventory.contains("installation-BasicShapes-cube"));
    EXPECT_TRUE(inventory.contains("user-MyShapes-cube"));
}

TEST_F(ExamplesInventoryTest, DuplicateKeyRejected) {
    ExamplesInventory inventory;
    
    QString shapesPath = testDataPath + "/BasicShapes";
    
    int addCount = 0;
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            if (inventory.addExample(entry, "installation", "BasicShapes")) addCount++;
            if (inventory.addExample(entry, "installation", "BasicShapes")) addCount++; // Same key = rejected
        }
    }
    
    EXPECT_EQ(addCount, 1);
    EXPECT_EQ(inventory.count(), 1);
}

TEST_F(ExamplesInventoryTest, GetAll) {
    ExamplesInventory inventory;
    
    QString shapesPath = testDataPath + "/BasicShapes";
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            inventory.addExample(entry, "installation", "BasicShapes");
        }
    }
    
    QList<QVariant> all = inventory.getAll();
    EXPECT_EQ(all.size(), 1);
    
    for (const QVariant& var : all) {
        EXPECT_TRUE(var.canConvert<ResourceScript>());
    }
}

TEST_F(ExamplesInventoryTest, GetByCategory) {
    ExamplesInventory inventory;
    
    // Add with different categories
    QString shapesPath = testDataPath + "/BasicShapes";
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            inventory.addExample(entry, "installation", "BasicShapes");
            inventory.addExample(entry, "user", "Advanced"); // Different tier allows same file
        }
    }
    
    QList<QVariant> basics = inventory.getByCategory("BasicShapes");
    EXPECT_EQ(basics.size(), 1);
    
    QList<QVariant> advanced = inventory.getByCategory("Advanced");
    EXPECT_EQ(advanced.size(), 1);
}

TEST_F(ExamplesInventoryTest, GetCategories) {
    ExamplesInventory inventory;
    
    QString shapesPath = testDataPath + "/BasicShapes";
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            inventory.addExample(entry, "installation", "Basics");
            inventory.addExample(entry, "user", "Parametric");
            inventory.addExample(entry, "machine", "Advanced");
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
    
    // Scan for BasicShapes folder
    for (const auto& entry : QDirListing(testDataPath)) {
        if (entry.isDir() && entry.fileName() == "BasicShapes") {
            ASSERT_TRUE(inventory.addFolder(entry, "installation", "BasicShapes"));
        }
    }
    
    EXPECT_EQ(inventory.count(), 1); // cube.scad
    EXPECT_TRUE(inventory.contains("installation-BasicShapes-cube"));
}

TEST_F(ExamplesInventoryTest, Clear) {
    ExamplesInventory inventory;
    
    QString shapesPath = testDataPath + "/BasicShapes";
    for (const auto& entry : QDirListing(shapesPath, {"*.scad"})) {
        if (entry.isFile()) {
            inventory.addExample(entry, "installation", "BasicShapes");
        }
    }
    
    EXPECT_EQ(inventory.count(), 1);
    
    inventory.clear();
    EXPECT_EQ(inventory.count(), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
