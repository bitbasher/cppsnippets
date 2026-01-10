/**
 * @file ResourceLocation.cpp
 * @brief Implementation of ResourceLocation class
 */

#include "ResourceLocation.hpp"
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace platformInfo {

// Default constructor - displayName will be empty until path is set
ResourceLocation::ResourceLocation()
    : m_path()
    , m_displayName()
    , m_description()
    , m_isEnabled(true)
    , m_exists(false)
    , m_isWritable(false)
    , m_hasResourceFolders(false)
{}

// Path-only constructor - generates display name from path
ResourceLocation::ResourceLocation(const QString& p, const QString& name, const QString& desc)
    : m_path(p)
    , m_displayName(name.isEmpty() ? generateDisplayName(p) : name)
    , m_description(desc)
    , m_isEnabled(true)
    , m_exists(false)
    , m_isWritable(false)
    , m_hasResourceFolders(false)
{}

// Copy constructor
ResourceLocation::ResourceLocation(const ResourceLocation& other)
    : m_path(other.m_path)
    , m_displayName(other.m_displayName)
    , m_description(other.m_description)
    , m_isEnabled(other.m_isEnabled)
    , m_exists(other.m_exists)
    , m_isWritable(other.m_isWritable)
    , m_hasResourceFolders(other.m_hasResourceFolders)
{}

QString ResourceLocation::generateDisplayName(const QString& absolutePath)
{
    // Validate input path
    if (absolutePath.isEmpty()) {
        return QString();
    }
    
    // Reject paths with environment variable placeholders
    if (absolutePath.contains("&&") || 
        absolutePath.contains("$env:", Qt::CaseInsensitive) ||
        absolutePath.contains("${") ||
        absolutePath.contains("%")) {
        qWarning() << "generateDisplayName: Path contains environment variable placeholders:" << absolutePath;
        return absolutePath;  // Return as-is if invalid
    }
    
    // Check if path is absolute
    QFileInfo pathInfo(absolutePath);
    if (!pathInfo.isAbsolute()) {
        qWarning() << "generateDisplayName: Path is not absolute:" << absolutePath;
        return absolutePath;  // Return as-is if not absolute
    }
    
    // Get canonical path (resolves symlinks and . / .. components)
    QString canonicalPath = pathInfo.canonicalFilePath();
    if (canonicalPath.isEmpty()) {
        // Path doesn't exist, use cleaned absolute path
        canonicalPath = QDir::cleanPath(pathInfo.absoluteFilePath());
    }
    
    // Check if path is short enough to display as-is
    const int shortPathThreshold = 24;
    if (canonicalPath.length() < shortPathThreshold) {
        return canonicalPath;
    }
    
    // Replace home directory with tilde
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if (!homeDir.isEmpty() && canonicalPath.startsWith(homeDir)) {
        QString withTilde = "~" + canonicalPath.mid(homeDir.length());
        if (withTilde.length() < shortPathThreshold) {
            return withTilde;
        }
        // Continue with tilde version for truncation
        canonicalPath = withTilde;
    }
    
    // Check drive root (already minimal)
    QDir dir(canonicalPath);
    if (dir.isRoot()) {
        return canonicalPath;  // "C:", "/", etc.
    }
    
    // Get configured maximum display length
    int maxLength = getMaxDisplayLength();
    
    // If still too long, truncate with ellipsis
    if (canonicalPath.length() > maxLength) {
        // Show beginning and end of path
        int keepChars = (maxLength - 3) / 2;  // Leave room for "..."
        return canonicalPath.left(keepChars) + "..." + canonicalPath.right(keepChars);
    }
    
    return canonicalPath;
}

QString ResourceLocation::displayName() const
{
    // If displayName is empty and we have a path, generate it
    if (m_displayName.isEmpty() && !m_path.isEmpty()) {
        return generateDisplayName(m_path);
    }
    return m_displayName;
}

void ResourceLocation::setDisplayName(const QString& name)
{
    m_displayName = name;
}

void ResourceLocation::setPath(const QString& p)
{
    m_path = p;
    // Regenerate display name when path changes
    m_displayName = generateDisplayName(m_path);
}

int ResourceLocation::getMaxDisplayLength()
{
    QSettings settings(QStringLiteral("ScadTemplates"), QStringLiteral("ResourcePaths"));
    return settings.value(QStringLiteral("max_display_name_length"), 60).toInt();
}

} // namespace platformInfo
