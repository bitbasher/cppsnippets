/**
 * @file test_resource_paths.cpp
 * @brief Unit tests for ResourcePaths environment variable helpers
 */

#include <gtest/gtest.h>
#include <platformInfo/resourcePaths.h>
#include <resInventory/ResourceLocation.h>
#include <QDir>

using platformInfo::ResourcePaths;
using platformInfo::ResourceLocation;

TEST(ResourcePathsEnvVars, AddUpdateRemove) {
    ResourcePaths paths;

    paths.addEnvVar(QStringLiteral("FOO"), QStringLiteral("one"));
    EXPECT_EQ(paths.envVars().size(), 1);
    EXPECT_EQ(paths.envVarValue(QStringLiteral("FOO")), QStringLiteral("one"));

    paths.addEnvVar(QStringLiteral("FOO"), QStringLiteral("two"));
    EXPECT_EQ(paths.envVars().size(), 1);
    EXPECT_EQ(paths.envVarValue(QStringLiteral("FOO")), QStringLiteral("two"));

    paths.removeEnvVar(QStringLiteral("FOO"));
    EXPECT_TRUE(paths.envVars().isEmpty());
    EXPECT_TRUE(paths.envVarValue(QStringLiteral("FOO")).isEmpty());
}

TEST(ResourcePathsEnvVars, ExpandPlaceholdersAndMissing) {
    ResourcePaths paths;
    paths.setEnvVars({
        {QStringLiteral("FOO"), QStringLiteral("foo")},
        {QStringLiteral("BAR"), QStringLiteral("bar")},
    });

    const QString expanded = paths.expandEnvVars(
        QStringLiteral("/root/${FOO}/%BAR%/${MISSING}/tail"));

    EXPECT_EQ(expanded, QStringLiteral("/root/foo/bar//tail"));
}

TEST(ResourcePathsEnvVars, OverridesSystemEnvironment) {
    ResourcePaths paths;
    paths.addEnvVar(QStringLiteral("PATH"), QStringLiteral("OVERRIDE_VALUE"));

    const QString expanded = paths.expandEnvVars(QStringLiteral("%PATH%"));
    EXPECT_EQ(expanded, QStringLiteral("OVERRIDE_VALUE"));

    EXPECT_EQ(paths.envVarValue(QStringLiteral("PATH")),
              QStringLiteral("OVERRIDE_VALUE"));
}

// ============================================================================
// ResourceLocation Placeholder Tests
// ============================================================================

TEST(ResourceLocationPlaceholder, ExpandWithEnvVars) {
    ResourcePaths paths;
    paths.addEnvVar(QStringLiteral("TESTDIR"), QStringLiteral("MyTestDir"));
    paths.addEnvVar(QStringLiteral("SUBDIR"), QStringLiteral("sub"));

    // Create location with template
    ResourceLocation loc(QStringLiteral("${TESTDIR}/%SUBDIR%/resources"),
                         QStringLiteral("Test Location"),
                         QStringLiteral("Location with placeholders"),
                         true);  // isTemplate = true

    EXPECT_EQ(loc.rawPath(), QStringLiteral("${TESTDIR}/%SUBDIR%/resources"));
    
    QString expanded = loc.expandedPath(paths);
    // Should contain expanded values (path format depends on OS canonicalization)
    EXPECT_TRUE(expanded.contains(QStringLiteral("MyTestDir")));
    EXPECT_TRUE(expanded.contains(QStringLiteral("sub")));
}

TEST(ResourceLocationPlaceholder, NonTemplateUsesDirectPath) {
    ResourcePaths paths;
    
    // Create location without template
    ResourceLocation loc(QStringLiteral("C:/direct/path"),
                         QStringLiteral("Direct"),
                         QStringLiteral("No template"),
                         false);  // isTemplate = false

    EXPECT_TRUE(loc.rawPath().isEmpty());
    EXPECT_EQ(loc.path, QStringLiteral("C:/direct/path"));
    
    QString expanded = loc.expandedPath(paths);
    // Without rawPath, expandedPath returns path member
    EXPECT_EQ(expanded, QStringLiteral("C:/direct/path"));
}

TEST(ResourceLocationPlaceholder, RelativePathBecomesAbsolute) {
    ResourcePaths paths;
    
    // Create location with relative path template
    ResourceLocation loc(QStringLiteral("./relative/path"),
                         QStringLiteral("Relative"),
                         QStringLiteral("Relative template"),
                         true);

    QString expanded = loc.expandedPath(paths);
    QDir currentDir = QDir::current();
    
    // Expanded path should be absolute
    EXPECT_TRUE(QDir::isAbsolutePath(expanded));
    // Should resolve relative to current directory
    EXPECT_TRUE(expanded.contains(currentDir.absolutePath()));
}

// ============================================================================
// Default Path Template Tests
// ============================================================================

TEST(ResourcePathsDefaults, ExpandUserSearchPaths) {
    ResourcePaths paths;
    
    // Default paths may contain templates like ${HOME}, %APPDATA%, etc.
    QStringList expanded = paths.expandedSearchPaths(resourceInfo::ResourceTier::User);
    
    // Should have at least some paths
    EXPECT_FALSE(expanded.isEmpty());
    
    // None should contain unreplaced template syntax if env vars exist
    for (const QString& path : expanded) {
        // Check for common env vars that should exist
        if (path.contains("${HOME}") || path.contains("%HOME%")) {
            // HOME should be defined on all platforms, so it should be expanded
            FAIL() << "Path contains unexpanded ${HOME}: " << path.toStdString();
        }
    }
}

TEST(ResourcePathsDefaults, ExpandMachineSearchPaths) {
    ResourcePaths paths;
    
    QStringList expanded = paths.expandedSearchPaths(resourceInfo::ResourceTier::Machine);
    
    // Should have at least one path
    EXPECT_FALSE(expanded.isEmpty());
    
    // Platform-specific validation
#if defined(Q_OS_WIN) || defined(_WIN32) || defined(_WIN64)
    // Windows should expand %PROGRAMDATA%
    bool foundExpanded = false;
    for (const QString& path : expanded) {
        if (path.contains("ProgramData", Qt::CaseInsensitive) && 
            !path.contains("%PROGRAMDATA%")) {
            foundExpanded = true;
            break;
        }
    }
    EXPECT_TRUE(foundExpanded) << "Windows machine paths should expand %PROGRAMDATA%";
#endif
}
