/**
 * @file test_resource_paths.cpp
 * @brief Unit tests for ResourcePaths environment variable helpers
 */

#include <gtest/gtest.h>
#include <platformInfo/resourcePaths.h>

using platformInfo::ResourcePaths;

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
