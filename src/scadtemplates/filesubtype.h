/**
 * @file filesubtype.h
 * @brief FileSubtype class representing a file extension with metadata
 */

#pragma once

#include "export.h"
#include <QString>

namespace scadtemplates {

/**
 * @brief Represents a file subtype (extension) with associated metadata
 * 
 * FileSubtype is an immutable class that holds information about a specific
 * file extension including its display title and MIME type. Instances are
 * created as const objects and grouped under FileType parents.
 */
class SCADTEMPLATES_API FileSubtype {
public:
    /**
     * @brief Construct a FileSubtype with all metadata
     * @param extension File extension without dot (e.g., "txt", "scad")
     * @param title Human-readable title (e.g., "Text File")
     * @param mimeType MIME type string (e.g., "text/plain")
     */
    FileSubtype(const QString& extension, 
                const QString& title,
                const QString& mimeType) noexcept
        : m_extension(extension)
        , m_title(title)
        , m_mimeType(mimeType)
    {}

    /// @brief Get the file extension (without dot)
    QString extension() const noexcept { return m_extension; }
    
    /// @brief Get the display title
    QString title() const noexcept { return m_title; }
    
    /// @brief Get the MIME type
    QString mimeType() const noexcept { return m_mimeType; }
    
    /// @brief Get the extension with dot prefix (e.g., ".txt")
    QString dotExtension() const { return QLatin1Char('.') + m_extension; }
    
    /// @brief Get the glob pattern for this subtype (e.g., "*.txt")
    QString globPattern() const { return QStringLiteral("*.") + m_extension; }
    
    /// @brief Check if a filename matches this subtype (case-insensitive)
    bool matchesFilename(const QString& filename) const;
    
    /// @brief Check if an extension matches this subtype (case-insensitive, with or without dot)
    bool matchesExtension(const QString& ext) const;

private:
    QString m_extension;
    QString m_title;
    QString m_mimeType;
};

// ============================================================================
// Predefined FileSubtype instances for Text files
// ============================================================================
namespace subtypes {

/// @brief .txt text files
SCADTEMPLATES_API extern const FileSubtype Txt;

/// @brief .text text files  
SCADTEMPLATES_API extern const FileSubtype Text;

/// @brief .info information files
SCADTEMPLATES_API extern const FileSubtype Info;

/// @brief .nfo NFO files
SCADTEMPLATES_API extern const FileSubtype Nfo;

/// @brief .md Markdown files
SCADTEMPLATES_API extern const FileSubtype Md;

/// @brief .scad OpenSCAD source files
SCADTEMPLATES_API extern const FileSubtype Scad;

/// @brief .csg CSG (Constructive Solid Geometry) files
SCADTEMPLATES_API extern const FileSubtype Csg;

/// @brief .json JSON files
SCADTEMPLATES_API extern const FileSubtype Json;

} // namespace subtypes

/**
 * @brief Find a FileSubtype by extension
 * @param extension File extension (with or without leading dot)
 * @return Pointer to matching FileSubtype, or nullptr if not found
 */
SCADTEMPLATES_API const FileSubtype* findSubtypeByExtension(const QString& extension);

} // namespace scadtemplates
