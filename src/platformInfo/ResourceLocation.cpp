/**
 * @file ResourceLocation.cpp
 * @brief Implementation of ResourceLocation class
 */

#include "ResourceLocation.hpp"
#include "resourceInventory/resourceItem.hpp"
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace platformInfo {

// Default constructor
ResourceLocation::ResourceLocation()
    : m_path()
    , m_rawPath()
    , m_description()
    , m_tier(resourceInventory::ResourceTier::User)
    , m_isEnabled(true)
    , m_exists(false)
    , m_isWritable(false)
    , m_hasResourceFolders(false)
{}

// Path constructor
ResourceLocation::ResourceLocation(const QString& p, ResourceTier tier, const QString& rawPath, const QString& name, const QString& desc)
    : m_path(p)
    , m_rawPath(rawPath.isEmpty() ? p : rawPath)
    , m_description(desc)
    , m_tier(tier)
    , m_isEnabled(true)
    , m_exists(false)
    , m_isWritable(false)
    , m_hasResourceFolders(false)
{}

// Copy constructor
ResourceLocation::ResourceLocation(const ResourceLocation& other)
    : m_path(other.m_path)
    , m_rawPath(other.m_rawPath)
    , m_description(other.m_description)
    , m_tier(other.m_tier)
    , m_isEnabled(other.m_isEnabled)
    , m_exists(other.m_exists)
    , m_isWritable(other.m_isWritable)
    , m_hasResourceFolders(other.m_hasResourceFolders)
{}

QString ResourceLocation::getDisplayName() const
{
    // Validate input path
    if (m_path.isEmpty()) {
        return QString();
    }
    
    // Keep paths with environment variable placeholders as-is - they're unique identifiers
    if (m_path.contains("&&") || 
        m_path.contains("$env:", Qt::CaseInsensitive) ||
        m_path.contains("${") ||
        m_path.contains("%")) {
        return m_path;  // Use env var name as display name
    }
    
    // Check if path is absolute
    QFileInfo pathInfo(m_path);
    if (!pathInfo.isAbsolute()) {
        qWarning() << "getDisplayName: Path is not absolute:" << m_path;
        return m_path;  // Return as-is if not absolute
    }
    
    // Get canonical path (resolves symlinks and . / .. components)
    QString canonicalPath = pathInfo.canonicalFilePath();
    if (canonicalPath.isEmpty()) {
        // Path doesn't exist, use cleaned absolute path
        canonicalPath = QDir::cleanPath(pathInfo.absoluteFilePath());
    }
    
    // Safety check - ensure canonical path is valid
    if (canonicalPath.isNull() || canonicalPath.length() > 32767) {
        qWarning() << "getDisplayName: Invalid canonical path for:" << m_path;
        return m_path;
    }
    
    // Replace home directory with tilde (do this early as it may shorten the path enough)
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if (!homeDir.isEmpty() && canonicalPath.startsWith(homeDir)) {
        canonicalPath = "~" + canonicalPath.mid(homeDir.length());
    }
    
    // Check drive root (already minimal)
    QDir dir(canonicalPath);
    if (dir.isRoot()) {
        return canonicalPath;  // "C:", "/", etc.
    }
    
    // Get configured maximum display length
    int maxLength = getMaxDisplayLength();
    
    // Validate maxLength to prevent assertion failures
    if (maxLength < 10) {
        qWarning() << "getDisplayName: Invalid maxLength" << maxLength << "using default 60";
        maxLength = 60;
    }
    
    // If path is short enough, display as-is
    if (canonicalPath.length() <= maxLength) {
        return canonicalPath;
    }
    
    // Path is too long - truncate with ellipsis
    // Show beginning and end of path
    int available = maxLength - 3;  // Reserve 3 chars for "..."
    int keepLeft = available / 2;
    int keepRight = available - keepLeft;  // Automatically gets remainder when odd
    
    return canonicalPath.left(keepLeft) + "..." + canonicalPath.right(keepRight);
}

int ResourceLocation::getMaxDisplayLength()
{
    QSettings settings(QStringLiteral("ScadTemplates"), QStringLiteral("ResourcePaths"));
    return settings.value(QStringLiteral("max_display_name_length"), 60).toInt();
}

int ResourceLocation::getMinDisplayLength()
{
    QSettings settings(QStringLiteral("ScadTemplates"), QStringLiteral("ResourcePaths"));
    return settings.value(QStringLiteral("min_display_name_length"), 24).toInt();
}

} // namespace platformInfo
