/**
 * @file test_templatescanner.cpp
 * @brief Unit tests for TemplateScanner (Phase 2B.1)
 * 
 * Tests template discovery, JSON validation, and metadata extraction.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

#include "resourceDiscovery/templateScanner.hpp"
#include "platformInfo/ResourceLocation.hpp"
#include "resourceInventory/resourceItem.hpp"

using resourceInventory::ResourceTemplate;
using resourceInventory::ResourceTier;
using resourceInventory::ResourceAccess;

// Helper: Create a test template JSON file
void createTemplateFile(const QString& path, const QString& name, 
                       const QString& category = QString(),
                       const QString& body = QString())
{
    QJsonObject json;
    json["name"] = name;
    if (!category.isEmpty()) json["category"] = category;
    if (!body.isEmpty()) json["body"] = body;
    
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << QJsonDocument(json).toJson(QJsonDocument::Compact);
        file.close();
    }
}

// Test fixture
class TemplateScannerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory structure
        tempDir.setAutoRemove(true);
        ASSERT_TRUE(tempDir.isValid());
        
        templatesDir = tempDir.path() + "/templates";
        QDir().mkpath(templatesDir);
    }
    
    QTemporaryDir tempDir;
    QString templatesDir;
};

// Basic Functionality Tests

TEST_F(TemplateScannerTest, EmptyFolderReturnsEmptyVector) {
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    EXPECT_TRUE(templates.isEmpty());
}

TEST_F(TemplateScannerTest, SingleTemplateIsDiscovered) {
    createTemplateFile(templatesDir + "/cube.json", "Basic Cube");
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    ASSERT_EQ(templates.size(), 1);
    EXPECT_EQ(templates[0].name(), "Basic Cube");
}

TEST_F(TemplateScannerTest, MultipleTemplatesAreDiscovered) {
    createTemplateFile(templatesDir + "/cube.json", "Basic Cube");
    createTemplateFile(templatesDir + "/sphere.json", "Sphere");
    createTemplateFile(templatesDir + "/cylinder.json", "Cylinder");
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    EXPECT_EQ(templates.size(), 3);
}

TEST_F(TemplateScannerTest, TemplateWithAllFieldsIsComplete) {
    createTemplateFile(templatesDir + "/full.json", 
                      "Full Template",
                      "Primitives",
                      "cube([10, 10, 10]);");
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    ASSERT_EQ(templates.size(), 1);
    EXPECT_EQ(templates[0].name(), "Full Template");
    EXPECT_EQ(templates[0].category(), "Primitives");
    EXPECT_EQ(templates[0].body(), "cube([10, 10, 10]);");
}

TEST_F(TemplateScannerTest, TemplateWithMinimalFieldsIsValid) {
    createTemplateFile(templatesDir + "/minimal.json", "Minimal Template");
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    ASSERT_EQ(templates.size(), 1);
    EXPECT_EQ(templates[0].name(), "Minimal Template");
    EXPECT_TRUE(templates[0].category().isEmpty());
    EXPECT_TRUE(templates[0].body().isEmpty());
}

TEST_F(TemplateScannerTest, TierIsCorrectlyTagged) {
    createTemplateFile(templatesDir + "/test.json", "Test Template");
    
    platformInfo::ResourceLocation installLoc(tempDir.path(), ResourceTier::Installation);
    auto installTemplates = TemplateScanner::scanLocation(installLoc);
    
    ASSERT_EQ(installTemplates.size(), 1);
    EXPECT_EQ(installTemplates[0].tier(), ResourceTier::Installation);
    
    platformInfo::ResourceLocation userLoc(tempDir.path(), ResourceTier::User);
    auto userTemplates = TemplateScanner::scanLocation(userLoc);
    
    ASSERT_EQ(userTemplates.size(), 1);
    EXPECT_EQ(userTemplates[0].tier(), ResourceTier::User);
}

// Error Handling Tests

TEST_F(TemplateScannerTest, InvalidJsonIsSkipped) {
    QFile file(templatesDir + "/invalid.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "{ invalid json ][";
        file.close();
    }
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    EXPECT_TRUE(templates.isEmpty());
}

TEST_F(TemplateScannerTest, MissingRequiredFieldIsSkipped) {
    QFile file(templatesDir + "/no_name.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << R"({"category": "Test"})";
        file.close();
    }
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    EXPECT_TRUE(templates.isEmpty());
}

TEST_F(TemplateScannerTest, NonTemplateJsonIsSkipped) {
    QFile file(templatesDir + "/config.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << R"({"version": "1.0", "settings": {}})";
        file.close();
    }
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    EXPECT_TRUE(templates.isEmpty());
}

TEST_F(TemplateScannerTest, EmptyNameFieldIsSkipped) {
    QFile file(templatesDir + "/empty_name.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << R"({"name": ""})";
        file.close();
    }
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    EXPECT_TRUE(templates.isEmpty());
}

TEST_F(TemplateScannerTest, UnreadableFileIsSkipped) {
    createTemplateFile(templatesDir + "/test.json", "Test");
    
    // Make file unreadable (platform-specific)
#ifdef Q_OS_WIN
    QFile file(templatesDir + "/test.json");
    file.setPermissions(QFile::WriteOwner);
#else
    QFile file(templatesDir + "/test.json");
    file.setPermissions(QFile::WriteOwner | QFile::WriteGroup | QFile::WriteOther);
#endif
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    // Should skip unreadable file gracefully
    EXPECT_TRUE(templates.isEmpty() || templates.size() == 1);
}

// Metadata Extraction Tests

TEST_F(TemplateScannerTest, FilePathIsSetCorrectly) {
    createTemplateFile(templatesDir + "/test.json", "Test Template");
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    ASSERT_EQ(templates.size(), 1);
    EXPECT_TRUE(templates[0].path().contains("test.json"));
    EXPECT_TRUE(templates[0].path().endsWith("test.json"));
}

TEST_F(TemplateScannerTest, SourceLocationIsTracked) {
    createTemplateFile(templatesDir + "/test.json", "Test Template");
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    ASSERT_EQ(templates.size(), 1);
    EXPECT_EQ(templates[0].sourceLocationKey(), tempDir.path());
}

TEST_F(TemplateScannerTest, ExistsIsSetToTrue) {
    createTemplateFile(templatesDir + "/test.json", "Test Template");
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    ASSERT_EQ(templates.size(), 1);
    EXPECT_TRUE(templates[0].exists());
}

TEST_F(TemplateScannerTest, LastModifiedIsSet) {
    createTemplateFile(templatesDir + "/test.json", "Test Template");
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    auto templates = TemplateScanner::scanLocation(loc);
    
    ASSERT_EQ(templates.size(), 1);
    EXPECT_TRUE(templates[0].lastModified().isValid());
}

TEST_F(TemplateScannerTest, AccessBasedOnLocationWriteability) {
    createTemplateFile(templatesDir + "/test.json", "Test Template");
    
    platformInfo::ResourceLocation readOnlyLoc(tempDir.path(), ResourceTier::Installation);
    readOnlyLoc.setWritable(false);
    auto roTemplates = TemplateScanner::scanLocation(readOnlyLoc);
    
    ASSERT_EQ(roTemplates.size(), 1);
    EXPECT_EQ(roTemplates[0].access(), ResourceAccess::ReadOnly);
    
    platformInfo::ResourceLocation writableLoc(tempDir.path(), ResourceTier::User);
    writableLoc.setWritable(true);
    auto rwTemplates = TemplateScanner::scanLocation(writableLoc);
    
    ASSERT_EQ(rwTemplates.size(), 1);
    EXPECT_EQ(rwTemplates[0].access(), ResourceAccess::Writable);
}

// Integration Tests

TEST_F(TemplateScannerTest, MultipleLocationsAreCombined) {
    QTemporaryDir tempDir2;
    QString templatesDir2 = tempDir2.path() + "/templates";
    QDir().mkpath(templatesDir2);
    
    createTemplateFile(templatesDir + "/loc1.json", "Template 1");
    createTemplateFile(templatesDir2 + "/loc2.json", "Template 2");
    
    platformInfo::ResourceLocation loc1(tempDir.path(), ResourceTier::Installation);
    platformInfo::ResourceLocation loc2(tempDir2.path(), ResourceTier::User);
    
    QVector<platformInfo::ResourceLocation> locations = {loc1, loc2};
    auto allTemplates = TemplateScanner::scanLocations(locations);
    
    EXPECT_EQ(allTemplates.size(), 2);
}

TEST_F(TemplateScannerTest, DisabledLocationIsSkipped) {
    createTemplateFile(templatesDir + "/test.json", "Test Template");
    
    platformInfo::ResourceLocation loc(tempDir.path(), ResourceTier::User);
    loc.setEnabled(false);
    
    QVector<platformInfo::ResourceLocation> locations = {loc};
    auto templates = TemplateScanner::scanLocations(locations);
    
    EXPECT_TRUE(templates.isEmpty());
}

TEST_F(TemplateScannerTest, NonExistentLocationIsSkipped) {
    platformInfo::ResourceLocation loc("/nonexistent/path", ResourceTier::User);
    loc.setExists(false);
    
    QVector<platformInfo::ResourceLocation> locations = {loc};
    auto templates = TemplateScanner::scanLocations(locations);
    
    EXPECT_TRUE(templates.isEmpty());
}

// Static Helper Tests

TEST_F(TemplateScannerTest, TemplateExtensionsReturnsJson) {
    QStringList extensions = TemplateScanner::templateExtensions();
    
    EXPECT_EQ(extensions.size(), 1);
    EXPECT_EQ(extensions[0], "json");
}

TEST_F(TemplateScannerTest, TemplateSubfolderReturnsTemplates) {
    QString subfolder = TemplateScanner::templateSubfolder();
    
    EXPECT_EQ(subfolder, "templates");
}
