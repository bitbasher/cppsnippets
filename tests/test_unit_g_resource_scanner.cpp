/**
 * @file test_resource_scanner.cpp
 * @brief Phase 3 tests for ResourceScanner class
 * 
 * Tests top-level orchestration: consuming ResourceLocations,
 * delegating to ExamplesInventory, and populating QStandardItemModel.
 */

#include <gtest/gtest.h>
#include <QDir>
#include <QStandardItemModel>

#include "resourceScanning/ResourceScanner.hpp"
#include "platformInfo/ResourceLocation.hpp"

using namespace resourceScanning;
using namespace platformInfo;
using namespace resourceMetadata;

class ResourceScannerTest : public ::testing::Test {
protected:
    QString testDataPath;
    
    void SetUp() override {
        // Use testFileStructure for repeatable tests
        QDir current = QDir::current();
        testDataPath = current.absolutePath() + "/testFileStructure";
        ASSERT_TRUE(QDir(testDataPath).exists()) << "testFileStructure not found";
    }
};

// ============================================================================
// Phase 3: ResourceScanner Tests
// ============================================================================

TEST_F(ResourceScannerTest, ScanSingleLocation) {
    ResourceScanner scanner;
    QStandardItemModel model;
    
    // Create location for Installation tier
    QString instPath = testDataPath + "/inst/OpenSCAD";
    ResourceLocation instLoc(instPath, ResourceTier::Installation);
    
    QList<ResourceLocation> locations;
    locations.append(instLoc);
    
    ASSERT_TRUE(scanner.scanToModel(&model, locations));
    
    // Should have found examples from testFileStructure
    EXPECT_GT(scanner.examplesCount(), 0) << "Should find examples in testFileStructure";
    EXPECT_GT(model.rowCount(), 0) << "Model should be populated";
}

TEST_F(ResourceScannerTest, ScanMultipleTiers) {
    ResourceScanner scanner;
    QStandardItemModel model;
    
    // Create locations for Installation and Personal tiers
    QString instPath = testDataPath + "/inst/OpenSCAD";
    QString persPath = testDataPath + "/pers/OpenSCAD";
    
    QList<ResourceLocation> locations;
    locations.append(ResourceLocation(instPath, ResourceTier::Installation));
    locations.append(ResourceLocation(persPath, ResourceTier::User));
    
    ASSERT_TRUE(scanner.scanToModel(&model, locations));
    
    // Should aggregate examples and templates from both tiers
    int totalExamples = scanner.examplesCount();
    int totalTemplates = scanner.templatesCount();
    EXPECT_GT(totalExamples, 0);
    EXPECT_GT(totalTemplates, 0);
    // Model contains both examples and templates
    EXPECT_EQ(model.rowCount(), totalExamples + totalTemplates);
}

TEST_F(ResourceScannerTest, LocationWithoutExamples) {
    ResourceScanner scanner;
    QStandardItemModel model;
    
    // Create location that doesn't have examples/ folder
    QString emptyPath = testDataPath + "/nonexistent";
    ResourceLocation emptyLoc(emptyPath, ResourceTier::Machine);
    
    QList<ResourceLocation> locations;
    locations.append(emptyLoc);
    
    // Should succeed but find nothing
    ASSERT_TRUE(scanner.scanToModel(&model, locations));
    EXPECT_EQ(scanner.examplesCount(), 0);
    EXPECT_EQ(model.rowCount(), 0);
}

TEST_F(ResourceScannerTest, ModelPopulation) {
    ResourceScanner scanner;
    QStandardItemModel model;
    
    QString instPath = testDataPath + "/inst/OpenSCAD";
    QList<ResourceLocation> locations;
    locations.append(ResourceLocation(instPath, ResourceTier::Installation));
    
    ASSERT_TRUE(scanner.scanToModel(&model, locations));
    
    // Check model structure - now has 5 columns: Type, Name, Category, Tier, Path
    EXPECT_EQ(model.columnCount(), 5);
    
    // Check first row has data
    if (model.rowCount() > 0) {
        EXPECT_FALSE(model.item(0, 0)->text().isEmpty()) << "Type should be populated";
        EXPECT_FALSE(model.item(0, 1)->text().isEmpty()) << "Name should be populated";
        EXPECT_FALSE(model.item(0, 2)->text().isEmpty()) << "Category should be populated";
        EXPECT_FALSE(model.item(0, 3)->text().isEmpty()) << "Tier should be populated";
        EXPECT_FALSE(model.item(0, 4)->text().isEmpty()) << "Path should be populated";
    }
}

TEST_F(ResourceScannerTest, InventoryAccessible) {
    ResourceScanner scanner;
    QStandardItemModel model;
    
    QString instPath = testDataPath + "/inst/OpenSCAD";
    QList<ResourceLocation> locations;
    locations.append(ResourceLocation(instPath, ResourceTier::Installation));
    
    ASSERT_TRUE(scanner.scanToModel(&model, locations));
    
    // Should be able to access both inventories directly
    const auto& examplesInv = scanner.examplesInventory();
    const auto& templatesInv = scanner.templatesInventory();
    EXPECT_GT(examplesInv.count(), 0);
    EXPECT_GT(templatesInv.count(), 0);
    
    // Verify inventories match model
    EXPECT_EQ(examplesInv.count() + templatesInv.count(), model.rowCount());
}

TEST_F(ResourceScannerTest, ClearBetweenScans) {
    ResourceScanner scanner;
    QStandardItemModel model;
    
    QString instPath = testDataPath + "/inst/OpenSCAD";
    QList<ResourceLocation> locations;
    locations.append(ResourceLocation(instPath, ResourceTier::Installation));
    
    // First scan
    ASSERT_TRUE(scanner.scanToModel(&model, locations));
    int firstCount = scanner.examplesCount();
    
    // Second scan should clear and repopulate
    ASSERT_TRUE(scanner.scanToModel(&model, locations));
    int secondCount = scanner.examplesCount();
    
    EXPECT_EQ(firstCount, secondCount) << "Count should be consistent across rescans";
}

TEST_F(ResourceScannerTest, NullModelHandling) {
    ResourceScanner scanner;
    QList<ResourceLocation> locations;
    
    // Should handle null model gracefully
    EXPECT_FALSE(scanner.scanToModel(nullptr, locations));
}
