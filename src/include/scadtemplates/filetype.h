/**
 * @file filetype.h
 * @brief FileType class representing a category of files with subtypes
 */

#pragma once

#include "export.h"
#include "filesubtype.h"
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>

namespace scadtemplates {

/**
 * @brief Represents a file type category containing multiple subtypes
 * 
 * FileType groups related FileSubtype instances together (e.g., Text type
 * contains .txt, .text, .info, .nfo subtypes). It provides methods for
 * generating file dialog filters and searching the filesystem.
 */
class SCADTEMPLATES_API FileType {
public:
    /**
     * @brief Construct a FileType with subtypes
     * @param title Display title for the type (e.g., "Text Files")
     * @param mimeType Primary MIME type for the category
     * @param subtypes List of pointers to FileSubtype instances
     */
    FileType(const QString& title,
             const QString& mimeType,
             std::initializer_list<const FileSubtype*> subtypes);

    /// @brief Get the display title
    QString title() const noexcept { return m_title; }
    
    /// @brief Get the primary MIME type
    QString mimeType() const noexcept { return m_mimeType; }
    
    /// @brief Get the list of subtypes
    const QVector<const FileSubtype*>& subtypes() const noexcept { return m_subtypes; }
    
    /// @brief Get the number of subtypes
    qsizetype subtypeCount() const noexcept { return m_subtypes.size(); }
    
    /**
     * @brief Get file dialog filter string for this type
     * @return Filter string like "Text Files (*.txt *.text *.info *.nfo)"
     */
    QString fileDialogFilter() const;
    
    /**
     * @brief Get all glob patterns for this type
     * @return List of patterns like {"*.txt", "*.text", "*.info", "*.nfo"}
     */
    QStringList globPatterns() const;
    
    /**
     * @brief Get combined glob pattern string
     * @return Space-separated patterns like "*.txt *.text *.info *.nfo"
     */
    QString combinedGlobPattern() const;
    
    /**
     * @brief Check if a filename matches any subtype in this type
     * @param filename The filename to check
     * @return true if the file extension matches any subtype
     */
    bool matchesFilename(const QString& filename) const;
    
    /**
     * @brief Check if an extension matches any subtype in this type
     * @param extension Extension with or without leading dot
     * @return true if the extension matches any subtype
     */
    bool matchesExtension(const QString& extension) const;
    
    /**
     * @brief Find the specific subtype that matches an extension
     * @param extension Extension with or without leading dot
     * @return Pointer to matching FileSubtype, or nullptr
     */
    const FileSubtype* findSubtype(const QString& extension) const;
    
    /**
     * @brief Search a directory for files matching this type
     * @param directory Path to search
     * @param recursive Whether to search subdirectories
     * @return List of matching file paths
     */
    QStringList findFiles(
        const QString& directory,
        bool recursive = false) const;
    
    /**
     * @brief Search a directory for files matching this type (with filter)
     * @param directory Path to search
     * @param recursive Whether to search subdirectories  
     * @param filter Optional filter function for additional matching criteria
     * @return List of matching file paths
     */
    QStringList findFiles(
        const QString& directory,
        bool recursive,
        const std::function<bool(const QString&)>& filter) const;

private:
    QString m_title;
    QString m_mimeType;
    QVector<const FileSubtype*> m_subtypes;
};

// ============================================================================
// Predefined FileType instances
// ============================================================================
namespace filetypes {

/// @brief Text files (.txt, .text, .info, .nfo)
SCADTEMPLATES_API extern const FileType Text;

/// @brief Markdown files (.md)
SCADTEMPLATES_API extern const FileType Markdown;

/// @brief OpenSCAD files (.scad, .csg)
SCADTEMPLATES_API extern const FileType OpenSCAD;

/// @brief JSON files (.json)
SCADTEMPLATES_API extern const FileType Json;

} // namespace filetypes

/**
 * @brief Get all registered file types
 * @return Vector of pointers to all FileType instances
 */
SCADTEMPLATES_API QVector<const FileType*> getAllFileTypes();

/**
 * @brief Find a FileType by extension
 * @param extension File extension (with or without leading dot)
 * @return Pointer to matching FileType, or nullptr if not found
 */
SCADTEMPLATES_API const FileType* findFileTypeByExtension(const QString& extension);

/**
 * @brief Find a FileType that matches a filename
 * @param filename The filename to match
 * @return Pointer to matching FileType, or nullptr if not found
 */
SCADTEMPLATES_API const FileType* findFileTypeByFilename(const QString& filename);

/**
 * @brief Get combined file dialog filter for all types
 * @return Filter string with "All Supported Files" plus individual type filters
 */
SCADTEMPLATES_API QString getFileTypeDialogFilters();

/**
 * @brief Search a directory for all supported file types
 * @param directory Path to search
 * @param recursive Whether to search subdirectories
 * @return List of matching file paths
 */
SCADTEMPLATES_API QStringList findAllSupportedFiles(
    const QString& directory,
    bool recursive = false);

} // namespace scadtemplates
