/**
 * @file test_qvariant_phase1.cpp
 * @brief Phase 1 tests for QVariant-based resource architecture
 * 
 * Tests Q_DECLARE_METATYPE registration and QVariant storage/retrieval.
 */

#include <gtest/gtest.h>
#include <QVariant>
#include <QString>
#include <QStringList>

#include "resourceInventory/resourceItem.hpp"

using namespace resourceInventory;

// ============================================================================
// Phase 1: QVariant Registration Tests
// ============================================================================

/**
 * @brief Test that ResourceItem can be stored in QVariant
 */
TEST(QVariantPhase1, ResourceItemCanBeStored) {
    // Create a ResourceItem
    ResourceItem item("/path/to/resource");
    item.setType(ResourceType::Examples);
    item.setTier(ResourceTier::User);
    item.setName("test-resource");
    item.setDisplayName("Test Resource");
    item.setCategory("Test Category");
    
    // Store in QVariant
    QVariant var = QVariant::fromValue(item);
    
    // Verify it can be converted back
    ASSERT_TRUE(var.canConvert<ResourceItem>());
    
    // Retrieve and verify
    ResourceItem retrieved = var.value<ResourceItem>();
    EXPECT_EQ(retrieved.path(), "/path/to/resource");
    EXPECT_EQ(retrieved.type(), ResourceType::Examples);
    EXPECT_EQ(retrieved.tier(), ResourceTier::User);
    EXPECT_EQ(retrieved.name(), "test-resource");
    EXPECT_EQ(retrieved.displayName(), "Test Resource");
    EXPECT_EQ(retrieved.category(), "Test Category");
}

/**
 * @brief Test that ResourceScript can be stored in QVariant
 */
TEST(QVariantPhase1, ResourceScriptCanBeStored) {
    // Create a ResourceScript with attachments
    ResourceScript script("/path/to/script.scad");
    script.setType(ResourceType::Examples);
    script.setName("customizer-all");
    script.setCategory("Parametric");
    script.addAttachment("/path/to/customizer-all.json");
    script.addAttachment("/path/to/customizer-all.png");
    
    // Store in QVariant
    QVariant var = QVariant::fromValue(script);
    
    // Verify it can be converted back
    ASSERT_TRUE(var.canConvert<ResourceScript>());
    
    // Retrieve and verify
    ResourceScript retrieved = var.value<ResourceScript>();
    EXPECT_EQ(retrieved.path(), "/path/to/script.scad");
    EXPECT_EQ(retrieved.name(), "customizer-all");
    EXPECT_EQ(retrieved.category(), "Parametric");
    EXPECT_TRUE(retrieved.hasAttachments());
    EXPECT_EQ(retrieved.attachments().size(), 2);
    EXPECT_EQ(retrieved.attachments()[0], "/path/to/customizer-all.json");
    EXPECT_EQ(retrieved.attachments()[1], "/path/to/customizer-all.png");
}

/**
 * @brief Test that ResourceTemplate can be stored in QVariant
 */
TEST(QVariantPhase1, ResourceTemplateCanBeStored) {
    // Create a ResourceTemplate
    ResourceTemplate tmpl("/path/to/template.json");
    tmpl.setType(ResourceType::Templates);
    tmpl.setName("cylinder-template");
    tmpl.setPrefix("cyl");
    tmpl.setBody("cylinder(h=${1:10}, r=${2:5});");
    tmpl.setFormat("text/scad.template");
    tmpl.setVersion("1");
    
    // Store in QVariant
    QVariant var = QVariant::fromValue(tmpl);
    
    // Verify it can be converted back
    ASSERT_TRUE(var.canConvert<ResourceTemplate>());
    
    // Retrieve and verify
    ResourceTemplate retrieved = var.value<ResourceTemplate>();
    EXPECT_EQ(retrieved.path(), "/path/to/template.json");
    EXPECT_EQ(retrieved.name(), "cylinder-template");
    EXPECT_EQ(retrieved.prefix(), "cyl");
    EXPECT_EQ(retrieved.body(), "cylinder(h=${1:10}, r=${2:5});");
    EXPECT_EQ(retrieved.format(), "text/scad.template");
    EXPECT_EQ(retrieved.version(), "1");
}

/**
 * @brief Test copy constructor preserves data
 */
TEST(QVariantPhase1, CopyConstructorPreservesData) {
    // Create original with attachments
    ResourceScript original("/path/to/script.scad");
    original.setType(ResourceType::Examples);
    original.setName("test-script");
    original.addAttachment("/path/to/attachment1.json");
    original.addAttachment("/path/to/attachment2.png");
    
    // Copy via copy constructor
    ResourceScript copy(original);
    
    // Verify copy has same data
    EXPECT_EQ(copy.path(), original.path());
    EXPECT_EQ(copy.name(), original.name());
    EXPECT_EQ(copy.type(), original.type());
    EXPECT_EQ(copy.hasAttachments(), original.hasAttachments());
    EXPECT_EQ(copy.attachments().size(), original.attachments().size());
    EXPECT_EQ(copy.attachments()[0], original.attachments()[0]);
    EXPECT_EQ(copy.attachments()[1], original.attachments()[1]);
}

/**
 * @brief Test assignment operator preserves data
 */
TEST(QVariantPhase1, AssignmentOperatorPreservesData) {
    // Create original
    ResourceScript original("/path/to/script.scad");
    original.setType(ResourceType::Examples);
    original.setName("test-script");
    original.addAttachment("/path/to/attachment.json");
    
    // Create different object
    ResourceScript other("/other/path.scad");
    other.setType(ResourceType::Tests);
    other.setName("other-script");
    
    // Assign
    other = original;
    
    // Verify assignment copied data
    EXPECT_EQ(other.path(), original.path());
    EXPECT_EQ(other.name(), original.name());
    EXPECT_EQ(other.type(), original.type());
    EXPECT_EQ(other.hasAttachments(), original.hasAttachments());
    EXPECT_EQ(other.attachments().size(), original.attachments().size());
}

/**
 * @brief Test QVariant preserves attachments (no object slicing)
 */
TEST(QVariantPhase1, NoObjectSlicing) {
    // Create ResourceScript with attachments
    ResourceScript script("/path/to/customizer-all.scad");
    script.setType(ResourceType::Examples);
    script.setName("customizer-all");
    script.setCategory("Parametric");
    script.addAttachment("/path/to/customizer-all.json");
    
    // Store in QVariant
    QVariant var = QVariant::fromValue(script);
    
    // This is the critical test - can we get the full ResourceScript back?
    ASSERT_TRUE(var.canConvert<ResourceScript>()) 
        << "QVariant should preserve ResourceScript type";
    
    ResourceScript retrieved = var.value<ResourceScript>();
    
    // Verify attachments preserved (this would fail with object slicing)
    EXPECT_TRUE(retrieved.hasAttachments()) 
        << "Attachments should be preserved through QVariant storage";
    EXPECT_EQ(retrieved.attachments().size(), 1);
    EXPECT_EQ(retrieved.attachments()[0], "/path/to/customizer-all.json");
}

/**
 * @brief Test QVariant in a list (like scanner results)
 */
TEST(QVariantPhase1, QVariantInList) {
    QList<QVariant> results;
    
    // Add ResourceItem
    ResourceItem item("/path/to/font.otf");
    item.setType(ResourceType::Fonts);
    item.setName("MyFont");
    results.append(QVariant::fromValue(item));
    
    // Add ResourceScript with attachments
    ResourceScript script("/path/to/example.scad");
    script.setType(ResourceType::Examples);
    script.setName("example");
    script.addAttachment("/path/to/example.json");
    results.append(QVariant::fromValue(script));
    
    // Add ResourceTemplate
    ResourceTemplate tmpl("/path/to/template.json");
    tmpl.setType(ResourceType::Templates);
    tmpl.setName("template");
    tmpl.setPrefix("tmp");
    results.append(QVariant::fromValue(tmpl));
    
    // Verify we can retrieve correct types
    ASSERT_EQ(results.size(), 3);
    
    // First item is ResourceItem
    EXPECT_TRUE(results[0].canConvert<ResourceItem>());
    ResourceItem retrievedItem = results[0].value<ResourceItem>();
    EXPECT_EQ(retrievedItem.name(), "MyFont");
    
    // Second item is ResourceScript with attachments
    EXPECT_TRUE(results[1].canConvert<ResourceScript>());
    ResourceScript retrievedScript = results[1].value<ResourceScript>();
    EXPECT_EQ(retrievedScript.name(), "example");
    EXPECT_TRUE(retrievedScript.hasAttachments());
    
    // Third item is ResourceTemplate
    EXPECT_TRUE(results[2].canConvert<ResourceTemplate>());
    ResourceTemplate retrievedTemplate = results[2].value<ResourceTemplate>();
    EXPECT_EQ(retrievedTemplate.name(), "template");
}

/**
 * @brief Test type discrimination in QVariant
 */
TEST(QVariantPhase1, TypeDiscrimination) {
    // Create different resource types
    ResourceItem item("/path/to/item");
    ResourceScript script("/path/to/script.scad");
    ResourceTemplate tmpl("/path/to/template.json");
    
    QVariant itemVar = QVariant::fromValue(item);
    QVariant scriptVar = QVariant::fromValue(script);
    QVariant tmplVar = QVariant::fromValue(tmpl);
    
    // Verify each can convert to its own type
    EXPECT_TRUE(itemVar.canConvert<ResourceItem>());
    EXPECT_TRUE(scriptVar.canConvert<ResourceScript>());
    EXPECT_TRUE(tmplVar.canConvert<ResourceTemplate>());
    
    // ResourceScript is-a ResourceItem (inheritance relationship)
    // but QVariant stores the actual type
    EXPECT_TRUE(scriptVar.canConvert<ResourceScript>());
    
    // ResourceTemplate is-a ResourceItem
    EXPECT_TRUE(tmplVar.canConvert<ResourceTemplate>());
}

/**
 * @brief Test QHash storage with QVariant (Phase 1 storage strategy)
 */
TEST(QVariantPhase1, QHashStorage) {
    QHash<QString, QVariant> inventory;
    
    // Add ResourceScript with path as key
    ResourceScript script("/examples/customizer-all.scad");
    script.setName("customizer-all");
    script.setCategory("Parametric");
    script.addAttachment("/examples/customizer-all.json");
    inventory.insert(script.path(), QVariant::fromValue(script));
    
    // Add ResourceTemplate with path as key
    ResourceTemplate tmpl("/templates/cylinder.json");
    tmpl.setName("cylinder");
    tmpl.setPrefix("cyl");
    inventory.insert(tmpl.path(), QVariant::fromValue(tmpl));
    
    // Retrieve by key (O(1) lookup)
    ASSERT_TRUE(inventory.contains("/examples/customizer-all.scad"));
    QVariant scriptVar = inventory.value("/examples/customizer-all.scad");
    ASSERT_TRUE(scriptVar.canConvert<ResourceScript>());
    
    ResourceScript retrieved = scriptVar.value<ResourceScript>();
    EXPECT_EQ(retrieved.name(), "customizer-all");
    EXPECT_TRUE(retrieved.hasAttachments());
    
    // Retrieve template
    ASSERT_TRUE(inventory.contains("/templates/cylinder.json"));
    QVariant tmplVar = inventory.value("/templates/cylinder.json");
    ASSERT_TRUE(tmplVar.canConvert<ResourceTemplate>());
    
    ResourceTemplate retrievedTmpl = tmplVar.value<ResourceTemplate>();
    EXPECT_EQ(retrievedTmpl.name(), "cylinder");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
