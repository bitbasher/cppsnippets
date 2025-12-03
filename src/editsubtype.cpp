/**
 * @file editsubtype.cpp
 * @brief Implementation of EditSubtype utility functions
 */

#include <scadtemplates/editsubtype.h>

namespace scadtemplates {

EditSubtype subtypeFromExtension(const QString& extension) {
    QString ext = extension;
    
    // Remove leading dot if present
    if (ext.startsWith(QLatin1Char('.'))) {
        ext = ext.mid(1);
    }
    
    // Convert to lowercase for comparison
    ext = ext.toLower();
    
    // Match against known extensions
    if (ext == QLatin1String("txt"))  return EditSubtype::Txt;
    if (ext == QLatin1String("text")) return EditSubtype::Text;
    if (ext == QLatin1String("info")) return EditSubtype::Info;
    if (ext == QLatin1String("nfo"))  return EditSubtype::Nfo;
    if (ext == QLatin1String("md"))   return EditSubtype::Md;
    if (ext == QLatin1String("scad")) return EditSubtype::Scad;
    if (ext == QLatin1String("csg"))  return EditSubtype::Csg;
    if (ext == QLatin1String("json")) return EditSubtype::Json;
    
    return EditSubtype::Unknown;
}

} // namespace scadtemplates
