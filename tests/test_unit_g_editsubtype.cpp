/**
 * @file test_editsubtype.cpp
 * @brief Unit tests for EditSubtype enum and utility functions
 */

#include <gtest/gtest.h>
#include <scadtemplates/editsubtype.hpp>
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
    EXPECT_TRUE(getTitle(EditSubtype::Unknown).isEmpty());
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
    EXPECT_TRUE(getMimeType(EditSubtype::Unknown).isEmpty());
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

// Test getSubtypeInfo returns valid reference
TEST(EditSubtypeTest, GetSubtypeInfo) {
    const SubtypeInfo& txtInfo = getSubtypeInfo(EditSubtype::Txt);
    EXPECT_EQ(txtInfo.extension, QStringLiteral("txt"));
    EXPECT_EQ(txtInfo.title, QStringLiteral("Text File"));
    EXPECT_EQ(txtInfo.mimeType, QStringLiteral("text/plain"));
    
    const SubtypeInfo& scadInfo = getSubtypeInfo(EditSubtype::Scad);
    EXPECT_EQ(scadInfo.extension, QStringLiteral("scad"));
    EXPECT_EQ(scadInfo.title, QStringLiteral("OpenSCAD File"));
    EXPECT_EQ(scadInfo.mimeType, QStringLiteral("application/x-openscad"));
    
    const SubtypeInfo& unknownInfo = getSubtypeInfo(EditSubtype::Unknown);
    EXPECT_TRUE(unknownInfo.extension.isEmpty());
    EXPECT_TRUE(unknownInfo.title.isEmpty());
    EXPECT_TRUE(unknownInfo.mimeType.isEmpty());
}

// Test subtypeFromExtension with whitespace
TEST(EditSubtypeTest, SubtypeFromExtensionWithWhitespace) {
    // Leading/trailing whitespace should not match
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(" txt")), EditSubtype::Unknown);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("txt ")), EditSubtype::Unknown);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(" .scad ")), EditSubtype::Unknown);
}

// Test subtypeFromExtension with multiple dots
TEST(EditSubtypeTest, SubtypeFromExtensionMultipleDots) {
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("..txt")), EditSubtype::Unknown);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(".txt.scad")), EditSubtype::Unknown);
}

// Test subtypeFromExtension edge cases
TEST(EditSubtypeTest, SubtypeFromExtensionEdgeCases) {
    EXPECT_EQ(subtypeFromExtension(QStringLiteral(".")), EditSubtype::Unknown);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("..")), EditSubtype::Unknown);
    EXPECT_EQ(subtypeFromExtension(QStringLiteral("...")), EditSubtype::Unknown);
}

// Test round-trip: extension -> subtype -> extension
TEST(EditSubtypeTest, RoundTripExtension) {
    QStringList extensions = {
        QStringLiteral("txt"), QStringLiteral("text"), QStringLiteral("info"),
        QStringLiteral("nfo"), QStringLiteral("md"), QStringLiteral("scad"),
        QStringLiteral("csg"), QStringLiteral("json")
    };
    
    for (const QString& ext : extensions) {
        EditSubtype subtype = subtypeFromExtension(ext);
        EXPECT_NE(subtype, EditSubtype::Unknown);
        EXPECT_EQ(getExtension(subtype), ext);
    }
}
