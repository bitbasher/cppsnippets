/**
 * @file editsubtype.h
 * @brief File subtype enumeration for supported file extensions
 */

#pragma once

#include "export.hpp"
#include <QString>

namespace scadtemplates {

/**
 * @brief Enumeration of supported file subtypes (extensions)
 * 
 * Each subtype represents a specific file extension that the editor supports.
 * Subtypes are grouped by their parent EditType.
 */
enum class EditSubtype {
    // Text subtypes
    Txt,    ///< .txt files
    Text,   ///< .text files
    Info,   ///< .info files
    Nfo,    ///< .nfo files
    
    // Markdown subtypes
    Md,     ///< .md files
    
    // OpenSCAD subtypes
    Scad,   ///< .scad files
    Csg,    ///< .csg files
    
    // JSON subtypes
    Json,   ///< .json files
    
    Unknown ///< Unknown/unsupported subtype
};

/**
 * @brief Get the file extension for a subtype (without dot)
 * @param subtype The subtype to query
 * @return The file extension string (e.g., "txt", "scad")
 */
inline QString getExtension(EditSubtype subtype) {
    switch (subtype) {
        case EditSubtype::Txt:     return QStringLiteral("txt");
        case EditSubtype::Text:    return QStringLiteral("text");
        case EditSubtype::Info:    return QStringLiteral("info");
        case EditSubtype::Nfo:     return QStringLiteral("nfo");
        case EditSubtype::Md:      return QStringLiteral("md");
        case EditSubtype::Scad:    return QStringLiteral("scad");
        case EditSubtype::Csg:     return QStringLiteral("csg");
        case EditSubtype::Json:    return QStringLiteral("json");
        case EditSubtype::Unknown:
        default:                   return QString();
    }
}

/**
 * @brief Get the display title for a subtype
 * @param subtype The subtype to query
 * @return Human-readable title (e.g., "Text File", "OpenSCAD File")
 */
inline QString getTitle(EditSubtype subtype) {
    switch (subtype) {
        case EditSubtype::Txt:
        case EditSubtype::Text:    return QStringLiteral("Text File");
        case EditSubtype::Info:    return QStringLiteral("Info File");
        case EditSubtype::Nfo:     return QStringLiteral("NFO File");
        case EditSubtype::Md:      return QStringLiteral("Markdown File");
        case EditSubtype::Scad:    return QStringLiteral("OpenSCAD File");
        case EditSubtype::Csg:     return QStringLiteral("CSG File");
        case EditSubtype::Json:    return QStringLiteral("JSON File");
        case EditSubtype::Unknown:
        default:                   return QStringLiteral("Unknown");
    }
}

/**
 * @brief Get the MIME type for a subtype
 * @param subtype The subtype to query
 * @return MIME type string (e.g., "text/plain", "application/json")
 */
inline QString getMimeType(EditSubtype subtype) {
    switch (subtype) {
        case EditSubtype::Txt:
        case EditSubtype::Text:
        case EditSubtype::Info:
        case EditSubtype::Nfo:     return QStringLiteral("text/plain");
        case EditSubtype::Md:      return QStringLiteral("text/markdown");
        case EditSubtype::Scad:
        case EditSubtype::Csg:     return QStringLiteral("application/x-openscad");
        case EditSubtype::Json:    return QStringLiteral("application/json");
        case EditSubtype::Unknown:
        default:                   return QStringLiteral("application/octet-stream");
    }
}

/**
 * @brief Get the filter pattern for a subtype (e.g., "*.txt")
 * @param subtype The subtype to query
 * @return Filter pattern string
 */
inline QString getFilterPattern(EditSubtype subtype) {
    QString ext = getExtension(subtype);
    if (ext.isEmpty()) {
        return QStringLiteral("*.*");
    }
    return QStringLiteral("*.") + ext;
}

/**
 * @brief Get the subtype from a file extension
 * @param extension File extension (with or without leading dot)
 * @return The matching EditSubtype, or EditSubtype::Unknown if not found
 */
SCADTEMPLATES_API EditSubtype subtypeFromExtension(const QString& extension);

} // namespace scadtemplates
