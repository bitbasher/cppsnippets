/**
 * @file editsubtype.h
 * @brief File subtype enumeration for supported file extensions
 */

#pragma once

#include "export.hpp"
#include <QString>
#include <QHash>

namespace scadtemplates {

/**
 * @brief Enumeration of supported file subtypes (extensions)
 * 
 * Each subtype represents a specific file extension that the editor supports.
 * Subtypes are grouped by their parent EditType.
 */
enum class EditSubtype {
    Unknown = 0, ///< Unknown/unsupported subtype

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
    Json   ///< .json files
};

/**
 * @brief Metadata for a file subtype
 */
struct SubtypeInfo {
    QString extension;  ///< File extension without dot (e.g., "txt")
    QString title;      ///< Display title (e.g., "Text File")
    QString mimeType;   ///< MIME type (e.g., "text/plain")
};

// Private implementation details
namespace detail {
    // Static table - single instance shared across all translation units
    inline const QHash<EditSubtype, SubtypeInfo> subtypeTable = {
        {EditSubtype::Unknown, {QString(), QString(), QString()}},
        {EditSubtype::Txt,  {QStringLiteral("txt"),  QStringLiteral("Text File"),     QStringLiteral("text/plain")}},
        {EditSubtype::Text, {QStringLiteral("text"), QStringLiteral("Text File"),     QStringLiteral("text/plain")}},
        {EditSubtype::Info, {QStringLiteral("info"), QStringLiteral("Info File"),     QStringLiteral("text/plain")}},
        {EditSubtype::Nfo,  {QStringLiteral("nfo"),  QStringLiteral("NFO File"),      QStringLiteral("text/plain")}},
        {EditSubtype::Md,   {QStringLiteral("md"),   QStringLiteral("Markdown File"), QStringLiteral("text/markdown")}},
        {EditSubtype::Scad, {QStringLiteral("scad"), QStringLiteral("OpenSCAD File"), QStringLiteral("application/x-openscad")}},
        {EditSubtype::Csg,  {QStringLiteral("csg"),  QStringLiteral("CSG File"),      QStringLiteral("application/x-openscad")}},
        {EditSubtype::Json, {QStringLiteral("json"), QStringLiteral("JSON File"),     QStringLiteral("application/json")}}
    };
    
    // Reverse lookup table: extension string -> EditSubtype
    inline const QHash<QString, EditSubtype> extensionToSubtype = {
        {QStringLiteral("txt"),  EditSubtype::Txt},
        {QStringLiteral("text"), EditSubtype::Text},
        {QStringLiteral("info"), EditSubtype::Info},
        {QStringLiteral("nfo"),  EditSubtype::Nfo},
        {QStringLiteral("md"),   EditSubtype::Md},
        {QStringLiteral("scad"), EditSubtype::Scad},
        {QStringLiteral("csg"),  EditSubtype::Csg},
        {QStringLiteral("json"), EditSubtype::Json}
    };
} // namespace detail

/**
 * @brief Get the metadata table for all subtypes
 * @return Static hash map of subtype metadata
 */
inline const QHash<EditSubtype, SubtypeInfo>& getSubtypeInfoTable() {
    return detail::subtypeTable;
}

/**
 * @brief Get the SubtypeInfo for a given subtype
 * @param subtype The subtype to query
 * @return SubtypeInfo (copy with implicit sharing, returns Unknown entry if not found)
 */
inline SubtypeInfo getSubtypeInfo(EditSubtype subtype) {
    return detail::subtypeTable.value(subtype, detail::subtypeTable.value(EditSubtype::Unknown));
}

/**
 * @brief Get the file extension for a subtype (without dot)
 * @param subtype The subtype to query
 * @return The file extension string (e.g., "txt", "scad"), or empty string for Unknown
 */
inline QString getExtension(EditSubtype subtype) {
    return getSubtypeInfo(subtype).extension;
}

/**
 * @brief Get the display title for a subtype
 * @param subtype The subtype to query
 * @return Human-readable title (e.g., "Text File", "OpenSCAD File"), or empty string for Unknown
 */
inline QString getTitle(EditSubtype subtype) {
    return getSubtypeInfo(subtype).title;
}

/**
 * @brief Get the MIME type for a subtype
 * @param subtype The subtype to query
 * @return MIME type string (e.g., "text/plain", "application/json"), or empty string for Unknown
 */
inline QString getMimeType(EditSubtype subtype) {
    return getSubtypeInfo(subtype).mimeType;
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
inline EditSubtype subtypeFromExtension(const QString& extension) {
    // Remove leading dot if present and convert to lowercase
    QString ext = (extension.startsWith('.') ? extension.mid(1) : extension).toLower();
    
    // O(1) hash lookup instead of O(n) iteration
    return detail::extensionToSubtype.value(ext, EditSubtype::Unknown);
}

} // namespace scadtemplates
