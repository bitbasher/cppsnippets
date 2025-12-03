/**
 * @file filesubtype.cpp
 * @brief Implementation of FileSubtype class methods
 */

#include <scadtemplates/filesubtype.h>
#include <QFileInfo>

namespace scadtemplates {

namespace {

// Extract extension from filename
QString extractExtension(const QString& filename) {
    int dotPos = filename.lastIndexOf(QLatin1Char('.'));
    if (dotPos == -1 || dotPos == filename.size() - 1) {
        return QString();
    }
    return filename.mid(dotPos + 1);
}

} // anonymous namespace

bool FileSubtype::matchesFilename(const QString& filename) const {
    QString ext = extractExtension(filename);
    return !ext.isEmpty() && ext.compare(m_extension, Qt::CaseInsensitive) == 0;
}

bool FileSubtype::matchesExtension(const QString& ext) const {
    QString extension = ext;
    // Remove leading dot if present
    if (extension.startsWith(QLatin1Char('.'))) {
        extension = extension.mid(1);
    }
    return extension.compare(m_extension, Qt::CaseInsensitive) == 0;
}

// ============================================================================
// Predefined FileSubtype instances
// ============================================================================
namespace subtypes {

const FileSubtype Txt{QStringLiteral("txt"), QStringLiteral("Text File"), QStringLiteral("text/plain")};
const FileSubtype Text{QStringLiteral("text"), QStringLiteral("Text File"), QStringLiteral("text/plain")};
const FileSubtype Info{QStringLiteral("info"), QStringLiteral("Info File"), QStringLiteral("text/plain")};
const FileSubtype Nfo{QStringLiteral("nfo"), QStringLiteral("NFO File"), QStringLiteral("text/plain")};
const FileSubtype Md{QStringLiteral("md"), QStringLiteral("Markdown File"), QStringLiteral("text/markdown")};
const FileSubtype Scad{QStringLiteral("scad"), QStringLiteral("OpenSCAD File"), QStringLiteral("application/x-openscad")};
const FileSubtype Csg{QStringLiteral("csg"), QStringLiteral("CSG File"), QStringLiteral("application/x-openscad")};
const FileSubtype Json{QStringLiteral("json"), QStringLiteral("JSON File"), QStringLiteral("application/json")};

} // namespace subtypes

const FileSubtype* findSubtypeByExtension(const QString& extension) {
    QString ext = extension;
    // Remove leading dot if present
    if (ext.startsWith(QLatin1Char('.'))) {
        ext = ext.mid(1);
    }
    
    // Check all known subtypes
    static const FileSubtype* allSubtypes[] = {
        &subtypes::Txt,
        &subtypes::Text,
        &subtypes::Info,
        &subtypes::Nfo,
        &subtypes::Md,
        &subtypes::Scad,
        &subtypes::Csg,
        &subtypes::Json
    };
    
    for (const auto* subtype : allSubtypes) {
        if (subtype->matchesExtension(ext)) {
            return subtype;
        }
    }
    return nullptr;
}

} // namespace scadtemplates
