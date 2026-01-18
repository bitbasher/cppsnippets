/**
 * @file ResourceScanner.cpp
 * @brief Implementation of top-level resource scanner
 */

#include "ResourceScanner.hpp"
#include "../resourceMetadata/ResourceTypeInfo.hpp"

#include <QDir>
#include <QDirListing>
#include <QDebug>

namespace resourceScanning {

QStandardItemModel* ResourceScanner::scanToModel(const QList<platformInfo::ResourceLocation>& locations)
{
    using resourceMetadata::ResourceType;
    using resourceMetadata::s_topLevelReverse;
    
    // Dispatch map: ResourceType → scanner function
    using ScannerFunc = int (ResourceScanner::*)(const platformInfo::ResourceLocation&);
    const QMap<ResourceType, ScannerFunc> scannerDispatch = {
        {ResourceType::Examples, &ResourceScanner::scanExamplesAt},
        {ResourceType::Templates, &ResourceScanner::scanTemplatesAt},
        {ResourceType::Fonts, &ResourceScanner::scanFontsAt},
        {ResourceType::Shaders, &ResourceScanner::scanShadersAt},
        {ResourceType::Translations, &ResourceScanner::scanTranslationsAt},
        {ResourceType::Tests, &ResourceScanner::scanTestsAt}
        // Future: ResourceType::Libraries, ResourceType::ColorSchemes
    };
    
    // Scan each location - discover what resource folders exist
    for (const auto& location : locations) {
        qDebug() << "Scanning location:" << location.path();
        
        using ItFlag = QDirListing::IteratorFlag;
        
        // Discover resource folders using QDirListing (non-recursive, dirs only)
        for (const auto& dirEntry : QDirListing(location.path(), ItFlag::DirsOnly)) {
            QString folderName = dirEntry.fileName();
            
            // Check if this is a known resource folder
            if (s_topLevelReverse.contains(folderName)) {
                ResourceType resType = s_topLevelReverse[folderName];
                
                // Dispatch to scanner if implemented
                if (scannerDispatch.contains(resType)) {
                    (this->*(scannerDispatch[resType]))(location);
                }
            }
        }
    }
    
    // Create and populate model for templates only (this app's focus)
    QStandardItemModel* model = populateModel(ResourceType::Templates);
    
    qDebug() << "ResourceScanner: Total examples:" << m_examplesInventory.count()
             << "templates:" << m_templatesInventory.count()
             << "fonts:" << m_fontsInventory.count()
             << "shaders:" << m_shadersInventory.count()
             << "translations:" << m_translationsInventory.count()
             << "tests:" << m_testsInventory.count();
    
    return model;
}

int ResourceScanner::scanExamplesAt(const platformInfo::ResourceLocation& location)
{
    QString examplesPath = location.path() + "/examples";
    int addedCount = 0;
    QString tierStr = resourceMetadata::tierToString(location.tier());
    
    // Traverse examples folder - one level only (category folders)
    for (const auto& entry : QDirListing(examplesPath, QDirListing::IteratorFlag::Recursive)) {
        if (entry.isDir()) {
            // Category folder (e.g., "BasicShapes", "Functions")
            QString categoryName = entry.baseName();
            if (m_examplesInventory.addFolder(entry, tierStr, categoryName)) {
                addedCount++;
            }
        } else if (entry.isFile() && entry.fileName().endsWith(".scad")) {
            // Loose .scad file in examples root (uncategorized)
            if (m_examplesInventory.addExample(entry, tierStr, "uncategorized")) {
                addedCount++;
            }
        }
    }
    
    return addedCount;
}

int ResourceScanner::scanTemplatesAt(const platformInfo::ResourceLocation& location)
{
    QString templatesPath = location.path() + "/templates";
    int addedCount = 0;
    QString tierStr = resourceMetadata::tierToString(location.tier());
    
    // Scan templates folder for .json files (no categories for templates)
    for (const auto& entry : QDirListing(templatesPath, {"*.json"})) {
        if (entry.isFile()) {
            if (m_templatesInventory.addTemplate(entry, tierStr)) {
                addedCount++;
            }
        }
    }
    
    return addedCount;
}

QStandardItemModel* ResourceScanner::populateModel(resourceMetadata::ResourceType type)
{
    using resourceMetadata::ResourceType;
    
    switch (type) {
        case ResourceType::Examples: {
            if (m_examplesInventory.count() == 0) {
                return nullptr;
            }
            
            QStandardItemModel* model = new QStandardItemModel();
            model->setHorizontalHeaderLabels({"Name", "Category", "Tier", "Path"});
            
            QList<QVariant> allExamples = m_examplesInventory.getAll();
            for (const QVariant& var : allExamples) {
                if (!var.canConvert<resourceInventory::ResourceScript>()) {
                    continue;
                }
                
                resourceInventory::ResourceScript script = var.value<resourceInventory::ResourceScript>();
                
                QList<QStandardItem*> row;
                row.append(new QStandardItem(script.displayName()));
                row.append(new QStandardItem(script.category()));
                row.append(new QStandardItem(resourceMetadata::tierToString(script.tier())));
                row.append(new QStandardItem(script.scriptPath()));
                
                model->appendRow(row);
            }
            return model;
        }
        
        case ResourceType::Templates: {
            if (m_templatesInventory.count() == 0) {
                return nullptr;
            }
            
            QStandardItemModel* model = new QStandardItemModel();
            model->setHorizontalHeaderLabels({"Name", "Tier", "Path"});
            
            QList<QVariant> allTemplates = m_templatesInventory.getAll();
            for (const QVariant& var : allTemplates) {
                if (!var.canConvert<resourceInventory::ResourceItem>()) {
                    continue;
                }
                
                resourceInventory::ResourceItem tmpl = var.value<resourceInventory::ResourceItem>();
                
                QList<QStandardItem*> row;
                row.append(new QStandardItem(tmpl.displayName()));
                row.append(new QStandardItem(resourceMetadata::tierToString(tmpl.tier())));
                row.append(new QStandardItem(tmpl.path()));
                
                model->appendRow(row);
            }
            return model;
        }
        
        // Future: Other resource types
        default:
            qWarning() << "populateModel: ResourceType not yet implemented";
            return nullptr;
    }
}

int ResourceScanner::scanFontsAt(const platformInfo::ResourceLocation& location)
{
    QString fontsPath = location.path() + "/fonts";
    int addedCount = 0;
    
    // Scan for font files (.ttf, .otf)
    for (const auto& entry : QDirListing(fontsPath, QDirListing::IteratorFlag::Recursive)) {
        if (!entry.isFile()) {
            continue;
        }
        
        QString fileName = entry.fileName();
        if (fileName.endsWith(".ttf", Qt::CaseInsensitive) || 
            fileName.endsWith(".otf", Qt::CaseInsensitive)) {
            
            if (m_fontsInventory.addFont(entry.filePath(), location.tier())) {
                addedCount++;
            }
        }
    }
    
    return addedCount;
}

int ResourceScanner::scanShadersAt(const platformInfo::ResourceLocation& location)
{
    QString shadersPath = location.path() + "/shaders";
    int addedCount = 0;
    
    // Scan for shader files (.frag, .vert)
    for (const auto& entry : QDirListing(shadersPath, QDirListing::IteratorFlag::Recursive)) {
        if (!entry.isFile()) {
            continue;
        }
        
        QString fileName = entry.fileName();
        if (fileName.endsWith(".frag", Qt::CaseInsensitive) || 
            fileName.endsWith(".vert", Qt::CaseInsensitive)) {
            
            if (m_shadersInventory.addShader(entry.filePath(), location.tier())) {
                addedCount++;
            }
        }
    }
    
    return addedCount;
}

int ResourceScanner::scanTranslationsAt(const platformInfo::ResourceLocation& location)
{
    QString localePath = location.path() + "/locale";
    int addedCount = 0;
    
    // Scan for translation files (.qm, .ts)
    for (const auto& entry : QDirListing(localePath, QDirListing::IteratorFlag::Recursive)) {
        if (!entry.isFile()) {
            continue;
        }
        
        QString fileName = entry.fileName();
        if (fileName.endsWith(".qm", Qt::CaseInsensitive) || 
            fileName.endsWith(".ts", Qt::CaseInsensitive)) {
            
            if (m_translationsInventory.addTranslation(entry.filePath(), location.tier())) {
                addedCount++;
            }
        }
    }
    
    return addedCount;
}

int ResourceScanner::scanTestsAt(const platformInfo::ResourceLocation& location)
{
    QString testsPath = location.path() + "/tests";
    int addedCount = 0;
    QString tierStr = resourceMetadata::tierToString(location.tier());
    
    // Scan for test files (.scad) - flat structure, no categories
    for (const auto& entry : QDirListing(testsPath, QDirListing::IteratorFlag::Recursive)) {
        if (entry.isFile() && entry.fileName().endsWith(".scad")) {
            if (m_testsInventory.addTest(entry.filePath(), tierStr)) {
                addedCount++;
            }
        }
    }
    
    return addedCount;
}

// ============================================================================
// Location Index Management (Static Members)
// ============================================================================

// Initialize static members
QHash<QString, QString> ResourceScanner::s_pathToLocationIndex;
QHash<QString, QString> ResourceScanner::s_locationIndexToPath;
int ResourceScanner::s_nextLocationIndex = 1;

QString ResourceScanner::normalizePath(const QString& path)
{
    // Convert backslashes to forward slashes
    // Windows handles both / and \ so this is safe
    // Avoids escape character issues in string literals
    QString normalized = path;
    normalized.replace('\\', '/');
    return normalized;
}

QString ResourceScanner::getOrCreateLocationIndex(const QString& folderPath)
{
    // Normalize path before indexing
    QString normalizedPath = normalizePath(folderPath);
    
    // Check if already indexed
    if (s_pathToLocationIndex.contains(normalizedPath)) {
        return s_pathToLocationIndex.value(normalizedPath);
    }
    
    // Create new index
    QString newIndex = numberToLocationIndex(s_nextLocationIndex);
    s_nextLocationIndex++;
    
    // Store bidirectional mapping using normalized path
    s_pathToLocationIndex.insert(normalizedPath, newIndex);
    s_locationIndexToPath.insert(newIndex, normalizedPath);
    
    qDebug() << "ResourceScanner: Indexed location" << newIndex << "=" << normalizedPath;
    
    return newIndex;
}

QString ResourceScanner::getLocationPath(const QString& index)
{
    return s_locationIndexToPath.value(index, QString());
}

int ResourceScanner::locationIndexCount()
{
    return s_pathToLocationIndex.size();
}

QString ResourceScanner::numberToLocationIndex(int index)
{
    // Generate 3-letter indices: aaa, aab, aac, ..., aaz, aba, abb, ..., zzz
    // Supports 26^3 = 17,576 unique locations
    
    if (index < 1) {
        return "aaa";  // Safety fallback
    }
    
    // Convert to 0-based
    index--;
    
    // Calculate three letter positions (base-26)
    int third = index % 26;           // Rightmost letter
    int second = (index / 26) % 26;   // Middle letter
    int first = (index / 676) % 26;   // Leftmost letter (676 = 26²)
    
    // Convert to characters
    QString result;
    result += QChar('a' + first);
    result += QChar('a' + second);
    result += QChar('a' + third);
    
    return result;
}

} // namespace resourceScanning
