#pragma once
#include <QString>
#include "platformInfo/export.h"

namespace platformInfo {

class PLATFORMINFO_API ResourceLocation {
public:
    QString path;
    QString displayName;
    QString description;
    bool isEnabled = true;
    bool exists = false;
    bool isWritable = false;
    bool hasResourceFolders = false;  // true if path contains resource folders (examples, fonts, libraries, locale)

    ResourceLocation() = default;
    ResourceLocation(const QString& p, const QString& name, const QString& desc = QString())
        : path(p), displayName(name), description(desc) {}
    // Copy constructor
    ResourceLocation(const ResourceLocation& other) :
        path(other.path),
        displayName(other.displayName),
        description(other.description),
        isEnabled(other.isEnabled),
        exists(other.exists),
        isWritable(other.isWritable),
        hasResourceFolders(other.hasResourceFolders) {}

    bool operator==(const ResourceLocation& other) const { return path == other.path; }
};

} // namespace platformInfo
