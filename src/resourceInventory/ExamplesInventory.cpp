/**
 * @file ExamplesInventory.cpp
 * @brief Implementation of ExamplesInventory
 */

#include "ExamplesInventory.hpp"
#include "../platformInfo/ResourceLocation.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"

#include <QFileInfo>
#include <QDir>
#include <QDirListing>

namespace resourceInventory {

using namespace resourceMetadata;
using namespace platformInfo;

bool ExamplesInventory::addExample(const QDirListing::DirEntry& entry, 
                                    const platformInfo::ResourceLocation& location,
                                    const QString& category)
{
    QString scriptPath = entry.filePath();
    
    // Validate it's a .scad file
    if (!scriptPath.endsWith(QStringLiteral(".scad"), Qt::CaseInsensitive)) {
        return false;
    }
    
    // Create ResourceScript using location-based constructor
    ResourceScript script(scriptPath, location);
    script.setCategory(category);
    
    // Scan for attachments
    QStringList attachments = scanAttachments(scriptPath);
    if (!attachments.isEmpty()) {
        script.setAttachments(attachments);
    }
    
    // Try to insert - fails if uniqueID already exists (atomic duplicate detection)
    QString uniqueID = script.uniqueID();
    auto result = m_scripts.tryInsert(uniqueID, QVariant::fromValue(script));
    
    if (!result.inserted) {
        qWarning() << "ExamplesInventory: Duplicate example ID:" << uniqueID
                   << "at" << entry.filePath();
        return false;
    }
    
    return true;
}

int ExamplesInventory::addFolder(const QString& folderPath, 
                                  const platformInfo::ResourceLocation& location,
                                  const QString& category)
{
    int sizeBefore = m_scripts.size();
    
    // Scan folder for .scad files (FilesOnly flag eliminates need for isFile() check)
    QDirListing listing(folderPath, {"*.scad"}, QDirListing::IteratorFlag::FilesOnly);
    
    for (const auto& fileEntry : listing) {
        addExample(fileEntry, location, category);  // Failures logged internally
    }
    
    return m_scripts.size() - sizeBefore;
}

QVariant ExamplesInventory::get(const QString& key) const
{
    return m_scripts.value(key);
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
