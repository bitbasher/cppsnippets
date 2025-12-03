/**
 * @file edittype.cpp
 * @brief Implementation of EditType utility functions
 */

#include <scadtemplates/edittype.h>

namespace scadtemplates {

QVector<EditSubtype> getSubtypes(EditType type) {
    switch (type) {
        case EditType::Text:
            return { EditSubtype::Txt, EditSubtype::Text, EditSubtype::Info, EditSubtype::Nfo };
        case EditType::Markdown:
            return { EditSubtype::Md };
        case EditType::OpenSCAD:
            return { EditSubtype::Scad, EditSubtype::Csg };
        case EditType::Json:
            return { EditSubtype::Json };
        case EditType::Unknown:
        default:
            return { EditSubtype::Unknown };
    }
}

QString getFileDialogFilter(EditType type) {
    QString result = getTitle(type) + QStringLiteral(" (");
    
    bool first = true;
    for (const auto& subtype : getSubtypes(type)) {
        if (!first) {
            result += QLatin1Char(' ');
        }
        result += getFilterPattern(subtype);
        first = false;
    }
    
    result += QLatin1Char(')');
    return result;
}

QString getAllFileDialogFilters() {
    QString result;
    QVector<EditType> allTypes = getAllTypes();
    
    // First, add "All Supported Files" entry
    result = QStringLiteral("All Supported Files (");
    bool firstExt = true;
    for (const auto& type : allTypes) {
        for (const auto& subtype : getSubtypes(type)) {
            if (!firstExt) {
                result += QLatin1Char(' ');
            }
            result += getFilterPattern(subtype);
            firstExt = false;
        }
    }
    result += QLatin1Char(')');
    
    // Then add individual type filters
    for (const auto& type : allTypes) {
        result += QStringLiteral(";;") + getFileDialogFilter(type);
    }
    
    // Add "All Files" at the end
    result += QStringLiteral(";;All Files (*.*)");
    
    return result;
}

QVector<EditType> getAllTypes() {
    return { EditType::Text, EditType::Markdown, EditType::OpenSCAD, EditType::Json };
}

} // namespace scadtemplates
