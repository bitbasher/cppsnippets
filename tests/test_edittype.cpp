/**
 * @file test_edittype.cpp
 * @brief Unit tests for EditType enum and utility functions
 */

#include <gtest/gtest.h>
#include <scadtemplates/edittype.h>
#include <QString>
#include <QVector>

using namespace scadtemplates;

// Test getTitle for all types
TEST(EditTypeTest, GetTitle) {
    EXPECT_EQ(getTitle(EditType::Text), QStringLiteral("Text Files"));
    EXPECT_EQ(getTitle(EditType::Markdown), QStringLiteral("Markdown Files"));
    EXPECT_EQ(getTitle(EditType::OpenSCAD), QStringLiteral("OpenSCAD Files"));
    EXPECT_EQ(getTitle(EditType::Json), QStringLiteral("JSON Files"));
    EXPECT_EQ(getTitle(EditType::Unknown), QStringLiteral("Unknown"));
}

// Test getMimeType for all types
TEST(EditTypeTest, GetMimeType) {
    EXPECT_EQ(getMimeType(EditType::Text), QStringLiteral("text/plain"));
    EXPECT_EQ(getMimeType(EditType::Markdown), QStringLiteral("text/markdown"));
    EXPECT_EQ(getMimeType(EditType::OpenSCAD), QStringLiteral("application/x-openscad"));
    EXPECT_EQ(getMimeType(EditType::Json), QStringLiteral("application/json"));
    EXPECT_EQ(getMimeType(EditType::Unknown), QStringLiteral("application/octet-stream"));
}

// Test getSubtypes returns correct subtypes for Text
TEST(EditTypeTest, GetSubtypesText) {
    QVector<EditSubtype> subtypes = getSubtypes(EditType::Text);
    EXPECT_EQ(subtypes.size(), 4);
    EXPECT_EQ(subtypes[0], EditSubtype::Txt);
    EXPECT_EQ(subtypes[1], EditSubtype::Text);
    EXPECT_EQ(subtypes[2], EditSubtype::Info);
    EXPECT_EQ(subtypes[3], EditSubtype::Nfo);
}

// Test getSubtypes returns correct subtypes for Markdown
TEST(EditTypeTest, GetSubtypesMarkdown) {
    QVector<EditSubtype> subtypes = getSubtypes(EditType::Markdown);
    EXPECT_EQ(subtypes.size(), 1);
    EXPECT_EQ(subtypes[0], EditSubtype::Md);
}

// Test getSubtypes returns correct subtypes for OpenSCAD
TEST(EditTypeTest, GetSubtypesOpenSCAD) {
    QVector<EditSubtype> subtypes = getSubtypes(EditType::OpenSCAD);
    EXPECT_EQ(subtypes.size(), 2);
    EXPECT_EQ(subtypes[0], EditSubtype::Scad);
    EXPECT_EQ(subtypes[1], EditSubtype::Csg);
}

// Test getSubtypes returns correct subtypes for Json
TEST(EditTypeTest, GetSubtypesJson) {
    QVector<EditSubtype> subtypes = getSubtypes(EditType::Json);
    EXPECT_EQ(subtypes.size(), 1);
    EXPECT_EQ(subtypes[0], EditSubtype::Json);
}

// Test typeFromSubtype
TEST(EditTypeTest, TypeFromSubtype) {
    EXPECT_EQ(typeFromSubtype(EditSubtype::Txt), EditType::Text);
    EXPECT_EQ(typeFromSubtype(EditSubtype::Text), EditType::Text);
    EXPECT_EQ(typeFromSubtype(EditSubtype::Info), EditType::Text);
    EXPECT_EQ(typeFromSubtype(EditSubtype::Nfo), EditType::Text);
    EXPECT_EQ(typeFromSubtype(EditSubtype::Md), EditType::Markdown);
    EXPECT_EQ(typeFromSubtype(EditSubtype::Scad), EditType::OpenSCAD);
    EXPECT_EQ(typeFromSubtype(EditSubtype::Csg), EditType::OpenSCAD);
    EXPECT_EQ(typeFromSubtype(EditSubtype::Json), EditType::Json);
    EXPECT_EQ(typeFromSubtype(EditSubtype::Unknown), EditType::Unknown);
}

// Test typeFromExtension
TEST(EditTypeTest, TypeFromExtension) {
    EXPECT_EQ(typeFromExtension(QStringLiteral("txt")), EditType::Text);
    EXPECT_EQ(typeFromExtension(QStringLiteral(".txt")), EditType::Text);
    EXPECT_EQ(typeFromExtension(QStringLiteral("md")), EditType::Markdown);
    EXPECT_EQ(typeFromExtension(QStringLiteral(".scad")), EditType::OpenSCAD);
    EXPECT_EQ(typeFromExtension(QStringLiteral("csg")), EditType::OpenSCAD);
    EXPECT_EQ(typeFromExtension(QStringLiteral("json")), EditType::Json);
    EXPECT_EQ(typeFromExtension(QStringLiteral("xyz")), EditType::Unknown);
}

// Test getFileDialogFilter for Text type
TEST(EditTypeTest, GetFileDialogFilterText) {
    QString filter = getFileDialogFilter(EditType::Text);
    EXPECT_EQ(filter, QStringLiteral("Text Files (*.txt *.text *.info *.nfo)"));
}

// Test getFileDialogFilter for Markdown type
TEST(EditTypeTest, GetFileDialogFilterMarkdown) {
    QString filter = getFileDialogFilter(EditType::Markdown);
    EXPECT_EQ(filter, QStringLiteral("Markdown Files (*.md)"));
}

// Test getFileDialogFilter for OpenSCAD type
TEST(EditTypeTest, GetFileDialogFilterOpenSCAD) {
    QString filter = getFileDialogFilter(EditType::OpenSCAD);
    EXPECT_EQ(filter, QStringLiteral("OpenSCAD Files (*.scad *.csg)"));
}

// Test getFileDialogFilter for Json type
TEST(EditTypeTest, GetFileDialogFilterJson) {
    QString filter = getFileDialogFilter(EditType::Json);
    EXPECT_EQ(filter, QStringLiteral("JSON Files (*.json)"));
}

// Test getAllFileDialogFilters
TEST(EditTypeTest, GetAllFileDialogFilters) {
    QString filters = getAllFileDialogFilters();
    
    // Should start with "All Supported Files"
    EXPECT_TRUE(filters.contains(QStringLiteral("All Supported Files")));
    
    // Should contain all individual type filters
    EXPECT_TRUE(filters.contains(QStringLiteral("Text Files")));
    EXPECT_TRUE(filters.contains(QStringLiteral("Markdown Files")));
    EXPECT_TRUE(filters.contains(QStringLiteral("OpenSCAD Files")));
    EXPECT_TRUE(filters.contains(QStringLiteral("JSON Files")));
    
    // Should contain all extensions
    EXPECT_TRUE(filters.contains(QStringLiteral("*.txt")));
    EXPECT_TRUE(filters.contains(QStringLiteral("*.md")));
    EXPECT_TRUE(filters.contains(QStringLiteral("*.scad")));
    EXPECT_TRUE(filters.contains(QStringLiteral("*.json")));
    
    // Should end with "All Files"
    EXPECT_TRUE(filters.contains(QStringLiteral("All Files (*.*)")));
}

// Test getAllTypes
TEST(EditTypeTest, GetAllTypes) {
    QVector<EditType> types = getAllTypes();
    EXPECT_EQ(types.size(), 4);
    
    // Should not contain Unknown
    for (auto type : types) {
        EXPECT_NE(type, EditType::Unknown);
    }
    
    // Should contain all valid types
    bool hasText = false, hasMarkdown = false, hasOpenSCAD = false, hasJson = false;
    for (auto type : types) {
        if (type == EditType::Text) hasText = true;
        if (type == EditType::Markdown) hasMarkdown = true;
        if (type == EditType::OpenSCAD) hasOpenSCAD = true;
        if (type == EditType::Json) hasJson = true;
    }
    EXPECT_TRUE(hasText);
    EXPECT_TRUE(hasMarkdown);
    EXPECT_TRUE(hasOpenSCAD);
    EXPECT_TRUE(hasJson);
}

// Test filter string format is suitable for Qt file dialogs
TEST(EditTypeTest, FilterStringFormatForQt) {
    QString filters = getAllFileDialogFilters();
    
    // Qt uses ";;" as separator between filter entries
    int separatorCount = filters.count(QStringLiteral(";;"));
    // Should have separators: All Supported ;; Text ;; Markdown ;; OpenSCAD ;; JSON ;; All Files
    EXPECT_GE(separatorCount, 5);
}
