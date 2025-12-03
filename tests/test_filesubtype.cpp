/**
 * @file test_filesubtype.cpp
 * @brief Unit tests for FileSubtype class
 */

#include <gtest/gtest.h>
#include <scadtemplates/filesubtype.h>
#include <QString>

using namespace scadtemplates;

// Test FileSubtype construction and accessors
TEST(FileSubtypeTest, ConstexprConstruction) {
    FileSubtype test{QStringLiteral("test"), QStringLiteral("Test File"), QStringLiteral("text/plain")};
    EXPECT_EQ(test.extension(), QStringLiteral("test"));
    EXPECT_EQ(test.title(), QStringLiteral("Test File"));
    EXPECT_EQ(test.mimeType(), QStringLiteral("text/plain"));
}

// Test predefined subtypes
TEST(FileSubtypeTest, PredefinedSubtypes) {
    EXPECT_EQ(subtypes::Txt.extension(), QStringLiteral("txt"));
    EXPECT_EQ(subtypes::Txt.title(), QStringLiteral("Text File"));
    EXPECT_EQ(subtypes::Txt.mimeType(), QStringLiteral("text/plain"));
    
    EXPECT_EQ(subtypes::Scad.extension(), QStringLiteral("scad"));
    EXPECT_EQ(subtypes::Scad.title(), QStringLiteral("OpenSCAD File"));
    EXPECT_EQ(subtypes::Scad.mimeType(), QStringLiteral("application/x-openscad"));
    
    EXPECT_EQ(subtypes::Json.extension(), QStringLiteral("json"));
    EXPECT_EQ(subtypes::Json.title(), QStringLiteral("JSON File"));
    EXPECT_EQ(subtypes::Json.mimeType(), QStringLiteral("application/json"));
}

// Test dotExtension method
TEST(FileSubtypeTest, DotExtension) {
    EXPECT_EQ(subtypes::Txt.dotExtension(), QStringLiteral(".txt"));
    EXPECT_EQ(subtypes::Scad.dotExtension(), QStringLiteral(".scad"));
    EXPECT_EQ(subtypes::Md.dotExtension(), QStringLiteral(".md"));
}

// Test globPattern method
TEST(FileSubtypeTest, GlobPattern) {
    EXPECT_EQ(subtypes::Txt.globPattern(), QStringLiteral("*.txt"));
    EXPECT_EQ(subtypes::Scad.globPattern(), QStringLiteral("*.scad"));
    EXPECT_EQ(subtypes::Json.globPattern(), QStringLiteral("*.json"));
}

// Test matchesFilename
TEST(FileSubtypeTest, MatchesFilename) {
    EXPECT_TRUE(subtypes::Txt.matchesFilename(QStringLiteral("readme.txt")));
    EXPECT_TRUE(subtypes::Txt.matchesFilename(QStringLiteral("README.TXT")));
    EXPECT_TRUE(subtypes::Txt.matchesFilename(QStringLiteral("file.name.txt")));
    EXPECT_FALSE(subtypes::Txt.matchesFilename(QStringLiteral("readme.md")));
    EXPECT_FALSE(subtypes::Txt.matchesFilename(QStringLiteral("readme")));
    EXPECT_FALSE(subtypes::Txt.matchesFilename(QStringLiteral("txt")));
}

// Test matchesExtension
TEST(FileSubtypeTest, MatchesExtension) {
    EXPECT_TRUE(subtypes::Scad.matchesExtension(QStringLiteral("scad")));
    EXPECT_TRUE(subtypes::Scad.matchesExtension(QStringLiteral("SCAD")));
    EXPECT_TRUE(subtypes::Scad.matchesExtension(QStringLiteral(".scad")));
    EXPECT_TRUE(subtypes::Scad.matchesExtension(QStringLiteral(".SCAD")));
    EXPECT_FALSE(subtypes::Scad.matchesExtension(QStringLiteral("csg")));
    EXPECT_FALSE(subtypes::Scad.matchesExtension(QString()));
}

// Test findSubtypeByExtension
TEST(FileSubtypeTest, FindSubtypeByExtension) {
    const FileSubtype* result = findSubtypeByExtension(QStringLiteral("txt"));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->extension(), QStringLiteral("txt"));
    
    result = findSubtypeByExtension(QStringLiteral(".txt"));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->extension(), QStringLiteral("txt"));
    
    result = findSubtypeByExtension(QStringLiteral("TXT"));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->extension(), QStringLiteral("txt"));
    
    result = findSubtypeByExtension(QStringLiteral("scad"));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->extension(), QStringLiteral("scad"));
    
    result = findSubtypeByExtension(QStringLiteral("json"));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->extension(), QStringLiteral("json"));
    
    result = findSubtypeByExtension(QStringLiteral("md"));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->extension(), QStringLiteral("md"));
    
    EXPECT_EQ(findSubtypeByExtension(QStringLiteral("xyz")), nullptr);
    EXPECT_EQ(findSubtypeByExtension(QString()), nullptr);
}

// Test all text subtypes
TEST(FileSubtypeTest, AllTextSubtypes) {
    EXPECT_EQ(subtypes::Txt.mimeType(), QStringLiteral("text/plain"));
    EXPECT_EQ(subtypes::Text.mimeType(), QStringLiteral("text/plain"));
    EXPECT_EQ(subtypes::Info.mimeType(), QStringLiteral("text/plain"));
    EXPECT_EQ(subtypes::Nfo.mimeType(), QStringLiteral("text/plain"));
}

// Test all OpenSCAD subtypes
TEST(FileSubtypeTest, AllOpenSCADSubtypes) {
    EXPECT_EQ(subtypes::Scad.mimeType(), QStringLiteral("application/x-openscad"));
    EXPECT_EQ(subtypes::Csg.mimeType(), QStringLiteral("application/x-openscad"));
}
