/**
 * @file test_filetype.cpp
 * @brief Unit tests for FileType class
 */

#include <gtest/gtest.h>
#include <scadtemplates/filetype.h>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QDir>
#include <QFile>
#include <QTextStream>

using namespace scadtemplates;

// Test predefined FileType instances
TEST(FileTypeTest, PredefinedTextType) {
    EXPECT_EQ(filetypes::Text.title(), QStringLiteral("Text Files"));
    EXPECT_EQ(filetypes::Text.mimeType(), QStringLiteral("text/plain"));
    EXPECT_EQ(filetypes::Text.subtypeCount(), 4);
}

TEST(FileTypeTest, PredefinedMarkdownType) {
    EXPECT_EQ(filetypes::Markdown.title(), QStringLiteral("Markdown Files"));
    EXPECT_EQ(filetypes::Markdown.mimeType(), QStringLiteral("text/markdown"));
    EXPECT_EQ(filetypes::Markdown.subtypeCount(), 1);
}

TEST(FileTypeTest, PredefinedOpenSCADType) {
    EXPECT_EQ(filetypes::OpenSCAD.title(), QStringLiteral("OpenSCAD Files"));
    EXPECT_EQ(filetypes::OpenSCAD.mimeType(), QStringLiteral("application/x-openscad"));
    EXPECT_EQ(filetypes::OpenSCAD.subtypeCount(), 2);
}

TEST(FileTypeTest, PredefinedJsonType) {
    EXPECT_EQ(filetypes::Json.title(), QStringLiteral("JSON Files"));
    EXPECT_EQ(filetypes::Json.mimeType(), QStringLiteral("application/json"));
    EXPECT_EQ(filetypes::Json.subtypeCount(), 1);
}

// Test subtypes access
TEST(FileTypeTest, TextSubtypes) {
    const auto& subtypes = filetypes::Text.subtypes();
    EXPECT_EQ(subtypes.size(), 4);
    EXPECT_EQ(subtypes[0]->extension(), QStringLiteral("txt"));
    EXPECT_EQ(subtypes[1]->extension(), QStringLiteral("text"));
    EXPECT_EQ(subtypes[2]->extension(), QStringLiteral("info"));
    EXPECT_EQ(subtypes[3]->extension(), QStringLiteral("nfo"));
}

TEST(FileTypeTest, OpenSCADSubtypes) {
    const auto& subtypes = filetypes::OpenSCAD.subtypes();
    EXPECT_EQ(subtypes.size(), 2);
    EXPECT_EQ(subtypes[0]->extension(), QStringLiteral("scad"));
    EXPECT_EQ(subtypes[1]->extension(), QStringLiteral("csg"));
}

// Test fileDialogFilter
TEST(FileTypeTest, FileDialogFilterText) {
    QString filter = filetypes::Text.fileDialogFilter();
    EXPECT_EQ(filter, QStringLiteral("Text Files (*.txt *.text *.info *.nfo)"));
}

TEST(FileTypeTest, FileDialogFilterMarkdown) {
    QString filter = filetypes::Markdown.fileDialogFilter();
    EXPECT_EQ(filter, QStringLiteral("Markdown Files (*.md)"));
}

TEST(FileTypeTest, FileDialogFilterOpenSCAD) {
    QString filter = filetypes::OpenSCAD.fileDialogFilter();
    EXPECT_EQ(filter, QStringLiteral("OpenSCAD Files (*.scad *.csg)"));
}

TEST(FileTypeTest, FileDialogFilterJson) {
    QString filter = filetypes::Json.fileDialogFilter();
    EXPECT_EQ(filter, QStringLiteral("JSON Files (*.json)"));
}

// Test globPatterns
TEST(FileTypeTest, GlobPatterns) {
    QStringList patterns = filetypes::Text.globPatterns();
    EXPECT_EQ(patterns.size(), 4);
    EXPECT_EQ(patterns[0], QStringLiteral("*.txt"));
    EXPECT_EQ(patterns[1], QStringLiteral("*.text"));
    EXPECT_EQ(patterns[2], QStringLiteral("*.info"));
    EXPECT_EQ(patterns[3], QStringLiteral("*.nfo"));
}

// Test combinedGlobPattern
TEST(FileTypeTest, CombinedGlobPattern) {
    EXPECT_EQ(filetypes::Text.combinedGlobPattern(), QStringLiteral("*.txt *.text *.info *.nfo"));
    EXPECT_EQ(filetypes::OpenSCAD.combinedGlobPattern(), QStringLiteral("*.scad *.csg"));
    EXPECT_EQ(filetypes::Json.combinedGlobPattern(), QStringLiteral("*.json"));
}

// Test matchesFilename
TEST(FileTypeTest, MatchesFilename) {
    EXPECT_TRUE(filetypes::Text.matchesFilename(QStringLiteral("readme.txt")));
    EXPECT_TRUE(filetypes::Text.matchesFilename(QStringLiteral("notes.text")));
    EXPECT_TRUE(filetypes::Text.matchesFilename(QStringLiteral("file.info")));
    EXPECT_TRUE(filetypes::Text.matchesFilename(QStringLiteral("release.nfo")));
    EXPECT_FALSE(filetypes::Text.matchesFilename(QStringLiteral("readme.md")));
    
    EXPECT_TRUE(filetypes::OpenSCAD.matchesFilename(QStringLiteral("model.scad")));
    EXPECT_TRUE(filetypes::OpenSCAD.matchesFilename(QStringLiteral("output.csg")));
    EXPECT_FALSE(filetypes::OpenSCAD.matchesFilename(QStringLiteral("model.stl")));
}

// Test matchesExtension
TEST(FileTypeTest, MatchesExtension) {
    EXPECT_TRUE(filetypes::Text.matchesExtension(QStringLiteral("txt")));
    EXPECT_TRUE(filetypes::Text.matchesExtension(QStringLiteral(".txt")));
    EXPECT_TRUE(filetypes::Text.matchesExtension(QStringLiteral("TXT")));
    EXPECT_TRUE(filetypes::Text.matchesExtension(QStringLiteral("text")));
    EXPECT_FALSE(filetypes::Text.matchesExtension(QStringLiteral("md")));
    
    EXPECT_TRUE(filetypes::OpenSCAD.matchesExtension(QStringLiteral("scad")));
    EXPECT_TRUE(filetypes::OpenSCAD.matchesExtension(QStringLiteral("csg")));
    EXPECT_FALSE(filetypes::OpenSCAD.matchesExtension(QStringLiteral("json")));
}

// Test findSubtype
TEST(FileTypeTest, FindSubtype) {
    const FileSubtype* subtype = filetypes::Text.findSubtype(QStringLiteral("txt"));
    ASSERT_NE(subtype, nullptr);
    EXPECT_EQ(subtype->extension(), QStringLiteral("txt"));
    
    subtype = filetypes::Text.findSubtype(QStringLiteral("nfo"));
    ASSERT_NE(subtype, nullptr);
    EXPECT_EQ(subtype->extension(), QStringLiteral("nfo"));
    
    subtype = filetypes::Text.findSubtype(QStringLiteral("md"));
    EXPECT_EQ(subtype, nullptr);
}

// Test getAllFileTypes
TEST(FileTypeTest, GetAllFileTypes) {
    QVector<const FileType*> types = getAllFileTypes();
    EXPECT_EQ(types.size(), 4);
    EXPECT_EQ(types[0], &filetypes::Text);
    EXPECT_EQ(types[1], &filetypes::Markdown);
    EXPECT_EQ(types[2], &filetypes::OpenSCAD);
    EXPECT_EQ(types[3], &filetypes::Json);
}

// Test findFileTypeByExtension
TEST(FileTypeTest, FindFileTypeByExtension) {
    EXPECT_EQ(findFileTypeByExtension(QStringLiteral("txt")), &filetypes::Text);
    EXPECT_EQ(findFileTypeByExtension(QStringLiteral(".txt")), &filetypes::Text);
    EXPECT_EQ(findFileTypeByExtension(QStringLiteral("text")), &filetypes::Text);
    EXPECT_EQ(findFileTypeByExtension(QStringLiteral("md")), &filetypes::Markdown);
    EXPECT_EQ(findFileTypeByExtension(QStringLiteral("scad")), &filetypes::OpenSCAD);
    EXPECT_EQ(findFileTypeByExtension(QStringLiteral("csg")), &filetypes::OpenSCAD);
    EXPECT_EQ(findFileTypeByExtension(QStringLiteral("json")), &filetypes::Json);
    EXPECT_EQ(findFileTypeByExtension(QStringLiteral("xyz")), nullptr);
}

// Test findFileTypeByFilename
TEST(FileTypeTest, FindFileTypeByFilename) {
    EXPECT_EQ(findFileTypeByFilename(QStringLiteral("readme.txt")), &filetypes::Text);
    EXPECT_EQ(findFileTypeByFilename(QStringLiteral("README.MD")), &filetypes::Markdown);
    EXPECT_EQ(findFileTypeByFilename(QStringLiteral("model.scad")), &filetypes::OpenSCAD);
    EXPECT_EQ(findFileTypeByFilename(QStringLiteral("config.json")), &filetypes::Json);
    EXPECT_EQ(findFileTypeByFilename(QStringLiteral("unknown.xyz")), nullptr);
}

// Test getFileTypeDialogFilters
TEST(FileTypeTest, GetFileTypeDialogFilters) {
    QString filters = getFileTypeDialogFilters();
    
    // Should contain "All Supported Files"
    EXPECT_TRUE(filters.contains(QStringLiteral("All Supported Files")));
    
    // Should contain all type filters
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

// Test file searching (create temp directory with test files)
class FileTypeSearchTest : public ::testing::Test {
protected:
    QString tempDir;
    
    void SetUp() override {
        tempDir = QDir::tempPath() + QStringLiteral("/scadtemplates_test");
        QDir().mkpath(tempDir);
        QDir().mkpath(tempDir + QStringLiteral("/subdir"));
        
        // Create test files
        createFile(tempDir + QStringLiteral("/readme.txt"));
        createFile(tempDir + QStringLiteral("/notes.text"));
        createFile(tempDir + QStringLiteral("/doc.md"));
        createFile(tempDir + QStringLiteral("/model.scad"));
        createFile(tempDir + QStringLiteral("/output.csg"));
        createFile(tempDir + QStringLiteral("/config.json"));
        createFile(tempDir + QStringLiteral("/other.xyz"));
        createFile(tempDir + QStringLiteral("/subdir/nested.txt"));
        createFile(tempDir + QStringLiteral("/subdir/nested.scad"));
    }
    
    void TearDown() override {
        QDir(tempDir).removeRecursively();
    }
    
    void createFile(const QString& path) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "test";
            file.close();
        }
    }
};

TEST_F(FileTypeSearchTest, FindTextFiles) {
    QStringList files = filetypes::Text.findFiles(tempDir, false);
    EXPECT_EQ(files.size(), 2); // readme.txt, notes.text
}

TEST_F(FileTypeSearchTest, FindTextFilesRecursive) {
    QStringList files = filetypes::Text.findFiles(tempDir, true);
    EXPECT_EQ(files.size(), 3); // readme.txt, notes.text, subdir/nested.txt
}

TEST_F(FileTypeSearchTest, FindOpenSCADFiles) {
    QStringList files = filetypes::OpenSCAD.findFiles(tempDir, false);
    EXPECT_EQ(files.size(), 2); // model.scad, output.csg
}

TEST_F(FileTypeSearchTest, FindOpenSCADFilesRecursive) {
    QStringList files = filetypes::OpenSCAD.findFiles(tempDir, true);
    EXPECT_EQ(files.size(), 3); // model.scad, output.csg, subdir/nested.scad
}

TEST_F(FileTypeSearchTest, FindAllSupportedFiles) {
    QStringList files = findAllSupportedFiles(tempDir, false);
    EXPECT_EQ(files.size(), 6); // All except other.xyz
}

TEST_F(FileTypeSearchTest, FindAllSupportedFilesRecursive) {
    QStringList files = findAllSupportedFiles(tempDir, true);
    EXPECT_EQ(files.size(), 8); // All except other.xyz, including subdir files
}

TEST_F(FileTypeSearchTest, FindFilesNonExistentDirectory) {
    QStringList files = filetypes::Text.findFiles(tempDir + QStringLiteral("/nonexistent"), false);
    EXPECT_TRUE(files.isEmpty());
}

TEST_F(FileTypeSearchTest, FindFilesWithFilter) {
    QStringList files = filetypes::Text.findFiles(tempDir, true, 
        [](const QString& p) { return p.contains(QStringLiteral("nested")); });
    EXPECT_EQ(files.size(), 1); // Only nested.txt matches filter
}
