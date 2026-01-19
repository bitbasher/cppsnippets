/**
 * @file ResourceLocation.cpp
 * @brief Implementation of ResourceLocation class
 */

#include "ResourceLocation.hpp"
#include "resourceInventory/resourceItem.hpp"

#include <algorithm>

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QStringView>
#include <QDebug>
#include <QRegularExpressionMatch>

namespace platformInfo {

// Static member initialization
int ResourceLocation::s_nextIndex = 1000;
QHash<QString, QString> ResourceLocation::s_pathToIndex;
QHash<QString, QString> ResourceLocation::s_indexToPath;

// Default constructor
ResourceLocation::ResourceLocation()
    : m_path()
    , m_rawPath()
    , m_description()
    , m_tier(ResourceTier::User)
{
    m_index = s_nextIndex++;
    m_indexString = QString::number(m_index).rightJustified(4, '0');
    m_uniqueID = QString("loc-%1").arg(m_indexString);
}

// Path constructor
ResourceLocation::ResourceLocation(const QString& p, ResourceTier tier, const QString& rawPath, const QString& name, const QString& desc)
    : m_path(p)
    , m_rawPath(rawPath.isEmpty() ? p : rawPath)
    , m_description(desc)
    , m_tier(tier)
{
    // Normalize path for consistent indexing (forward slashes)
    QString normalizedPath = QString(p).replace('\\', '/');
    
    // Create compound key: path + tier (allows same path in different tiers)
    QString tierName = resourceMetadata::tierToString(tier);
    QString compoundKey = QString("%1|%2").arg(normalizedPath, tierName);
    
    // Check if this (path,tier) combination already has an index assigned
    if (s_pathToIndex.contains(compoundKey)) {
        m_indexString = s_pathToIndex.value(compoundKey);
        m_index = m_indexString.toInt();
    } else {
        m_index = s_nextIndex++;
        m_indexString = QString::number(m_index).rightJustified(4, '0');
        s_pathToIndex.insert(compoundKey, m_indexString);
        s_indexToPath.insert(m_indexString, normalizedPath);  // Still map index -> path for reverse lookup
    }
    
    m_uniqueID = QString("loc-%1").arg(m_indexString);
}

// Copy constructor
ResourceLocation::ResourceLocation(const ResourceLocation& other)
    : m_path(other.m_path)
    , m_rawPath(other.m_rawPath)
    , m_description(other.m_description)
    , m_tier(other.m_tier)
    , m_index(other.m_index)
    , m_indexString(other.m_indexString)
    , m_uniqueID(other.m_uniqueID)
{}

QString ResourceLocation::getDisplayName() const
{
    // Check drive root - the root is going to be the shortest possible unique path
    // so we can skip all other checks including length checks
    QDir dir(m_path);
    if (dir.isRoot()) {
        return m_path;
    }
     
    // Keep paths with environment variable placeholders as-is - they're unique identifiers
    // Matches: ${VARNAME}, $VARNAME, %VARNAME%, or PowerShell $env:VARNAME
    // QRegularExpression envVarRegex(R"(\$\{[a-zA-Z_][a-zA-Z0-9_]*\}|\$[a-zA-Z_][a-zA-Z0-9_]*|%[a-zA-Z_][a-zA-Z0-9_]*%)");
    QRegularExpression matedBraces(R"(\$\{[A-Z_][A-Z0-9_]*\})");
    QRegularExpression leadingDollar(R"(\$[A-Z_][A-Z0-9_]*)");
    QRegularExpression leadingEnv(R"(\$[eE][nN][vV]:(\{[A-Z_][A-Z0-9_]*\}|[A-Z_][A-Z0-9_]*))");
    QRegularExpression percentages(R"(%[A-Z_][A-Z0-9_]*%)");

    // Keep paths with environment variable placeholders as-is - they're unique identifiers
    QRegularExpressionMatch match;
    QString envVarString;
    
    match = leadingEnv.match(m_rawPath);
    if (match.hasMatch()) {
        envVarString = QStringLiteral("$") + match.captured(1);
    } else {
        match = matedBraces.match(m_rawPath);
        if (match.hasMatch()) {
            envVarString = match.captured(0);
        } else {
            match = leadingDollar.match(m_rawPath);
            if (match.hasMatch()) {
                envVarString = match.captured(0);
            } else {
                match = percentages.match(m_rawPath);
                if (match.hasMatch()) {
                    envVarString = match.captured(0);
                }
            }
        }
    }
    
    QString candidateName;
    if (match.hasMatch())
        candidateName = envVarString;
    else {
        // Get canonical path (resolves symlinks and . / .. components)
        QString canonicalPath = QFileInfo(m_path).canonicalFilePath();
        if (canonicalPath.isEmpty()) {
            canonicalPath = m_path;  // Fallback if path doesn't exist
        }
        
        // Replace home directory with tilde (do this early as it may shorten the path enough)
        QString homeDir = QDir::homePath(); // homeDir cannot be empty
        if ( canonicalPath.startsWith(homeDir)) {
            canonicalPath = "~" + canonicalPath.mid(homeDir.length());
        }
        candidateName = canonicalPath;
    }

    // ensure that the max display length is within reasonable bounds
    int maxLength = std::max(20, std::min( getMaxDisplayLength(), 60));
    
    // If path is short enough, display as-is
    if (candidateName.length() <= maxLength) {
        return candidateName;
    }
    
    // Path is too long - truncate with ellipsis
    // Show beginning and end of path
    int available = maxLength - 3;  // Reserve 3 chars for "..."
    int keepLeft = available / 2;
    int keepRight = available - keepLeft;  // Automatically gets remainder when odd
    
    return candidateName.left(keepLeft) + "..." + candidateName.right(keepRight);
}

int ResourceLocation::getMaxDisplayLength()
{
    QSettings settings(QStringLiteral("ScadTemplates"), QStringLiteral("ResourcePaths"));
    return settings.value(QStringLiteral("max_display_name_length"), 60).toInt();
}

} // namespace platformInfo
