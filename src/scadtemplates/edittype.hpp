/**
 * @file edittype.h
 * @brief File type enumeration for supported editor file categories
 */

#pragma once

#include "export.hpp"
#include "editsubtype.hpp"
#include <QString>
#include <QHash>
#include <QList>

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
 * @brief Metadata for an EditType
 */
struct TypeInfo {
    QString title;                  ///< Display title (e.g., "Text Files")
    QString mimeType;               ///< Primary MIME type (e.g., "text/plain")
    QList<EditSubtype> subtypes;    ///< List of subtypes in this category
    QString fileDialogFilter;       ///< Pre-built filter string for QFileDialog
};

// Private implementation details
namespace detail {
    // Static table - single instance shared across all translation units
    inline const QHash<EditType, TypeInfo> typeTable = {
        {EditType::Unknown, {QString(), QString(), {EditSubtype::Unknown}, QString()}},
        {EditType::Text,     {QStringLiteral("Text Files"),     QStringLiteral("text/plain"),
            {EditSubtype::Txt, EditSubtype::Text, EditSubtype::Info, EditSubtype::Nfo}, 
            QStringLiteral("Text Files (*.txt *.text *.info *.nfo)")}
        },
        {EditType::Markdown, {QStringLiteral("Markdown Files"), QStringLiteral("text/markdown"),
            {EditSubtype::Md},
            QStringLiteral("Markdown Files (*.md)")}
        },
        {EditType::OpenSCAD, {QStringLiteral("OpenSCAD Files"), QStringLiteral("application/x-openscad"),
            {EditSubtype::Scad, EditSubtype::Csg},
            QStringLiteral("OpenSCAD Files (*.scad *.csg)")}
        },
        {EditType::Json,     {QStringLiteral("JSON Files"),     QStringLiteral("application/json"),
            {EditSubtype::Json},
            QStringLiteral("JSON Files (*.json)")}
        }
    };
} // namespace detail

/**
 * @brief Get complete info for an EditType
 * @param type The type to query
 * @return TypeInfo struct with title and MIME type
 */
inline TypeInfo getTypeInfo(EditType type) {
    return detail::typeTable.value(type, 
        detail::typeTable.value(EditType::Unknown));
}

/**
 * @brief Get the display title for a type
 * @param type The type to query
 * @return Human-readable title (e.g., "Text Files", "OpenSCAD Files")
 */
inline QString getTitle(EditType type) {
    return getTypeInfo(type).title;
}

/**
 * @brief Get the primary MIME type for a type
 * @param type The type to query
 * @return MIME type string (e.g., "text/plain", "application/json")
 */
inline QString getMimeType(EditType type) {
    return getTypeInfo(type).mimeType;
}

/**
 * @brief Get the type from a subtype
 * @param subtype The subtype to query
 * @return The parent EditType for this subtype
 */
inline EditType typeFromSubtype(EditSubtype subtype) {
    // Local reverse lookup table - only used by this function
    static const QHash<EditSubtype, EditType> subtypeToType = {
        {EditSubtype::Txt,  EditType::Text},
        {EditSubtype::Text, EditType::Text},
        {EditSubtype::Info, EditType::Text},
        {EditSubtype::Nfo,  EditType::Text},
        {EditSubtype::Md,   EditType::Markdown},
        {EditSubtype::Scad, EditType::OpenSCAD},
        {EditSubtype::Csg,  EditType::OpenSCAD},
        {EditSubtype::Json, EditType::Json}
    };
    
    return subtypeToType.value(subtype, EditType::Unknown);
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
 * @return List of EditSubtype values for this category
 */
inline QList<EditSubtype> getSubtypes(EditType type) {
    return getTypeInfo(type).subtypes;
}

/**
 * @brief Get a file dialog filter string for a type
 * 
 * Returns a string suitable for use in file open/save dialogs.
 * Format: "Title (*.ext1 *.ext2)"
 * 
 * @param type The type to generate filter for
 * @return Filter string for file dialogs
 */
inline QString getFileDialogFilter(EditType type) {
    return getTypeInfo(type).fileDialogFilter;
}

/**
 * @brief Get all supported EditType values (excluding Unknown)
 * @return List of all valid EditType values
 */
inline QList<EditType> getAllTypes() {
    return { EditType::Text, EditType::Markdown, EditType::OpenSCAD, EditType::Json };
}

/**
 * @brief Get a file dialog filter string for all supported types
 * 
 * Returns a combined filter string with all supported file types.
 * Includes an "All Supported Files" entry followed by individual type filters.
 * 
 * @return Combined filter string for file dialogs
 */
inline QString getAllFileDialogFilters() {
    // Build "All Supported Files" entry by collecting all patterns
    QStringList allPatterns;
    QList<EditType> allTypes = getAllTypes();
    
    for (const auto& type : allTypes) {
        for (const auto& subtype : getSubtypes(type)) {
            allPatterns << getFilterPattern(subtype);
        }
    }
    
    QString result = QStringLiteral("All Supported Files (") + allPatterns.join(' ') + ')';
    
    // Append individual type filters (pre-built strings from table)
    for (const auto& type : allTypes) {
        result += QStringLiteral(";;") + getFileDialogFilter(type);
    }
    
    // Add "All Files" at the end
    result += QStringLiteral(";;All Files (*.*)");
    
    return result;
}

} // namespace scadtemplates
