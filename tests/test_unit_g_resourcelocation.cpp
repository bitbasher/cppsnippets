/**
 * @file test_resourcelocation.cpp
 * @brief Unit tests for ResourceLocation display name generation
 */

#include <platformInfo/ResourceLocation.hpp>
#include <resourceInventory/resourceItem.hpp>
#include <gtest/gtest.h>
#include <QDir>
#include <QTemporaryDir>
#include <QStandardPaths>

using namespace platformInfo;
using namespace resourceInventory;

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
    EXPECT_TRUE(loc.getDisplayName().isEmpty());
}

TEST_F(ResourceLocationTest, PathOnlyConstructor) {
    QDir tempDir = QDir::temp();
    QString testPath = tempDir.absolutePath();
    
    ResourceLocation loc(testPath, ResourceTier::User);
    EXPECT_EQ(loc.path(), testPath);
    EXPECT_FALSE(loc.getDisplayName().isEmpty());
    // Display name might be same or shortened depending on path length
}

TEST_F(ResourceLocationTest, PathAndNameConstructor) {
    QString testPath = "/some/absolute/path";
    QString testName = "Custom Name";
    
    ResourceLocation loc(testPath, ResourceTier::User, QString(), testName);
    EXPECT_EQ(loc.path(), testPath);
    // Display name is now always generated from path (name parameter ignored)
    EXPECT_FALSE(loc.getDisplayName().isEmpty());
}

TEST_F(ResourceLocationTest, CopyConstructor) {
    ResourceLocation original("/test/path", ResourceTier::User, QString(), "Test Name");
    original.setDescription("Test Description");
    
    ResourceLocation copy(original);
    EXPECT_EQ(copy.path(), original.path());
    EXPECT_EQ(copy.getDisplayName(), original.getDisplayName());
    EXPECT_EQ(copy.description(), original.description());
}

// ============================================================================
// getDisplayName() Validation Tests
// ============================================================================

TEST_F(ResourceLocationTest, RejectsEmptyPath) {
    ResourceLocation loc;
    QString result = loc.getDisplayName();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ResourceLocationTest, ExtractsEnvironmentVariables) {
    // Test various environment variable formats - extracts just the env var
    
    // Windows batch style: %VARNAME%
    ResourceLocation loc1("C:\\%APPDATA%\\test", ResourceTier::User, "C:\\%APPDATA%\\test");
    EXPECT_EQ(loc1.getDisplayName(), "%APPDATA%");
    
    // Brace style: ${VARNAME}
    ResourceLocation loc2("/home/${USER}/test", ResourceTier::User, "/home/${USER}/test");
    EXPECT_EQ(loc2.getDisplayName(), "${USER}");
    
    // PowerShell style: $env:VARNAME
    ResourceLocation loc3("$env:USERPROFILE\\test", ResourceTier::User, "$env:USERPROFILE\\test");
    EXPECT_EQ(loc3.getDisplayName(), "$USERPROFILE");
    
    // PowerShell with braces: $env:{VARNAME}
    ResourceLocation loc4("$env:{USERPROFILE}\\test", ResourceTier::User, "$env:{USERPROFILE}\\test");
    EXPECT_EQ(loc4.getDisplayName(), "${USERPROFILE}");
    
    // Unix style: $VARNAME
    ResourceLocation loc5("$HOME/test", ResourceTier::User, "$HOME/test");
    EXPECT_EQ(loc5.getDisplayName(), "$HOME");
}

TEST_F(ResourceLocationTest, ExtractsFirstEnvironmentVariableOnly) {
    // When path contains multiple env vars, extracts first one based on check order:
    // 1. $env:VAR or $env:{VAR}
    // 2. ${VAR}
    // 3. $VAR
    // 4. %VAR%
    
    // Test 1: $env:VAR comes first (check order), even though ${VAR} appears first in string
    ResourceLocation loc1("${HOME}/$env:USERNAME/test", ResourceTier::User, "${HOME}/$env:USERNAME/test");
    EXPECT_EQ(loc1.getDisplayName(), "$USERNAME") << "Should extract $env:USERNAME (higher priority)";
    
    // Test 2: Swap order in string - should still extract $env:VAR first
    ResourceLocation loc2("$env:USERNAME/${HOME}/test", ResourceTier::User, "$env:USERNAME/${HOME}/test");
    EXPECT_EQ(loc2.getDisplayName(), "$USERNAME") << "Should extract $env:USERNAME regardless of position";
    
    // Test 3: ${VAR} and $VAR - ${VAR} has higher priority
    ResourceLocation loc3("$HOME/${USER}/test", ResourceTier::User, "$HOME/${USER}/test");
    EXPECT_EQ(loc3.getDisplayName(), "${USER}") << "Should extract ${USER} (higher priority than $HOME)";
    
    // Test 4: Swap order
    ResourceLocation loc4("${USER}/$HOME/test", ResourceTier::User, "${USER}/$HOME/test");
    EXPECT_EQ(loc4.getDisplayName(), "${USER}") << "Should extract ${USER} regardless of position";
    
    // Test 5: $VAR and %VAR% - $VAR has higher priority
    ResourceLocation loc5("$HOME/%USERPROFILE%/test", ResourceTier::User, "$HOME/%USERPROFILE%/test");
    EXPECT_EQ(loc5.getDisplayName(), "$HOME") << "Should extract $HOME (higher priority than %USERPROFILE%)";
    
    // Test 6: Swap order
    ResourceLocation loc6("%USERPROFILE%/$HOME/test", ResourceTier::User, "%USERPROFILE%/$HOME/test");
    EXPECT_EQ(loc6.getDisplayName(), "$HOME") << "Should extract $HOME regardless of position";
}

TEST_F(ResourceLocationTest, RejectsRelativePaths) {
    QString relative = "../relative/path";
    ResourceLocation loc(relative, ResourceTier::User);
    QString result = loc.getDisplayName();
    EXPECT_EQ(result, relative);  // Returns as-is if not absolute
}

// ============================================================================
// Short Path Tests
// ============================================================================

TEST_F(ResourceLocationTest, ShortPathReturnedAsIs) {
    // Short paths should be returned unchanged or minimally modified
#if defined(Q_OS_WIN)
    QString shortPath = "C:/Windows";
#else
    QString shortPath = "/usr/bin";
#endif
    ResourceLocation loc(shortPath, ResourceTier::Installation);
    QString result = loc.getDisplayName();
    
    EXPECT_LE(result.length(), 60);  // Within max length
    EXPECT_TRUE(result == shortPath || result.startsWith(shortPath)) 
        << "Expected: " << shortPath.toStdString() 
        << " Got: " << result.toStdString();
}

TEST_F(ResourceLocationTest, DriveRootReturnedAsIs) {
#if defined(Q_OS_WIN)
    ResourceLocation loc("C:/", ResourceTier::Installation);
    QString result = loc.getDisplayName();
    EXPECT_TRUE(result == "C:/" || result == "C:");
#else
    ResourceLocation loc("/", ResourceTier::Installation);
    QString result = loc.getDisplayName();
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
    ResourceLocation loc(testPath, ResourceTier::User);
    QString result = loc.getDisplayName();
    
    EXPECT_TRUE(result.startsWith("~"));
    EXPECT_FALSE(result.contains(m_homeDir));
}

TEST_F(ResourceLocationTest, HomeDirTildeResultsInShorterPath) {
    if (m_homeDir.isEmpty()) {
        GTEST_SKIP() << "Home directory not available";
    }
    
    QString testPath = m_homeDir + "/subfolder";
    ResourceLocation loc(testPath, ResourceTier::User);
    QString result = loc.getDisplayName();
    
    EXPECT_LT(result.length(), testPath.length());
}

// ============================================================================
// Long Path Truncation Tests
// ============================================================================

TEST_F(ResourceLocationTest, LongPathIsTruncated) {
    // Create a very long absolute path
    QString longPath = "/very/long/path/that/exceeds/the/maximum/display/length/configured/in/settings/with/many/folders";
    ResourceLocation loc(longPath, ResourceTier::User);
    QString result = loc.getDisplayName();
    
    // Should be truncated (default max is 60 chars)
    EXPECT_LE(result.length(), 60);
}

TEST_F(ResourceLocationTest, TruncatedPathContainsEllipsis) {
    QString longPath = "/very/long/path/that/exceeds/maximum/display/length/configured/settings/with/many/folders/here";
    ResourceLocation loc(longPath, ResourceTier::User);
    QString result = loc.getDisplayName();
    
    if (result.length() < longPath.length()) {
        EXPECT_TRUE(result.contains("..."));
    }
}

TEST_F(ResourceLocationTest, TruncatedPathShowsBeginningAndEnd) {
    QString longPath = "/start/of/path/that/exceeds/maximum/display/length/configured/in/settings/with/many/folders/end/of/path";
    ResourceLocation loc(longPath, ResourceTier::User);
    QString result = loc.getDisplayName();
    
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

TEST_F(ResourceLocationTest, SetPathUpdatesDisplayName) {
    ResourceLocation loc;
    
    QString newPath = "/new/test/path";
    loc.setPath(newPath);
    
    EXPECT_EQ(loc.path(), newPath);
    EXPECT_FALSE(loc.getDisplayName().isEmpty());
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(ResourceLocationTest, RealWorldWindowsPath) {
#if defined(Q_OS_WIN)
    QString winPath = "C:/Program Files/OpenSCAD";
    ResourceLocation loc(winPath, ResourceTier::Installation);
    QString result = loc.getDisplayName();
    
    EXPECT_FALSE(result.isEmpty());
    EXPECT_FALSE(result.contains("%"));
    EXPECT_FALSE(result.contains("${"));
#endif
}

TEST_F(ResourceLocationTest, RealWorldLinuxPath) {
#if !defined(Q_OS_WIN)
    QString linuxPath = "/usr/local/share/openscad";
    ResourceLocation loc(linuxPath, ResourceTier::Installation);
    QString result = loc.getDisplayName();
    
    EXPECT_FALSE(result.isEmpty());
    EXPECT_FALSE(result.contains("$"));
    EXPECT_FALSE(result.contains("%"));
#endif
}

TEST_F(ResourceLocationTest, RealWorldMacPath) {
#if defined(Q_OS_MACOS)
    QString macPath = "/Applications/OpenSCAD.app/Contents/Resources";
    ResourceLocation loc(macPath, ResourceTier::Installation);
    QString result = loc.getDisplayName();
    
    EXPECT_FALSE(result.isEmpty());
    EXPECT_FALSE(result.contains("$"));
#endif
}

// ============================================================================
// Main
// ============================================================================
// Note: main() is provided by test_main.cpp when building with other tests

