#include "resInventory/ResourceLocation.h"
#include "platformInfo/resourcePaths.h"
#include <QFileInfo>

namespace platformInfo {

QString ResourceLocation::expandedPath(const ResourcePaths& paths) const
{
    // If no raw template, return existing path
    if (m_rawPath.isEmpty()) {
        return path;
    }
    
    // Expand env vars from template
    QString expanded = paths.expandEnvVars(m_rawPath);
    
    // Canonicalize to absolute path (resolves .., symlinks, etc.)
    QFileInfo fileInfo(expanded);
    QString canonical = fileInfo.canonicalFilePath();
    
    // If canonicalFilePath returns empty (path doesn't exist), use absoluteFilePath
    return canonical.isEmpty() ? fileInfo.absoluteFilePath() : canonical;
}

} // namespace platformInfo
