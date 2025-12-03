/**
 * @file filetype.cpp
 * @brief Implementation of FileType class methods
 */

#include <scadtemplates/filetype.h>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

namespace scadtemplates {

// ============================================================================
// FileType implementation
// ============================================================================

FileType::FileType(const QString& title,
                   const QString& mimeType,
                   std::initializer_list<const FileSubtype*> subtypes)
    : m_title(title)
    , m_mimeType(mimeType)
    , m_subtypes(subtypes)
{
}

QString FileType::fileDialogFilter() const {
    QString result = m_title + QStringLiteral(" (");
    
    bool first = true;
    for (const auto* subtype : m_subtypes) {
        if (!first) result += QLatin1Char(' ');
        result += subtype->globPattern();
        first = false;
    }
    
    result += QLatin1Char(')');
    return result;
}

QStringList FileType::globPatterns() const {
    QStringList patterns;
    patterns.reserve(m_subtypes.size());
    for (const auto* subtype : m_subtypes) {
        patterns.append(subtype->globPattern());
    }
    return patterns;
}

QString FileType::combinedGlobPattern() const {
    QString result;
    bool first = true;
    for (const auto* subtype : m_subtypes) {
        if (!first) result += QLatin1Char(' ');
        result += subtype->globPattern();
        first = false;
    }
    return result;
}

bool FileType::matchesFilename(const QString& filename) const {
    for (const auto* subtype : m_subtypes) {
        if (subtype->matchesFilename(filename)) {
            return true;
        }
    }
    return false;
}

bool FileType::matchesExtension(const QString& extension) const {
    for (const auto* subtype : m_subtypes) {
        if (subtype->matchesExtension(extension)) {
            return true;
        }
    }
    return false;
}

const FileSubtype* FileType::findSubtype(const QString& extension) const {
    for (const auto* subtype : m_subtypes) {
        if (subtype->matchesExtension(extension)) {
            return subtype;
        }
    }
    return nullptr;
}

QStringList FileType::findFiles(
    const QString& directory,
    bool recursive) const
{
    return findFiles(directory, recursive, nullptr);
}

QStringList FileType::findFiles(
    const QString& directory,
    bool recursive,
    const std::function<bool(const QString&)>& filter) const
{
    QStringList results;
    
    QDir dir(directory);
    if (!dir.exists()) {
        return results;
    }
    
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;
    if (recursive) {
        flags |= QDirIterator::Subdirectories;
    }
    
    QDirIterator it(directory, QDir::Files, flags);
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        QString ext = fileInfo.suffix();
        
        if (matchesExtension(ext)) {
            if (!filter || filter(filePath)) {
                results.append(filePath);
            }
        }
    }
    
    return results;
}

// ============================================================================
// Predefined FileType instances
// ============================================================================
namespace filetypes {

const FileType Text{
    QStringLiteral("Text Files"),
    QStringLiteral("text/plain"),
    {&subtypes::Txt, &subtypes::Text, &subtypes::Info, &subtypes::Nfo}
};

const FileType Markdown{
    QStringLiteral("Markdown Files"),
    QStringLiteral("text/markdown"),
    {&subtypes::Md}
};

const FileType OpenSCAD{
    QStringLiteral("OpenSCAD Files"),
    QStringLiteral("application/x-openscad"),
    {&subtypes::Scad, &subtypes::Csg}
};

const FileType Json{
    QStringLiteral("JSON Files"),
    QStringLiteral("application/json"),
    {&subtypes::Json}
};

} // namespace filetypes

// ============================================================================
// Global functions
// ============================================================================

QVector<const FileType*> getAllFileTypes() {
    return {
        &filetypes::Text,
        &filetypes::Markdown,
        &filetypes::OpenSCAD,
        &filetypes::Json
    };
}

const FileType* findFileTypeByExtension(const QString& extension) {
    auto allTypes = getAllFileTypes();
    for (const auto* type : allTypes) {
        if (type->matchesExtension(extension)) {
            return type;
        }
    }
    return nullptr;
}

const FileType* findFileTypeByFilename(const QString& filename) {
    auto allTypes = getAllFileTypes();
    for (const auto* type : allTypes) {
        if (type->matchesFilename(filename)) {
            return type;
        }
    }
    return nullptr;
}

QString getFileTypeDialogFilters() {
    QString result;
    auto allTypes = getAllFileTypes();
    
    // First entry: All Supported Files
    result = QStringLiteral("All Supported Files (");
    bool firstExt = true;
    for (const auto* type : allTypes) {
        for (const auto* subtype : type->subtypes()) {
            if (!firstExt) result += QLatin1Char(' ');
            result += subtype->globPattern();
            firstExt = false;
        }
    }
    result += QLatin1Char(')');
    
    // Individual type filters
    for (const auto* type : allTypes) {
        result += QStringLiteral(";;") + type->fileDialogFilter();
    }
    
    // All Files at the end
    result += QStringLiteral(";;All Files (*.*)");
    
    return result;
}

QStringList findAllSupportedFiles(
    const QString& directory,
    bool recursive)
{
    QStringList results;
    
    QDir dir(directory);
    if (!dir.exists()) {
        return results;
    }
    
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;
    if (recursive) {
        flags |= QDirIterator::Subdirectories;
    }
    
    QDirIterator it(directory, QDir::Files, flags);
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        QString ext = fileInfo.suffix();
        
        if (findFileTypeByExtension(ext) != nullptr) {
            results.append(filePath);
        }
    }
    
    return results;
}

} // namespace scadtemplates
