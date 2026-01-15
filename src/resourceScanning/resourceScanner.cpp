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

bool ResourceScanner::scanToModel(QStandardItemModel* model, 
                                   const QList<platformInfo::ResourceLocation>& locations)
{
    if (!model) {
        qWarning() << "ResourceScanner::scanToModel: null model provided";
        return false;
    }
    
    // Clear existing data
    model->clear();
    m_examplesInventory.clear();
    m_templatesInventory.clear();
    m_fontsInventory.clear();
    m_shadersInventory.clear();
    m_translationsInventory.clear();
    m_testsInventory.clear();
    
    // Scan each location
    for (const auto& location : locations) {
        qDebug() << "Scanning location:" << location.path() 
                 << "tier:" << resourceMetadata::tierToString(location.tier());
        
        // Phase 3: Examples
        int examplesAdded = scanExamplesAt(location);
        if (examplesAdded > 0) {
            qDebug() << "  Added" << examplesAdded << "examples";
        }
        
        // Phase 4: Templates
        int templatesAdded = scanTemplatesAt(location);
        if (templatesAdded > 0) {
            qDebug() << "  Added" << templatesAdded << "templates";
        }
        
        // Phase 5: Fonts
        int fontsAdded = scanFontsAt(location);
        if (fontsAdded > 0) {
            qDebug() << "  Added" << fontsAdded << "fonts";
        }
        
        // Phase 5: Shaders
        int shadersAdded = scanShadersAt(location);
        if (shadersAdded > 0) {
            qDebug() << "  Added" << shadersAdded << "shaders";
        }
        
        // Phase 5: Translations
        int translationsAdded = scanTranslationsAt(location);
        if (translationsAdded > 0) {
            qDebug() << "  Added" << translationsAdded << "translations";
        }
        
        // Phase 5: Tests
        int testsAdded = scanTestsAt(location);
        if (testsAdded > 0) {
            qDebug() << "  Added" << testsAdded << "tests";
        }
        
        // Future phases: scanLibrariesAt(), scanColorSchemesAt(), etc.
    }
    
    // Populate model from inventories
    populateModel(model);
    
    qDebug() << "ResourceScanner: Total examples:" << m_examplesInventory.count()
             << "templates:" << m_templatesInventory.count()
             << "fonts:" << m_fontsInventory.count()
             << "shaders:" << m_shadersInventory.count()
             << "translations:" << m_translationsInventory.count()
             << "tests:" << m_testsInventory.count();
    
    return true;
}

int ResourceScanner::scanExamplesAt(const platformInfo::ResourceLocation& location)
{
    QString examplesPath = location.path() + "/examples";
    
    // Check if examples folder exists
    QDir examplesDir(examplesPath);
    if (!examplesDir.exists()) {
        return 0; // Not an error - location may not have examples
    }
    
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
    
    // Check if templates folder exists
    QDir templatesDir(templatesPath);
    if (!templatesDir.exists()) {
        return 0; // Not an error - location may not have templates
    }
    
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

void ResourceScanner::populateModel(QStandardItemModel* model)
{
    // Phase 3-4: Simple flat list for debugging
    // Future: Hierarchical model with Tier → ResourceType → Category → Items
    
    model->setHorizontalHeaderLabels({"Name", "Type", "Category", "Tier", "Path"});
    
    // Add examples
    QList<QVariant> allExamples = m_examplesInventory.getAll();
    for (const QVariant& var : allExamples) {
        if (!var.canConvert<resourceInventory::ResourceScript>()) {
            continue;
        }
        
        resourceInventory::ResourceScript script = var.value<resourceInventory::ResourceScript>();
        
        QList<QStandardItem*> row;
        row.append(new QStandardItem(script.displayName()));
        row.append(new QStandardItem("Example"));
        row.append(new QStandardItem(script.category()));
        row.append(new QStandardItem(resourceMetadata::tierToString(script.tier())));
        row.append(new QStandardItem(script.scriptPath()));
        
        model->appendRow(row);
    }
    
    // Add templates
    QList<QVariant> allTemplates = m_templatesInventory.getAll();
    for (const QVariant& var : allTemplates) {
        if (!var.canConvert<resourceInventory::ResourceItem>()) {
            continue;
        }
        
        resourceInventory::ResourceItem tmpl = var.value<resourceInventory::ResourceItem>();
        
        QList<QStandardItem*> row;
        row.append(new QStandardItem(tmpl.displayName()));
        row.append(new QStandardItem("Template"));
        row.append(new QStandardItem(""));  // No category for templates
        row.append(new QStandardItem(resourceMetadata::tierToString(tmpl.tier())));
        row.append(new QStandardItem(tmpl.path()));
        
        model->appendRow(row);
    }
}

int ResourceScanner::scanFontsAt(const platformInfo::ResourceLocation& location)
{
    QString fontsPath = location.path() + "/fonts";
    
    // Check if fonts folder exists
    QDir fontsDir(fontsPath);
    if (!fontsDir.exists()) {
        return 0; // Not an error - location may not have fonts
    }
    
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
    
    // Check if shaders folder exists
    QDir shadersDir(shadersPath);
    if (!shadersDir.exists()) {
        return 0; // Not an error - location may not have shaders
    }
    
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
    
    // Check if locale folder exists
    QDir localeDir(localePath);
    if (!localeDir.exists()) {
        return 0; // Not an error - location may not have translations
    }
    
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
    
    // Check if tests folder exists
    QDir testsDir(testsPath);
    if (!testsDir.exists()) {
        return 0; // Not an error - location may not have tests
    }
    
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

} // namespace resourceScanning
