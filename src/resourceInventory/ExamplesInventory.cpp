/**
 * @file ExamplesInventory.cpp
 * @brief Implementation of ExamplesInventory
 */

#include "ExamplesInventory.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"

#include <QFileInfo>
#include <QDir>
#include <QDirListing>

namespace resourceInventory {

using namespace resourceMetadata;

bool ExamplesInventory::addExample(const QDirListing::DirEntry& entry, const QString& tier, const QString& category)
{
    QString scriptPath = entry.filePath();
    
    // Validate it's a .scad file
    if (!scriptPath.endsWith(QStringLiteral(".scad"), Qt::CaseInsensitive)) {
        return false;
    }
    
    // Generate hierarchical key: "tier-category-name"
    QFileInfo fi(scriptPath);
    QString baseName = fi.baseName();
    QString key = QString("%1-%2-%3").arg(tier, category, baseName);
    
    // Check if already exists
    if (m_scripts.contains(key)) {
        return false;
    }
    
    // Create ResourceScript
    ResourceScript script(scriptPath);
    script.setType(ResourceType::Examples);
    script.setName(entry.fileName());
    script.setCategory(category);
    script.setDisplayName(baseName);
    script.setScriptPath(scriptPath);
    
    // Scan for attachments
    QStringList attachments = scanAttachments(scriptPath);
    if (!attachments.isEmpty()) {
        script.setAttachments(attachments);
    }
    
    // Mark as exists
    script.setExists(true);
    
    // Store in hash with hierarchical key
    m_scripts.insert(key, QVariant::fromValue(script));
    
    return true;
}

bool ExamplesInventory::addFolder(const QDirListing::DirEntry& entry, const QString& tier, const QString& category)
{
    QString folderPath = entry.filePath();
    
    // Check if folder exists and is actually a directory
    QFileInfo fi(folderPath);
    if (!fi.exists() || !fi.isDir()) {
        return false;
    }
    
    // Scan folder for .scad files using QDirListing
    bool foundAny = false;
    for (const auto& scriptEntry : QDirListing(folderPath, {"*.scad"})) {
        if (scriptEntry.isFile()) {
            if (addExample(scriptEntry, tier, category)) {
                foundAny = true;
            }
        }
    }
    
    return foundAny;
}

QVariant ExamplesInventory::get(const QString& key) const
{
    return m_scripts.value(key);
}

QVariant ExamplesInventory::getByPath(const QString& path) const
{
    // Search all entries for matching path (slower but necessary for path-based lookups)
    for (auto it = m_scripts.constBegin(); it != m_scripts.constEnd(); ++it) {
        const QVariant& var = it.value();
        if (var.canConvert<ResourceScript>()) {
            ResourceScript script = var.value<ResourceScript>();
            if (script.scriptPath() == path) {
                return var;
            }
        }
    }
    return QVariant(); // Not found
}

bool ExamplesInventory::contains(const QString& path) const
{
    return m_scripts.contains(path);
}

QList<QVariant> ExamplesInventory::getAll() const
{
    return m_scripts.values();
}

QList<QVariant> ExamplesInventory::getByCategory(const QString& category) const
{
    QList<QVariant> filtered;
    
    for (const QVariant& var : m_scripts) {
        if (var.canConvert<ResourceScript>()) {
            ResourceScript script = var.value<ResourceScript>();
            if (script.category() == category) {
                filtered.append(var);
            }
        }
    }
    
    return filtered;
}

QStringList ExamplesInventory::getCategories() const
{
    QStringList categories;
    
    for (const QVariant& var : m_scripts) {
        if (var.canConvert<ResourceScript>()) {
            ResourceScript script = var.value<ResourceScript>();
            QString cat = script.category();
            if (!cat.isEmpty() && !categories.contains(cat)) {
                categories.append(cat);
            }
        }
    }
    
    categories.sort(Qt::CaseInsensitive);
    return categories;
}

QStringList ExamplesInventory::scanAttachments(const QString& scriptPath) const
{
    QFileInfo scriptInfo(scriptPath);
    QString baseName = scriptInfo.baseName();  // filename without extension
    QString dirPath = scriptInfo.absolutePath();
    
    QStringList attachments;
    
    // Scan directory for files with same baseName but different extensions
    QDir dir(dirPath);
    if (!dir.exists()) {
        return attachments;
    }
    
    // Get s_attachments list from ResourceTypeInfo
    const QStringList& allowedExtensions = resourceMetadata::s_attachments;
    
    // Check each allowed extension
    for (const QString& ext : allowedExtensions) {
        QString candidatePath = dir.filePath(baseName + ext);
        if (QFileInfo::exists(candidatePath)) {
            attachments.append(candidatePath);
        }
    }
    
    return attachments;
}

} // namespace resourceInventory
