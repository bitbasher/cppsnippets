/**
 * @file test_resource_metadata_comprehensive.cpp
 * @brief Comprehensive unit tests for resourceMetadata namespace
 * 
 * Tests all static const members, QMap exports, and basic functionality
 * to verify linker issues and DLL exports work correctly.
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include <QMap>
#include "resourceMetadata/ResourceTier.hpp"
#include "resourceMetadata/ResourceTypeInfo.hpp"
#include "resourceMetadata/ResourceAccess.hpp"

using namespace resourceMetadata;

// ============================================================================
// ResourceTier Enum Tests
// ============================================================================

class ResourceTierTest : public ::testing::Test {};

TEST_F(ResourceTierTest, EnumValuesExist) {
    ResourceTier installation = ResourceTier::Installation;
    ResourceTier machine = ResourceTier::Machine;
    ResourceTier user = ResourceTier::User;
    
    // Basic assignment should work
    EXPECT_TRUE(true);
}

TEST_F(ResourceTierTest, EnumValuesAreDistinct) {
    EXPECT_NE(ResourceTier::Installation, ResourceTier::Machine);
    EXPECT_NE(ResourceTier::Machine, ResourceTier::User);
    EXPECT_NE(ResourceTier::User, ResourceTier::Installation);
}

TEST_F(ResourceTierTest, TierDisplayNameFunctionWorks) {
    // Verify the tierDisplayName function can be called
    EXPECT_NO_THROW({
        QString name = tierDisplayName(ResourceTier::Installation);
        EXPECT_FALSE(name.isEmpty());
    });
}

TEST_F(ResourceTierTest, TierDisplayNameReturnsAllThreeTiers) {
    EXPECT_FALSE(tierDisplayName(ResourceTier::Installation).isEmpty());
    EXPECT_FALSE(tierDisplayName(ResourceTier::Machine).isEmpty());
    EXPECT_FALSE(tierDisplayName(ResourceTier::User).isEmpty());
}

TEST_F(ResourceTierTest, TierDisplayNameForInstallation) {
    QString name = tierDisplayName(ResourceTier::Installation);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_EQ(name, QString("Installation"));
}

TEST_F(ResourceTierTest, TierDisplayNameForMachine) {
    QString name = tierDisplayName(ResourceTier::Machine);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_EQ(name, QString("Machine"));
}

TEST_F(ResourceTierTest, TierDisplayNameForUser) {
    QString name = tierDisplayName(ResourceTier::User);
    EXPECT_FALSE(name.isEmpty());
    EXPECT_EQ(name, QString("User"));
}

TEST_F(ResourceTierTest, DisplayNamesAreNotEmpty) {
    EXPECT_FALSE(tierDisplayName(ResourceTier::Installation).isEmpty());
    EXPECT_FALSE(tierDisplayName(ResourceTier::Machine).isEmpty());
    EXPECT_FALSE(tierDisplayName(ResourceTier::User).isEmpty());
    
    EXPECT_GT(tierDisplayName(ResourceTier::Installation).length(), 0);
    EXPECT_GT(tierDisplayName(ResourceTier::Machine).length(), 0);
    EXPECT_GT(tierDisplayName(ResourceTier::User).length(), 0);
}

// ============================================================================
// ResourceTypeInfo Static Lists Tests
// ============================================================================

class ResourceTypeInfoStaticListsTest : public ::testing::Test {};

TEST_F(ResourceTypeInfoStaticListsTest, TopLevelListExists) {
    EXPECT_NO_THROW({
        auto size = s_topLevel.size();
        EXPECT_GT(size, 0);
    });
}

TEST_F(ResourceTypeInfoStaticListsTest, TopLevelListIsNotEmpty) {
    EXPECT_FALSE(s_topLevel.isEmpty());
    EXPECT_GT(s_topLevel.size(), 0);
}

TEST_F(ResourceTypeInfoStaticListsTest, TopLevelListContainsTemplates) {
    EXPECT_TRUE(s_topLevel.contains(ResourceType::Templates));
}

TEST_F(ResourceTypeInfoStaticListsTest, TopLevelListContainsFonts) {
    EXPECT_TRUE(s_topLevel.contains(ResourceType::Fonts));
}

TEST_F(ResourceTypeInfoStaticListsTest, TopLevelListContainsLibraries) {
    EXPECT_TRUE(s_topLevel.contains(ResourceType::Libraries));
}

TEST_F(ResourceTypeInfoStaticListsTest, TopLevelListContainsExamples) {
    EXPECT_TRUE(s_topLevel.contains(ResourceType::Examples));
}

TEST_F(ResourceTypeInfoStaticListsTest, NonContainerListExists) {
    EXPECT_FALSE(s_nonContainer.isEmpty());
    EXPECT_GT(s_nonContainer.size(), 0);
}

TEST_F(ResourceTypeInfoStaticListsTest, NonContainerContainsFonts) {
    EXPECT_TRUE(s_nonContainer.contains(ResourceType::Fonts));
}

TEST_F(ResourceTypeInfoStaticListsTest, NonContainerContainsTemplates) {
    EXPECT_TRUE(s_nonContainer.contains(ResourceType::Templates));
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersExists) {
    // Test the newly added s_allResourceFolders
    EXPECT_NO_THROW({
        auto size = s_allResourceFolders.size();
        EXPECT_GT(size, 0);
    });
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersIsNotEmpty) {
    EXPECT_FALSE(s_allResourceFolders.isEmpty());
    EXPECT_GT(s_allResourceFolders.size(), 5); // At least 5 folders
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersContainsTemplates) {
    EXPECT_TRUE(s_allResourceFolders.contains("templates"));
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersContainsFonts) {
    EXPECT_TRUE(s_allResourceFolders.contains("fonts"));
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersContainsColorSchemes) {
    EXPECT_TRUE(s_allResourceFolders.contains("color-schemes"));
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersContainsLibraries) {
    EXPECT_TRUE(s_allResourceFolders.contains("libraries"));
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersContainsExamples) {
    EXPECT_TRUE(s_allResourceFolders.contains("examples"));
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersContainsShaders) {
    EXPECT_TRUE(s_allResourceFolders.contains("shaders"));
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersContainsLocale) {
    EXPECT_TRUE(s_allResourceFolders.contains("locale"));
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersDoesNotContainNewResources) {
    // newresources is a location, not a resource folder
    EXPECT_FALSE(s_allResourceFolders.contains("newresources"));
}

TEST_F(ResourceTypeInfoStaticListsTest, AllResourceFoldersEntriesAreNotEmpty) {
    for (const QString& folder : s_allResourceFolders) {
        EXPECT_FALSE(folder.isEmpty()) << "Found empty folder name in s_allResourceFolders";
        EXPECT_GT(folder.length(), 0);
    }
}

// ============================================================================
// ResourceTypeInfo s_resourceTypes Map Tests
// ============================================================================

class ResourceTypeInfoMapTest : public ::testing::Test {};

TEST_F(ResourceTypeInfoMapTest, ResourceTypesMapIsAccessible) {
    EXPECT_NO_THROW({
        auto size = ResourceTypeInfo::s_resourceTypes.size();
        EXPECT_GT(size, 0);
    });
}

TEST_F(ResourceTypeInfoMapTest, ResourceTypesMapIsNotEmpty) {
    EXPECT_FALSE(ResourceTypeInfo::s_resourceTypes.isEmpty());
    EXPECT_GT(ResourceTypeInfo::s_resourceTypes.size(), 0);
}

TEST_F(ResourceTypeInfoMapTest, ResourceTypesMapContainsTemplates) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(ResourceType::Templates);
    EXPECT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd());
}

TEST_F(ResourceTypeInfoMapTest, ResourceTypesMapContainsFonts) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(ResourceType::Fonts);
    EXPECT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd());
}

TEST_F(ResourceTypeInfoMapTest, ResourceTypesMapContainsLibraries) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(ResourceType::Libraries);
    EXPECT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd());
}

TEST_F(ResourceTypeInfoMapTest, ResourceTypesMapContainsExamples) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(ResourceType::Examples);
    EXPECT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd());
}

TEST_F(ResourceTypeInfoMapTest, TemplatesInfoIsCorrect) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(ResourceType::Templates);
    ASSERT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd());
    
    EXPECT_EQ(it.value().getType(), ResourceType::Templates);
    EXPECT_EQ(it.value().getSubDir(), QString("templates"));
    EXPECT_FALSE(it.value().getDescription().isEmpty());
}

TEST_F(ResourceTypeInfoMapTest, FontsInfoIsCorrect) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(ResourceType::Fonts);
    ASSERT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd());
    
    EXPECT_EQ(it.value().getType(), ResourceType::Fonts);
    EXPECT_EQ(it.value().getSubDir(), QString("fonts"));
    EXPECT_FALSE(it.value().getDescription().isEmpty());
}

TEST_F(ResourceTypeInfoMapTest, LibrariesInfoIsCorrect) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(ResourceType::Libraries);
    ASSERT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd());
    
    EXPECT_EQ(it.value().getType(), ResourceType::Libraries);
    EXPECT_EQ(it.value().getSubDir(), QString("libraries"));
    EXPECT_FALSE(it.value().getDescription().isEmpty());
}

TEST_F(ResourceTypeInfoMapTest, ExamplesInfoIsCorrect) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(ResourceType::Examples);
    ASSERT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd());
    
    EXPECT_EQ(it.value().getType(), ResourceType::Examples);
    EXPECT_EQ(it.value().getSubDir(), QString("examples"));
    EXPECT_FALSE(it.value().getDescription().isEmpty());
}

TEST_F(ResourceTypeInfoMapTest, TemplatesHasExtensions) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(ResourceType::Templates);
    ASSERT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd());
    
    QStringList exts = it.value().getPrimaryExtensions();
    EXPECT_FALSE(exts.isEmpty());
    EXPECT_TRUE(exts.contains(".json"));
}

TEST_F(ResourceTypeInfoMapTest, LibrariesHasExtensions) {
    auto it = ResourceTypeInfo::s_resourceTypes.constFind(ResourceType::Libraries);
    ASSERT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd());
    
    QStringList exts = it.value().getPrimaryExtensions();
    EXPECT_FALSE(exts.isEmpty());
    EXPECT_TRUE(exts.contains(".scad"));
}

TEST_F(ResourceTypeInfoMapTest, AllResourceTypesMethod) {
    QList<ResourceTypeInfo> allTypes = ResourceTypeInfo::allResourceTypes();
    EXPECT_FALSE(allTypes.isEmpty());
    EXPECT_GT(allTypes.size(), 0);
    EXPECT_EQ(allTypes.size(), ResourceTypeInfo::s_resourceTypes.size());
}

// ============================================================================
// Integration Tests
// ============================================================================

class ResourceMetadataIntegrationTest : public ::testing::Test {};

TEST_F(ResourceMetadataIntegrationTest, AllResourceFoldersMatchResourceTypes) {
    // Verify s_allResourceFolders entries exist in s_resourceTypes
    int matchCount = 0;
    
    for (const QString& folderName : s_allResourceFolders) {
        bool found = false;
        
        for (auto it = ResourceTypeInfo::s_resourceTypes.constBegin();
             it != ResourceTypeInfo::s_resourceTypes.constEnd(); ++it) {
            if (it.value().getSubDir() == folderName) {
                found = true;
                matchCount++;
                break;
            }
        }
        
        EXPECT_TRUE(found) << "Folder '" << folderName.toStdString() 
                          << "' not found in s_resourceTypes subdirectories";
    }
    
    EXPECT_GT(matchCount, 0);
    EXPECT_EQ(matchCount, s_allResourceFolders.size());
}

TEST_F(ResourceMetadataIntegrationTest, TopLevelTypesHaveValidInfo) {
    for (ResourceType type : s_topLevel) {
        auto it = ResourceTypeInfo::s_resourceTypes.constFind(type);
        EXPECT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd()) 
            << "Top-level type not found in s_resourceTypes";
        
        if (it != ResourceTypeInfo::s_resourceTypes.constEnd()) {
            EXPECT_FALSE(it.value().getSubDir().isEmpty()) 
                << "Top-level type has empty subdirectory";
            EXPECT_FALSE(it.value().getDescription().isEmpty())
                << "Top-level type has empty description";
        }
    }
}

TEST_F(ResourceMetadataIntegrationTest, NonContainerTypesHaveValidInfo) {
    for (ResourceType type : s_nonContainer) {
        auto it = ResourceTypeInfo::s_resourceTypes.constFind(type);
        EXPECT_NE(it, ResourceTypeInfo::s_resourceTypes.constEnd())
            << "Non-container type not found in s_resourceTypes";
        
        if (it != ResourceTypeInfo::s_resourceTypes.constEnd()) {
            EXPECT_FALSE(it.value().getSubDir().isEmpty())
                << "Non-container type has empty subdirectory";
        }
    }
}

TEST_F(ResourceMetadataIntegrationTest, TierDisplayNamesComplete) {
    // All three active tiers should have display names
    EXPECT_FALSE(tierDisplayName(ResourceTier::Installation).isEmpty());
    EXPECT_FALSE(tierDisplayName(ResourceTier::Machine).isEmpty());
    EXPECT_FALSE(tierDisplayName(ResourceTier::User).isEmpty());
}
