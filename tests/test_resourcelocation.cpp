/**
 * @file test_resourcelocation.cpp
 * @brief Unit tests for ResourceLocation display name generation
 */

#include <platformInfo/ResourceLocation.hpp>
#include <gtest/gtest.h>
#include <QDir>
#include <QTemporaryDir>
#include <QStandardPaths>

using namespace platformInfo;

// ============================================================================
// ResourceLocation Display Name Tests
// ============================================================================

class ResourceLocationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Get home directory for tilde tests
        m_homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }
    
    QString m_homeDir;
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(ResourceLocationTest, DefaultConstructor) {
    ResourceLocation loc;
    EXPECT_TRUE(loc.path().isEmpty());
    EXPECT_TRUE(loc.displayName().isEmpty());
}

TEST_F(ResourceLocationTest, PathOnlyConstructor) {
    QDir tempDir = QDir::temp();
    QString testPath = tempDir.absolutePath();
    
    ResourceLocation loc(testPath);
    EXPECT_EQ(loc.path(), testPath);
    EXPECT_FALSE(loc.displayName().isEmpty());
    EXPECT_NE(loc.displayName(), testPath);  // Should be shortened
}

TEST_F(ResourceLocationTest, PathAndNameConstructor) {
    QString testPath = "/some/absolute/path";
    QString testName = "Custom Name";
    
    ResourceLocation loc(testPath, testName);
    EXPECT_EQ(loc.path(), testPath);
    EXPECT_EQ(loc.displayName(), testName);  // Should use provided name
}

TEST_F(ResourceLocationTest, CopyConstructor) {
    ResourceLocation original("/test/path", "Test Name");
    original.setDescription("Test Description");
    original.setWritable(true);
    
    ResourceLocation copy(original);
    EXPECT_EQ(copy.path(), original.path());
    EXPECT_EQ(copy.displayName(), original.displayName());
    EXPECT_EQ(copy.description(), original.description());
    EXPECT_EQ(copy.isWritable(), original.isWritable());
}

// ============================================================================
// generateDisplayName() Validation Tests
// ============================================================================

TEST_F(ResourceLocationTest, RejectsEmptyPath) {
    QString result = ResourceLocation::generateDisplayName(QString());
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ResourceLocationTest, RejectsEnvironmentVariables) {
    // Test various environment variable formats
    EXPECT_EQ(ResourceLocation::generateDisplayName("C:\\%APPDATA%\\test"), 
              "C:\\%APPDATA%\\test");  // Returns as-is
    EXPECT_EQ(ResourceLocation::generateDisplayName("/home/${USER}/test"), 
              "/home/${USER}/test");
    EXPECT_EQ(ResourceLocation::generateDisplayName("$env:USERPROFILE\\test"), 
              "$env:USERPROFILE\\test");
    EXPECT_EQ(ResourceLocation::generateDisplayName("path && command"), 
              "path && command");
}

TEST_F(ResourceLocationTest, RejectsRelativePaths) {
    QString relative = "../relative/path";
    QString result = ResourceLocation::generateDisplayName(relative);
    EXPECT_EQ(result, relative);  // Returns as-is if not absolute
}

// ============================================================================
// Short Path Tests
// ============================================================================

TEST_F(ResourceLocationTest, ShortPathReturnedAsIs) {
    // Paths < 24 characters should be returned unchanged
    QString shortPath = "/usr/bin";
    QString result = ResourceLocation::generateDisplayName(shortPath);
    
    EXPECT_EQ(result.length(), shortPath.length());
    EXPECT_TRUE(result == shortPath || result.startsWith(shortPath));
}

TEST_F(ResourceLocationTest, DriveRootReturnedAsIs) {
#if defined(Q_OS_WIN)
    QString result = ResourceLocation::generateDisplayName("C:/");
    EXPECT_TRUE(result == "C:/" || result == "C:");
#else
    QString result = ResourceLocation::generateDisplayName("/");
    EXPECT_EQ(result, "/");
#endif
}

// ============================================================================
// Home Directory Tilde Replacement Tests
// ============================================================================

TEST_F(ResourceLocationTest, HomeDirectoryReplacedWithTilde) {
    if (m_homeDir.isEmpty()) {
        GTEST_SKIP() << "Home directory not available";
    }
    
    QString testPath = m_homeDir + "/Documents/Test";
    QString result = ResourceLocation::generateDisplayName(testPath);
    
    EXPECT_TRUE(result.startsWith("~"));
    EXPECT_FALSE(result.contains(m_homeDir));
}

TEST_F(ResourceLocationTest, HomeDirTildeResultsInShorterPath) {
    if (m_homeDir.isEmpty()) {
        GTEST_SKIP() << "Home directory not available";
    }
    
    QString testPath = m_homeDir + "/subfolder";
    QString result = ResourceLocation::generateDisplayName(testPath);
    
    EXPECT_LT(result.length(), testPath.length());
}

// ============================================================================
// Long Path Truncation Tests
// ============================================================================

TEST_F(ResourceLocationTest, LongPathIsTruncated) {
    // Create a very long absolute path
    QString longPath = "/very/long/path/that/exceeds/the/maximum/display/length/configured/in/settings/with/many/folders";
    QString result = ResourceLocation::generateDisplayName(longPath);
    
    // Should be truncated (default max is 60 chars)
    EXPECT_LE(result.length(), 60);
}

TEST_F(ResourceLocationTest, TruncatedPathContainsEllipsis) {
    QString longPath = "/very/long/path/that/exceeds/maximum/display/length/configured/settings/with/many/folders/here";
    QString result = ResourceLocation::generateDisplayName(longPath);
    
    if (result.length() < longPath.length()) {
        EXPECT_TRUE(result.contains("..."));
    }
}

TEST_F(ResourceLocationTest, TruncatedPathShowsBeginningAndEnd) {
    QString longPath = "/start/of/path/that/exceeds/maximum/display/length/configured/in/settings/with/many/folders/end/of/path";
    QString result = ResourceLocation::generateDisplayName(longPath);
    
    if (result.contains("...")) {
        // Should show beginning and end
        int ellipsisPos = result.indexOf("...");
        EXPECT_GT(ellipsisPos, 0);  // Has beginning
        EXPECT_LT(ellipsisPos, result.length() - 3);  // Has end
    }
}

// ============================================================================
// Path Setter Tests
// ============================================================================

TEST_F(ResourceLocationTest, SetPathRegeneratesDisplayName) {
    ResourceLocation loc;
    
    QString newPath = "/new/test/path";
    loc.setPath(newPath);
    
    EXPECT_EQ(loc.path(), newPath);
    EXPECT_FALSE(loc.displayName().isEmpty());
}

TEST_F(ResourceLocationTest, SetDisplayNameOverridesGenerated) {
    QDir tempDir = QDir::temp();
    ResourceLocation loc(tempDir.absolutePath());
    
    QString original = loc.displayName();
    QString custom = "My Custom Name";
    
    loc.setDisplayName(custom);
    EXPECT_EQ(loc.displayName(), custom);
    EXPECT_NE(loc.displayName(), original);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(ResourceLocationTest, RealWorldWindowsPath) {
#if defined(Q_OS_WIN)
    QString winPath = "C:/Program Files/OpenSCAD";
    QString result = ResourceLocation::generateDisplayName(winPath);
    
    EXPECT_FALSE(result.isEmpty());
    EXPECT_FALSE(result.contains("%"));
    EXPECT_FALSE(result.contains("${"));
#endif
}

TEST_F(ResourceLocationTest, RealWorldLinuxPath) {
#if !defined(Q_OS_WIN)
    QString linuxPath = "/usr/local/share/openscad";
    QString result = ResourceLocation::generateDisplayName(linuxPath);
    
    EXPECT_FALSE(result.isEmpty());
    EXPECT_FALSE(result.contains("$"));
    EXPECT_FALSE(result.contains("%"));
#endif
}

TEST_F(ResourceLocationTest, RealWorldMacPath) {
#if defined(Q_OS_MACOS)
    QString macPath = "/Applications/OpenSCAD.app/Contents/Resources";
    QString result = ResourceLocation::generateDisplayName(macPath);
    
    EXPECT_FALSE(result.isEmpty());
    EXPECT_FALSE(result.contains("$"));
#endif
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
