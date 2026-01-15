/**
 * @file test_sa_inventory_tree.cpp
 * @brief Standalone test: Display resource inventory as a tree
 * 
 * Purpose: Demonstrate building and displaying complete resource inventory
 * Usage:   test_sa_inventory_tree [restype]
 * Examples:
 *   test_sa_inventory_tree             - Show all resource types
 *   test_sa_inventory_tree templates   - Show only templates
 *   test_sa_inventory_tree examples    - Show only examples
 *   test_sa_inventory_tree libraries   - Show only libraries
 */

#include <QCoreApplication>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <platformInfo/ResourceLocation.hpp>
#include <pathDiscovery/ResourcePaths.hpp>
#include <pathDiscovery/PathElement.hpp>
#include <resourceScanning/ResourceScanner.hpp>
#include <resourceInventory/TemplatesInventory.hpp>
#include <resourceInventory/ExamplesInventory.hpp>
#include <resourceInventory/resourceItem.hpp>
#include <resourceMetadata/ResourceTier.hpp>
#include <resourceMetadata/ResourceTypeInfo.hpp>

#ifdef USE_TEST_APP_INFO
#include "testAppNameInfo.hpp"
#else
#include "applicationNameInfo.hpp"
#endif

using namespace platformInfo;
using namespace pathDiscovery;
using namespace resourceScanning;
using namespace resourceInventory;
using namespace resourceMetadata;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
#ifdef USE_TEST_APP_INFO
    if (argc > 2) {
        appInfo::setTestAppName(argv[2]);
    } else {
        appInfo::setTestAppName("TestInventory");
    }
#endif
    
    QTextStream out(stdout);
    
    // Parse command-line argument for resource type filter
    QString filterType;
    if (argc > 1) {
        filterType = QString(argv[1]).toLower();
        
        // Validate filter type
        if (!s_allResourceFolders.contains(filterType)) {
            out << "ERROR: Invalid resource type '" << filterType << "'\n\n";
            out << "Valid types:\n";
            for (const QString& type : s_allResourceFolders) {
                out << "  - " << type << "\n";
            }
            return 1;
        }
    }
    
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "RESOURCE INVENTORY TREE\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    if (filterType.isEmpty()) {
        out << "Showing: ALL resource types\n";
    } else {
        out << "Showing: " << filterType << " only\n";
    }
    out << "Usage: test_sa_inventory_tree [templates|examples|libraries|fonts|...]\n\n";
    
    // ========================================================================
    // STEP 1: Discover Locations
    // ========================================================================
    out << "STEP 1: Discovering Resource Locations\n";
    out << "───────────────────────────────────────────────────────────────────\n\n";
    
    ResourcePaths resPaths;
    QList<PathElement> qualifiedPaths = resPaths.qualifiedSearchPaths();
    
    QList<ResourceLocation> allLocations;
    for (const PathElement& pe : qualifiedPaths) {
        QFileInfo info(pe.path());
        if (info.exists() && info.isDir()) {
            allLocations.append(ResourceLocation(pe.path(), pe.tier()));
        }
    }
    
    out << QString("Found %1 existing locations\n\n").arg(allLocations.size());
    
    // ========================================================================
    // STEP 2: Scan for Resources
    // ========================================================================
    out << "STEP 2: Scanning for Resources\n";
    out << "───────────────────────────────────────────────────────────────────\n\n";
    
    ResourceScanner scanner;
    
    // Scan templates
    TemplatesInventory templatesInv;
    if (filterType.isEmpty() || filterType == "templates") {
        for (const ResourceLocation& loc : allLocations) {
            QList<resourceItem> items = scanner.scanLocation(loc, "templates");
            for (const resourceItem& item : items) {
                templatesInv.addItem(item);
            }
        }
    }
    
    // Scan examples
    ExamplesInventory examplesInv;
    if (filterType.isEmpty() || filterType == "examples") {
        for (const ResourceLocation& loc : allLocations) {
            QList<resourceItem> items = scanner.scanLocation(loc, "examples");
            for (const resourceItem& item : items) {
                examplesInv.addItem(item);
            }
        }
    }
    
    out << QString("Templates found: %1\n").arg(templatesInv.itemCount());
    out << QString("Examples found:  %1\n\n").arg(examplesInv.itemCount());
    
    // ========================================================================
    // STEP 3: Display Tree
    // ========================================================================
    out << "STEP 3: Inventory Tree\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    // Display templates
    if ((filterType.isEmpty() || filterType == "templates") && templatesInv.itemCount() > 0) {
        out << "📄 TEMPLATES (" << templatesInv.itemCount() << " items)\n";
        out << "───────────────────────────────────────────────────────────────────\n";
        
        QList<resourceItem> allTemplates = templatesInv.allItems();
        
        // Group by tier
        QMap<ResourceTier, QList<resourceItem>> byTier;
        for (const resourceItem& item : allTemplates) {
            byTier[item.tier()].append(item);
        }
        
        for (ResourceTier tier : s_allTiersList) {
            if (!byTier.contains(tier)) continue;
            
            QList<resourceItem> tierItems = byTier[tier];
            out << QString("\n  📁 %1 Tier (%2 items)\n").arg(tierToString(tier)).arg(tierItems.size());
            
            for (const resourceItem& item : tierItems) {
                out << QString("    ├─ %1\n").arg(item.displayName());
                out << QString("    │  Category: %1\n").arg(item.category());
                out << QString("    │  Path: %1\n").arg(item.filePath());
            }
        }
        out << "\n";
    }
    
    // Display examples
    if ((filterType.isEmpty() || filterType == "examples") && examplesInv.itemCount() > 0) {
        out << "📘 EXAMPLES (" << examplesInv.itemCount() << " items)\n";
        out << "───────────────────────────────────────────────────────────────────\n";
        
        QList<resourceItem> allExamples = examplesInv.allItems();
        
        // Group by tier
        QMap<ResourceTier, QList<resourceItem>> byTier;
        for (const resourceItem& item : allExamples) {
            byTier[item.tier()].append(item);
        }
        
        for (ResourceTier tier : s_allTiersList) {
            if (!byTier.contains(tier)) continue;
            
            QList<resourceItem> tierItems = byTier[tier];
            out << QString("\n  📁 %1 Tier (%2 items)\n").arg(tierToString(tier)).arg(tierItems.size());
            
            for (const resourceItem& item : tierItems) {
                out << QString("    ├─ %1\n").arg(item.displayName());
                out << QString("    │  Category: %1\n").arg(item.category());
                out << QString("    │  Path: %1\n").arg(item.filePath());
            }
        }
        out << "\n";
    }
    
    // Summary
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "SUMMARY\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    int totalItems = templatesInv.itemCount() + examplesInv.itemCount();
    out << QString("Total items displayed: %1\n").arg(totalItems);
    
    if (filterType.isEmpty()) {
        out << "\nTip: Use 'test_sa_inventory_tree templates' to show only templates\n";
        out << "     Use 'test_sa_inventory_tree examples' to show only examples\n";
    }
    
    out << "\n";
    
    return 0;
}
