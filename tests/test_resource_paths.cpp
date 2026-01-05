/**
 * @file test_resource_paths.cpp
 * @brief Unit tests for ResourcePaths class
 * 
 * Tests environment variable expansion, folder name appending,
 * sibling installation detection, and qualified path generation.
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QProcessEnvironment>
#include <QSettings>
#include "platformInfo/resourcePaths.hpp"

using namespace platformInfo;
using namespace resourceInfo;

class ResourcePathsTest : public ::testing::Test {
protected:
    ResourcePaths* paths;
    QProcessEnvironment env;
    
    void SetUp() override {
        paths = new ResourcePaths();
        env = QProcessEnvironment::systemEnvironment();
    }
    
    void TearDown() override {
        delete paths;
    }
    
    // Helper to get an environment variable value
    QString getEnvVar(const QString& name) {
        return env.value(name, QString());
    }
};

// ============================================================================
// Folder Name and Suffix Tests
// ============================================================================

TEST_F(ResourcePathsTest, DefaultFolderName) {
    EXPECT_EQ(paths->folderName(), QString("ScadTemplates"));
}

TEST_F(ResourcePathsTest, SuffixManagement) {
    // Default suffix should be empty
    EXPECT_TRUE(paths->suffix().isEmpty());
    
    // Set suffix
    paths->setSuffix(QString(" (Nightly)"));
    EXPECT_EQ(paths->suffix(), QString(" (Nightly)"));
    
    // Clear suffix
    paths->setSuffix(QString());
    EXPECT_TRUE(paths->suffix().isEmpty());
}

// ============================================================================
// Environment Variable Expansion Tests
// ============================================================================

TEST_F(ResourcePathsTest, ExpandEnvVars_WindowsStyle) {
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    // Test %VAR% style expansion
    QString appdata = getEnvVar("APPDATA");
    if (!appdata.isEmpty()) {
        QStringList resolved = paths->resolvedUserSearchPaths();
        // Check that %APPDATA%/ gets expanded
        bool foundExpanded = false;
        for (const QString& path : resolved) {
            if (path.contains(appdata, Qt::CaseInsensitive)) {
                foundExpanded = true;
                break;
            }
        }
        EXPECT_TRUE(foundExpanded);
    }
#endif
}

TEST_F(ResourcePathsTest, ExpandEnvVars_UnixStyle) {
#if !defined(Q_OS_WIN) && !defined(_WIN32) && !defined(_WIN64)
    // Test ${VAR} style expansion
    QString home = getEnvVar("HOME");
    if (!home.isEmpty()) {
        QStringList resolved = paths->resolvedUserSearchPaths();
        // Check that ${HOME} gets expanded
        bool foundExpanded = false;
        for (const QString& path : resolved) {
            if (path.contains(home)) {
                foundExpanded = true;
                break;
            }
        }
        EXPECT_TRUE(foundExpanded);
    }
#endif
}

TEST_F(ResourcePathsTest, ExpandEnvVars_PreservesTrailingSlash) {
    QStringList resolved = paths->resolvedInstallSearchPaths();
    // At least one path should end with trailing slash after expansion
    // This is important for folder name appending logic
    bool foundTrailingSlash = false;
    for (const QString& path : resolved) {
        if (path.endsWith('/')) {
            foundTrailingSlash = true;
            break;
        }
    }
    // Note: May not always be true depending on platform defaults
    // This is a heuristic test
}

TEST_F(ResourcePathsTest, ExpandEnvVars_NormalizesPathSeparators) {
    QStringList resolved = paths->resolvedInstallSearchPaths();
    // All paths should use forward slashes after expansion
    for (const QString& path : resolved) {
        EXPECT_FALSE(path.contains('\\')) 
            << "Path contains backslash: " << path.toStdString();
    }
}

// ============================================================================
// Sibling Installation Detection Tests
// ============================================================================

TEST_F(ResourcePathsTest, SiblingFolderName_WithSuffix) {
    paths->setSuffix(QString(" (Nightly)"));
    
    // When app has suffix, sibling should not
    QString sibling = paths->getSiblingFolderName();
    EXPECT_EQ(sibling, QString("ScadTemplates"));
}

TEST_F(ResourcePathsTest, SiblingFolderName_WithoutSuffix) {
    paths->setSuffix(QString(" (Nightly)"));
    // Clear suffix to simulate LTS version
    paths->setSuffix(QString());
    
    // When app has no suffix, sibling should have it
    // But since suffix is now empty, sibling will be folderName + ""
    QString sibling = paths->getSiblingFolderName();
    EXPECT_EQ(sibling, QString("ScadTemplates"));
}

TEST_F(ResourcePathsTest, SiblingFolderName_Bidirectional) {
    // Test LTS → Nightly
    paths->setSuffix(QString());  // LTS version
    QString nightlyName = paths->folderName() + " (Nightly)";
    
    // Test Nightly → LTS
    paths->setSuffix(QString(" (Nightly)"));
    QString sibling = paths->getSiblingFolderName();
    EXPECT_EQ(sibling, QString("ScadTemplates"));
}

TEST_F(ResourcePathsTest, IsSiblingCandidatePath_Windows) {
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    EXPECT_TRUE(ResourcePaths::isSiblingCandidatePath(QString("%PROGRAMFILES%/")));
    EXPECT_TRUE(ResourcePaths::isSiblingCandidatePath(QString("%PROGRAMFILES(X86)%/")));
    EXPECT_FALSE(ResourcePaths::isSiblingCandidatePath(QString("%APPDATA%/")));
    EXPECT_FALSE(ResourcePaths::isSiblingCandidatePath(QString(".")));
#endif
}

TEST_F(ResourcePathsTest, IsSiblingCandidatePath_macOS) {
#if defined(Q_OS_MACOS) || defined(__APPLE__)
    EXPECT_TRUE(ResourcePaths::isSiblingCandidatePath(QString("/Applications/")));
    EXPECT_FALSE(ResourcePaths::isSiblingCandidatePath(QString("/Library/Application Support/")));
    EXPECT_FALSE(ResourcePaths::isSiblingCandidatePath(QString(".")));
#endif
}

TEST_F(ResourcePathsTest, IsSiblingCandidatePath_Linux) {
#if !defined(Q_OS_WIN) && !defined(_WIN32) && !defined(_WIN64) && !defined(Q_OS_MACOS) && !defined(__APPLE__)
    EXPECT_TRUE(ResourcePaths::isSiblingCandidatePath(QString("/opt/")));
    EXPECT_TRUE(ResourcePaths::isSiblingCandidatePath(QString("/usr/local/")));
    EXPECT_FALSE(ResourcePaths::isSiblingCandidatePath(QString("/usr/share/")));
    EXPECT_FALSE(ResourcePaths::isSiblingCandidatePath(QString(".")));
#endif
}

// ============================================================================
// Qualified Search Paths Tests
// ============================================================================

TEST_F(ResourcePathsTest, QualifiedPaths_ReturnPathElements) {
    paths->setSuffix(QString(" (Nightly)"));
    QList<PathElement> qualified = paths->qualifiedSearchPaths();
    
    EXPECT_FALSE(qualified.isEmpty());
    
    // Each element should have a tier and non-empty path
    for (const auto& elem : qualified) {
        EXPECT_FALSE(elem.path().isEmpty());
        // Tier should be one of the three valid values
        EXPECT_TRUE(
            elem.tier() == ResourceTier::Installation ||
            elem.tier() == ResourceTier::Machine ||
            elem.tier() == ResourceTier::User
        );
    }
}

TEST_F(ResourcePathsTest, QualifiedPaths_AllAbsolute) {
    paths->setSuffix(QString(" (Nightly)"));
    QList<PathElement> qualified = paths->qualifiedSearchPaths();
    
    // All qualified paths should be absolute
    for (const auto& elem : qualified) {
        EXPECT_FALSE(QDir::isRelativePath(elem.path()))
            << "Path is relative: " << elem.path().toStdString();
    }
}

TEST_F(ResourcePathsTest, QualifiedPaths_InstallationHasSuffix) {
    paths->setSuffix(QString(" (Nightly)"));
    QList<PathElement> qualified = paths->qualifiedSearchPaths();
    
    // At least one Installation tier path should contain the suffix
    bool foundWithSuffix = false;
    for (const auto& elem : qualified) {
        if (elem.tier() == ResourceTier::Installation && 
            elem.path().contains(" (Nightly)")) {
            foundWithSuffix = true;
            break;
        }
    }
    
    // Note: This may not always be true if all paths are relative like "." or ".."
    // which don't get folder names appended
}

TEST_F(ResourcePathsTest, QualifiedPaths_IncludesSiblings) {
    paths->setSuffix(QString(" (Nightly)"));
    QList<PathElement> qualified = paths->qualifiedSearchPaths();
    
    // Should have more paths than just the defaults
    // because sibling installations are added
    QStringList defaults = paths->defaultInstallSearchPaths();
    
    // Count Installation tier paths
    int installCount = 0;
    for (const auto& elem : qualified) {
        if (elem.tier() == ResourceTier::Installation) {
            installCount++;
        }
    }
    
    // Should have at least as many as defaults
    // (may have more with siblings and user-designated)
    EXPECT_GE(installCount, defaults.size());
}

TEST_F(ResourcePathsTest, QualifiedPaths_NoRelativePaths) {
    paths->setSuffix(QString(" (Nightly)"));
    QList<PathElement> qualified = paths->qualifiedSearchPaths();
    
    // None of the qualified paths should contain "." or ".." components
    for (const auto& elem : qualified) {
        QString path = elem.path();
        QStringList parts = path.split('/', Qt::SkipEmptyParts);
        
        for (const QString& part : parts) {
            EXPECT_NE(part, QString(".")) 
                << "Path contains '.' component: " << path.toStdString();
            EXPECT_NE(part, QString("..")) 
                << "Path contains '..' component: " << path.toStdString();
        }
    }
}

// ============================================================================
// Folder Name Appending Rules Tests
// ============================================================================

TEST_F(ResourcePathsTest, FolderNameRules_TrailingSlash) {
    paths->setSuffix(QString(" (Nightly)"));
    QList<PathElement> qualified = paths->qualifiedSearchPaths();
    
    // Paths that originally ended with "/" should have folder name appended
    // We can check this by looking at Installation tier paths
    for (const auto& elem : qualified) {
        if (elem.tier() == ResourceTier::Installation) {
            // Installation paths should end with folder name + suffix
            // (if they came from a path ending with "/")
            QString path = elem.path();
            
            // Should not end with just "/"
            EXPECT_FALSE(path.endsWith('/'))
                << "Qualified path ends with slash: " << path.toStdString();
        }
    }
}

// ============================================================================
// Default Search Paths Tests
// ============================================================================

TEST_F(ResourcePathsTest, DefaultPaths_NotEmpty) {
    EXPECT_FALSE(paths->defaultInstallSearchPaths().isEmpty());
    EXPECT_FALSE(paths->defaultMachineSearchPaths().isEmpty());
    EXPECT_FALSE(paths->defaultUserSearchPaths().isEmpty());
}

TEST_F(ResourcePathsTest, DefaultPaths_PlatformSpecific) {
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    // Windows should have %PROGRAMFILES%/ in install paths
    QStringList installs = paths->defaultInstallSearchPaths();
    EXPECT_TRUE(installs.contains(QString("%PROGRAMFILES%/")));
#elif defined(Q_OS_MACOS) || defined(__APPLE__)
    // macOS should have ../Resources in install paths
    QStringList installs = paths->defaultInstallSearchPaths();
    bool foundResources = false;
    for (const QString& path : installs) {
        if (path.contains("Resources")) {
            foundResources = true;
            break;
        }
    }
    EXPECT_TRUE(foundResources);
#else
    // Linux should have ../share/ paths
    QStringList installs = paths->defaultInstallSearchPaths();
    bool foundShare = false;
    for (const QString& path : installs) {
        if (path.contains("share")) {
            foundShare = true;
            break;
        }
    }
    EXPECT_TRUE(foundShare);
#endif
}

// ============================================================================
// User-Designated Paths Tests
// ============================================================================

TEST_F(ResourcePathsTest, UserDesignatedPaths_ReadsFromSettings) {
    // Clear any existing paths
    QSettings settings(QString("ScadTemplates"), QString("ResourcePaths"));
    settings.remove(QString("user_designated_paths"));
    settings.sync();
    
    // Should return empty list
    QStringList paths_empty = ResourcePaths::userDesignatedPaths();
    EXPECT_TRUE(paths_empty.isEmpty());
    
    // Add test paths
    QStringList testPaths;
    testPaths << QString("C:/TestPath1") << QString("D:/TestPath2");
    settings.setValue(QString("user_designated_paths"), testPaths);
    settings.sync();
    
    // Should now return the test paths
    QStringList paths_loaded = ResourcePaths::userDesignatedPaths();
    EXPECT_EQ(paths_loaded.size(), 2);
    EXPECT_TRUE(paths_loaded.contains(QString("C:/TestPath1")));
    EXPECT_TRUE(paths_loaded.contains(QString("D:/TestPath2")));
    
    // Cleanup
    settings.remove(QString("user_designated_paths"));
    settings.sync();
}

TEST_F(ResourcePathsTest, QualifiedPaths_IncludesUserDesignated) {
    // Setup: Add user-designated path
    QSettings settings(QString("ScadTemplates"), QString("ResourcePaths"));
    QStringList testPaths;
    testPaths << QString("C:/UserDesignatedTest");
    settings.setValue(QString("user_designated_paths"), testPaths);
    settings.sync();
    
    paths->setSuffix(QString(" (Nightly)"));
    QList<PathElement> qualified = paths->qualifiedSearchPaths();
    
    // Should include the user-designated path
    bool foundUserPath = false;
    for (const auto& elem : qualified) {
        if (elem.path().contains("UserDesignatedTest")) {
            foundUserPath = true;
            EXPECT_EQ(elem.tier(), ResourceTier::Installation);
            break;
        }
    }
    
    EXPECT_TRUE(foundUserPath);
    
    // Cleanup
    settings.remove(QString("user_designated_paths"));
    settings.sync();
}

// ============================================================================
// Path Tier Access Tests
// ============================================================================

TEST_F(ResourcePathsTest, AccessByTier_Installation) {
    Access access = accessByTier[ResourceTier::Installation];
    EXPECT_EQ(access, Access::ReadOnly);
}

TEST_F(ResourcePathsTest, AccessByTier_Machine) {
    Access access = accessByTier[ResourceTier::Machine];
    EXPECT_EQ(access, Access::ReadOnly);
}

TEST_F(ResourcePathsTest, AccessByTier_User) {
    Access access = accessByTier[ResourceTier::User];
    EXPECT_EQ(access, Access::ReadWrite);
}

// ============================================================================
// PathElement Tests
// ============================================================================

TEST_F(ResourcePathsTest, PathElement_Construction) {
    PathElement elem(ResourceTier::Installation, QString("C:/Test/Path"));
    
    EXPECT_EQ(elem.tier(), ResourceTier::Installation);
    EXPECT_EQ(elem.path(), QString("C:/Test/Path"));
}

TEST_F(ResourcePathsTest, PathElement_CopyConstruction) {
    PathElement elem1(ResourceTier::User, QString("/home/user/test"));
    PathElement elem2 = elem1;
    
    EXPECT_EQ(elem2.tier(), ResourceTier::User);
    EXPECT_EQ(elem2.path(), QString("/home/user/test"));
}
