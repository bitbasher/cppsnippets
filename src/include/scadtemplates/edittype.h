/**
 * @file edittype.h
 * @brief File type enumeration for supported editor file categories
 */

#pragma once

#include "export.h"
#include "editsubtype.h"
#include <QString>
#include <QVector>

namespace scadtemplates {

/**
 * @brief Enumeration of supported file types (categories)
 * 
 * Each type represents a category of files that share common characteristics.
 * Types contain one or more EditSubtype entries for their supported extensions.
 */
enum class EditType {
    Text,       ///< Plain text files (.txt, .text, .info, .nfo)
    Markdown,   ///< Markdown files (.md)
    OpenSCAD,   ///< OpenSCAD files (.scad, .csg)
    Json,       ///< JSON files (.json)
    Unknown     ///< Unknown/unsupported type
};

/**
 * @brief Get the display title for a type
 * @param type The type to query
 * @return Human-readable title (e.g., "Text Files", "OpenSCAD Files")
 */
inline QString getTitle(EditType type) {
    switch (type) {
        case EditType::Text:     return QStringLiteral("Text Files");
        case EditType::Markdown: return QStringLiteral("Markdown Files");
        case EditType::OpenSCAD: return QStringLiteral("OpenSCAD Files");
        case EditType::Json:     return QStringLiteral("JSON Files");
        case EditType::Unknown:
        default:                 return QStringLiteral("Unknown");
    }
}

/**
 * @brief Get the primary MIME type for a type
 * @param type The type to query
 * @return MIME type string (e.g., "text/plain", "application/json")
 */
inline QString getMimeType(EditType type) {
    switch (type) {
        case EditType::Text:     return QStringLiteral("text/plain");
        case EditType::Markdown: return QStringLiteral("text/markdown");
        case EditType::OpenSCAD: return QStringLiteral("application/x-openscad");
        case EditType::Json:     return QStringLiteral("application/json");
        case EditType::Unknown:
        default:                 return QStringLiteral("application/octet-stream");
    }
}

/**
 * @brief Get the type from a subtype
 * @param subtype The subtype to query
 * @return The parent EditType for this subtype
 */
inline constexpr EditType typeFromSubtype(EditSubtype subtype) {
    switch (subtype) {
        case EditSubtype::Txt:
        case EditSubtype::Text:
        case EditSubtype::Info:
        case EditSubtype::Nfo:
            return EditType::Text;
        
        case EditSubtype::Md:
            return EditType::Markdown;
        
        case EditSubtype::Scad:
        case EditSubtype::Csg:
            return EditType::OpenSCAD;
        
        case EditSubtype::Json:
            return EditType::Json;
        
        case EditSubtype::Unknown:
        default:
            return EditType::Unknown;
    }
}

/**
 * @brief Get the type from a file extension
 * @param extension File extension (with or without leading dot)
 * @return The matching EditType, or EditType::Unknown if not found
 */
inline EditType typeFromExtension(const QString& extension) {
    return typeFromSubtype(subtypeFromExtension(extension));
}

/**
 * @brief Get the subtypes associated with a type
 * @param type The type to query
 * @return Vector of EditSubtype values for this type
 */
SCADTEMPLATES_API QVector<EditSubtype> getSubtypes(EditType type);

/**
 * @brief Get a file dialog filter string for a type
 * 
 * Returns a string suitable for use in file open/save dialogs.
 * Format: "Title (*.ext1 *.ext2)"
 * 
 * @param type The type to generate filter for
 * @return Filter string for file dialogs
 */
SCADTEMPLATES_API QString getFileDialogFilter(EditType type);

/**
 * @brief Get a file dialog filter string for all supported types
 * 
 * Returns a combined filter string with all supported file types.
 * Includes an "All Supported Files" entry followed by individual type filters.
 * 
 * @return Combined filter string for file dialogs
 */
SCADTEMPLATES_API QString getAllFileDialogFilters();

/**
 * @brief Get all supported EditType values (excluding Unknown)
 * @return Vector of all valid EditType values
 */
SCADTEMPLATES_API QVector<EditType> getAllTypes();

} // namespace scadtemplates
