/**
 * @file test_editsubtype.cpp
 * @brief Unit tests for EditSubtype enum and utility functions
 */

#include <gtest/gtest.h>
#include <scadtemplates/editsubtype.h>
#include <QString>

using namespace scadtemplates;

// Test getExtension for all subtypes
TEST(EditSubtypeTest, GetExtension) {
    EXPECT_EQ(getExtension(EditSubtype::Txt), QStringLiteral("txt"));
    EXPECT_EQ(getExtension(EditSubtype::Text), QStringLiteral("text"));
    EXPECT_EQ(getExtension(EditSubtype::Info), QStringLiteral("info"));
    EXPECT_EQ(getExtension(EditSubtype::Nfo), QStringLiteral("nfo"));
    EXPECT_EQ(getExtension(EditSubtype::Md), QStringLiteral("md"));
    EXPECT_EQ(getExtension(EditSubtype::Scad), QStringLiteral("scad"));
    EXPECT_EQ(getExtension(EditSubtype::Csg), QStringLiteral("csg"));
    EXPECT_EQ(getExtension(EditSubtype::Json), QStringLiteral("json"));
    EXPECT_TRUE(getExtension(EditSubtype::Unknown).isEmpty());
}

// Test getTitle for all subtypes
TEST(EditSubtypeTest, GetTitle) {
    EXPECT_EQ(getTitle(EditSubtype::Txt), QStringLiteral("Text File"));
    EXPECT_EQ(getTitle(EditSubtype::Text), QStringLiteral("Text File"));
    EXPECT_EQ(getTitle(EditSubtype::Info), QStringLiteral("Info File"));
    EXPECT_EQ(getTitle(EditSubtype::Nfo), QStringLiteral("NFO File"));
    EXPECT_EQ(getTitle(EditSubtype::Md), QStringLiteral("Markdown File"));
    EXPECT_EQ(getTitle(EditSubtype::Scad), QStringLiteral("OpenSCAD File"));
    EXPECT_EQ(getTitle(EditSubtype::Csg), QStringLiteral("CSG File"));
    EXPECT_EQ(getTitle(EditSubtype::Json), QStringLiteral("JSON File"));
    EXPECT_EQ(getTitle(EditSubtype::Unknown), QStringLiteral("Unknown"));
}

// Test getMimeType for all subtypes
TEST(EditSubtypeTest, GetMimeType) {
    EXPECT_EQ(getMimeType(EditSubtype::Txt), QStringLiteral("text/plain"));
    EXPECT_EQ(getMimeType(EditSubtype::Text), QStringLiteral("text/plain"));
    EXPECT_EQ(getMimeType(EditSubtype::Info), QStringLiteral("text/plain"));
    EXPECT_EQ(getMimeType(EditSubtype::Nfo), QStringLiteral("text/plain"));
    EXPECT_EQ(getMimeType(EditSubtype::Md), QStringLiteral("text/markdown"));
    EXPECT_EQ(getMimeType(EditSubtype::Scad), QStringLiteral("application/x-openscad"));
    EXPECT_EQ(getMimeType(EditSubtype::Csg), QStringLiteral("application/x-openscad"));
    EXPECT_EQ(getMimeType(EditSubtype::Json), QStringLiteral("application/json"));
    EXPECT_EQ(getMimeType(EditSubtype::Unknown), QStringLiteral("application/octet-stream"));
}

// Test subtypeFromExtension with extensions without dot
TEST(EditSubtypeTest, SubtypeFromExtensionNoDot) {
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("txt")), EditSubtype::Txt);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("text")), EditSubtype::Text);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("info")), EditSubtype::Info);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("nfo")), EditSubtype::Nfo);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("md")), EditSubtype::Md);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("scad")), EditSubtype::Scad);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("csg")), EditSubtype::Csg);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("json")), EditSubtype::Json);
}

// Test subtypeFromExtension with extensions with dot
TEST(EditSubtypeTest, SubtypeFromExtensionWithDot) {
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(".txt")), EditSubtype::Txt);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(".text")), EditSubtype::Text);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(".md")), EditSubtype::Md);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(".scad")), EditSubtype::Scad);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(".json")), EditSubtype::Json);
}

// Test subtypeFromExtension case insensitivity
TEST(EditSubtypeTest, SubtypeFromExtensionCaseInsensitive) {
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("TXT")), EditSubtype::Txt);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("Txt")), EditSubtype::Txt);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("SCAD")), EditSubtype::Scad);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("JSON")), EditSubtype::Json);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(".MD")), EditSubtype::Md);
}

// Test subtypeFromExtension with unknown extension
TEST(EditSubtypeTest, SubtypeFromExtensionUnknown) {
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("xyz")), EditSubtype::Unknown);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(".doc")), EditSubtype::Unknown);
    EXPECT_EQ(subtypeFromExtension(QString()), EditSubtype::Unknown);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("cpp")), EditSubtype::Unknown);
}

// Test getFilterPattern
TEST(EditSubtypeTest, GetFilterPattern) {
    EXPECT_EQ(getFilterPattern(EditSubtype::Txt), QStringLiteral("*.txt"));
    EXPECT_EQ(getFilterPattern(EditSubtype::Md), QStringLiteral("*.md"));
    EXPECT_EQ(getFilterPattern(EditSubtype::Scad), QStringLiteral("*.scad"));
    EXPECT_EQ(getFilterPattern(EditSubtype::Json), QStringLiteral("*.json"));
    EXPECT_EQ(getFilterPattern(EditSubtype::Unknown), QStringLiteral("*.*"));
}
