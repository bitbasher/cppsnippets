/**
 * @file resourceIterator.cpp
 * @brief Implementation of resource iterator classes
 * 
 * Provides scanning and iteration over resource folder locations,
 * collecting found resources into ResLocMap (flat) or ResLocTree
 * (hierarchical) storage structures.
 */

#include "resInventory/resourceIterator.h"

#include <QDir>
#include <QFileInfo>

namespace resInventory {

// ============================================================================
// ResourceIteratorBase
// ============================================================================

ResourceIteratorBase::ResourceIteratorBase(ResourceTier tier,
                                           const QStringList& folderLocations,
                                           const QVector<platformInfo::ResourceType>& resourceTypes)
    : m_tier(tier)
    , m_folderLocations(folderLocations)
    , m_resourceTypes(resourceTypes)
{
}

// ============================================================================
// ResourceIteratorFlat
// ============================================================================

ResourceIteratorFlat::ResourceIteratorFlat(ResourceTier tier,
                                           const QStringList& folderLocations,
                                           const QVector<platformInfo::ResourceType>& resourceTypes)
    : ResourceIteratorBase(tier, folderLocations, resourceTypes)
{
}

bool ResourceIteratorFlat::scan()
{
    bool foundAny = false;
    
    for (const QString& folderPath : m_folderLocations) {
        QDir folder(folderPath);
        if (!folder.exists()) {
            continue;
        }
        
        // Check for each resource type subdirectory
        for (const auto& resType : m_resourceTypes) {
            QString subdir = platformInfo::ResourcePaths::resourceSubdirectory(resType);
            if (subdir.isEmpty()) {
                continue;
            }
            
            QString resPath = folder.filePath(subdir);
            QDir resDir(resPath);
            
            if (resDir.exists()) {
                // Create a ResourceLocation for this found resource
                platformInfo::ResourceLocation loc;
                loc.path = resDir.absolutePath();
                loc.displayName = subdir;
                loc.description = QString("Found in %1").arg(folderPath);
                loc.exists = true;
                loc.isEnabled = true;
                loc.hasResourceFolders = true;
                
                // Determine writability
                QFileInfo fi(resPath);
                loc.isWritable = fi.isWritable();
                
                // Use path as key for uniqueness
                QString key = loc.path;
                
                // Add to appropriate tier in results
                switch (m_tier) {
                    case ResourceTier::Installation:
                        if (!m_results.hasInstallLocation(key)) {
                            m_results.addInstallLocation(key, loc);
                            foundAny = true;
                        }
                        break;
                    case ResourceTier::Machine:
                        if (!m_results.hasMachineLocation(key)) {
                            m_results.addMachineLocation(key, loc);
                            foundAny = true;
                        }
                        break;
                    case ResourceTier::User:
                        if (!m_results.hasUserLocation(key)) {
                            m_results.addUserLocation(key, loc);
                            foundAny = true;
                        }
                        break;
                }
            }
        }
    }
    
    return foundAny;
}

// ============================================================================
// ResourceIteratorTree
// ============================================================================

ResourceIteratorTree::ResourceIteratorTree(ResourceTier tier,
                                           const QStringList& folderLocations,
                                           const QVector<platformInfo::ResourceType>& resourceTypes,
                                           QWidget* parent)
    : ResourceIteratorBase(tier, folderLocations, resourceTypes)
    , m_results(new ResLocTree(parent))
{
}

ResourceIteratorTree::~ResourceIteratorTree()
{
    delete m_results;
}

bool ResourceIteratorTree::scan()
{
    bool foundAny = false;
    
    for (const QString& folderPath : m_folderLocations) {
        QDir folder(folderPath);
        if (!folder.exists()) {
            continue;
        }
        
        // Check for each resource type subdirectory
        for (const auto& resType : m_resourceTypes) {
            QString subdir = platformInfo::ResourcePaths::resourceSubdirectory(resType);
            if (subdir.isEmpty()) {
                continue;
            }
            
            QString resPath = folder.filePath(subdir);
            QDir resDir(resPath);
            
            if (resDir.exists()) {
                // Create a ResourceLocation for the top-level resource
                platformInfo::ResourceLocation loc;
                loc.path = resDir.absolutePath();
                loc.displayName = subdir;
                loc.description = QString("Found in %1").arg(folderPath);
                loc.exists = true;
                loc.isEnabled = true;
                loc.hasResourceFolders = true;
                
                QFileInfo fi(resPath);
                loc.isWritable = fi.isWritable();
                
                // Add as top-level item
                ResLocTreeItem* topItem = m_results->addTopLevelLocation(loc);
                foundAny = true;
                
                // For hierarchical types (like Libraries), scan children
                if (ResourceIteratorFactory::isHierarchicalType(resType)) {
                    scanDirectory(topItem, resPath, 0);
                }
            }
        }
    }
    
    return foundAny;
}

ResLocTree* ResourceIteratorTree::takeResults()
{
    ResLocTree* tree = m_results;
    m_results = nullptr;
    return tree;
}

void ResourceIteratorTree::scanDirectory(ResLocTreeItem* parentItem, const QString& path, int depth)
{
    // Limit recursion depth to prevent infinite loops
    const int MAX_DEPTH = 10;
    if (depth >= MAX_DEPTH || !parentItem) {
        return;
    }
    
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }
    
    // Get all subdirectories
    QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    
    for (const QString& subdir : subdirs) {
        QString subdirPath = dir.filePath(subdir);
        QFileInfo fi(subdirPath);
        
        // Create location for this child
        platformInfo::ResourceLocation childLoc;
        childLoc.path = fi.absoluteFilePath();
        childLoc.displayName = subdir;
        childLoc.exists = true;
        childLoc.isEnabled = true;
        childLoc.isWritable = fi.isWritable();
        
        // Check if this subdirectory contains known resource types
        // (examples, tests, etc. within a library)
        bool hasKnownResources = false;
        for (const auto& resType : m_resourceTypes) {
            QString resSubdir = platformInfo::ResourcePaths::resourceSubdirectory(resType);
            if (subdir.compare(resSubdir, Qt::CaseInsensitive) == 0) {
                hasKnownResources = true;
                childLoc.description = QString("%1 resources").arg(resSubdir);
                break;
            }
        }
        
        if (!hasKnownResources) {
            // Could be a sub-library or other nested resource
            childLoc.description = "Subdirectory";
        }
        
        childLoc.hasResourceFolders = hasKnownResources;
        
        // Add as child item
        ResLocTreeItem* childItem = m_results->addChildLocation(parentItem, childLoc);
        
        // Recursively scan this subdirectory
        scanDirectory(childItem, subdirPath, depth + 1);
    }
}

// ============================================================================
// ResourceIteratorFactory
// ============================================================================

std::unique_ptr<ResourceIteratorBase> ResourceIteratorFactory::create(
    ResourceTier tier,
    const QStringList& folderLocations,
    const QVector<platformInfo::ResourceType>& resourceTypes,
    QWidget* parent)
{
    if (containsHierarchicalType(resourceTypes)) {
        return std::make_unique<ResourceIteratorTree>(tier, folderLocations, resourceTypes, parent);
    } else {
        return std::make_unique<ResourceIteratorFlat>(tier, folderLocations, resourceTypes);
    }
}

std::unique_ptr<ResourceIteratorFlat> ResourceIteratorFactory::createFlat(
    ResourceTier tier,
    const QStringList& folderLocations,
    const QVector<platformInfo::ResourceType>& resourceTypes)
{
    return std::make_unique<ResourceIteratorFlat>(tier, folderLocations, resourceTypes);
}

std::unique_ptr<ResourceIteratorTree> ResourceIteratorFactory::createTree(
    ResourceTier tier,
    const QStringList& folderLocations,
    const QVector<platformInfo::ResourceType>& resourceTypes,
    QWidget* parent)
{
    return std::make_unique<ResourceIteratorTree>(tier, folderLocations, resourceTypes, parent);
}

bool ResourceIteratorFactory::isHierarchicalType(platformInfo::ResourceType type)
{
    // Libraries are hierarchical - they may contain examples, tests, sub-libraries
    return (type == platformInfo::ResourceType::Libraries);
}

bool ResourceIteratorFactory::containsHierarchicalType(const QVector<platformInfo::ResourceType>& types)
{
    for (const auto& type : types) {
        if (isHierarchicalType(type)) {
            return true;
        }
    }
    return false;
}

} // namespace resInventory
