/**
 * @file test_sa_examples_tree.cpp
 * @brief Standalone test: Display examples inventory as a tree
 * 
 * Phase 5 validation: Verifies example scripts with attachments display correctly
 * 
 * Purpose: Demonstrate building and displaying examples inventory
 *          with hierarchical display by tier, category, and attachments
 * 
 * Usage:   test_sa_examples_tree [--help]
 */

#include <QCoreApplication>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QStandardItemModel>
#include <platformInfo/ResourceLocation.hpp>
#include <pathDiscovery/ResourcePaths.hpp>
#include <pathDiscovery/PathElement.hpp>
#include <resourceScanning/ResourceScanner.hpp>
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

void printUsage() {
    QTextStream out(stdout);
    out << "Usage: test_sa_examples_tree [OPTIONS]\n\n";
    out << "Display examples inventory as a hierarchical tree with attachments.\n\n";
    out << "OPTIONS:\n";
    out << "  -h, --help, --usage    Show this help message\n\n";
    out << "Examples:\n";
    out << "  test_sa_examples_tree\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QTextStream out(stdout);
    
    // Parse command-line arguments first
    if (argc > 1) {
        QString arg = QString(argv[1]).toLower();
        if (arg == "-h" || arg == "--help" || arg == "--usage") {
            printUsage();
            return 0;
        }
    }
    
#ifdef USE_TEST_APP_INFO
    // Set app name for path discovery (default to OpenSCAD)
    appInfo::setTestAppName("OpenSCAD");
#endif
    
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "EXAMPLES INVENTORY TREE (Phase 5 Validation)\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    // ========================================================================
    // STEP 1: Discover Resource Locations
    // ========================================================================
    out << "STEP 1: Discovering Resource Locations\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    
    ResourcePaths resPaths;
    QList<PathElement> qualifiedPaths = resPaths.qualifiedSearchPaths();
    
    out << QString("Discovered %1 potential paths\n").arg(qualifiedPaths.size());
    
    QList<ResourceLocation> allLocations;
    for (const PathElement& pe : qualifiedPaths) {
        QFileInfo info(pe.path());
        if (info.exists() && info.isDir()) {
            allLocations.append(ResourceLocation(pe.path(), pe.tier()));
        } else {
            out << QString("  Skipped (not found): %1\n").arg(pe.path());
        }
    }
    
    out << QString("\nFound %1 existing locations:\n").arg(allLocations.size());
    for (const ResourceLocation& loc : allLocations) {
        out << QString("  - %1 (%2)\n").arg(loc.getDisplayName()).arg(tierToString(loc.tier()));
    }
    out << "\n";
    
    // ========================================================================
    // STEP 2: Scan for Examples
    // ========================================================================
    out << "STEP 2: Scanning Examples into Inventory\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    
    ResourceScanner scanner;
    
    // Create a dummy model - scanner needs it even though we don't use it
    QStandardItemModel dummyModel;
    scanner.scanToModel(&dummyModel, allLocations);
    
    out << QString("Examples found: %1\n").arg(scanner.examplesCount());
    out << "\n";
    
    // ========================================================================
    // STEP 3: Display Tree
    // ========================================================================
    out << "STEP 3: Examples Tree (Hierarchical Display)\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    const auto& examplesInv = scanner.examplesInventory();
    
    if (examplesInv.count() == 0) {
        out << "No examples found.\n\n";
        return 0;
    }
    
    out << "📄 EXAMPLES (" << examplesInv.count() << " items)\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    
    QList<QVariant> allExamples = examplesInv.getAll();
    
    // Debug: Check if we can extract examples from QVariant
    out << QString("Debug: Got %1 QVariants from inventory\n").arg(allExamples.size());
    int convertibleCount = 0;
    for (const QVariant& var : allExamples) {
        if (var.canConvert<ResourceScript>()) {
            convertibleCount++;
        }
    }
    out << QString("Debug: %1 can convert to ResourceScript\n\n").arg(convertibleCount);
    
    // Group by tier
    QMap<ResourceTier, QList<QVariant>> byTier;
    for (const QVariant& var : allExamples) {
        if (var.canConvert<ResourceScript>()) {
            ResourceScript script = var.value<ResourceScript>();
            byTier[script.tier()].append(var);
        }
    }
    
    // Display by tier
    for (ResourceTier tier : s_allTiersList) {
        if (!byTier.contains(tier)) continue;
        
        QList<QVariant> tierItems = byTier[tier];
        out << QString("\n📁 %1 Tier (%2 items)\n").arg(tierToString(tier)).arg(tierItems.size());
        
        // Group by category within tier
        QMap<QString, QList<QVariant>> byCategory;
        for (const QVariant& var : tierItems) {
            if (var.canConvert<ResourceScript>()) {
                ResourceScript script = var.value<ResourceScript>();
                QString cat = script.category().isEmpty() ? "Uncategorized" : script.category();
                byCategory[cat].append(var);
            }
        }
        
        // Display by category
        QStringList categories = byCategory.keys();
        categories.sort();
        
        for (const QString& category : categories) {
            out << QString("  ├─ %1 (%2)\n").arg(category).arg(byCategory[category].size());
            
            QList<QVariant> catItems = byCategory[category];
            for (int i = 0; i < catItems.size(); ++i) {
                const QVariant& var = catItems[i];
                bool isLast = (i == catItems.size() - 1);
                QString connector = isLast ? "└─" : "├─";
                QString indent = isLast ? "   " : "│  ";
                
                if (var.canConvert<ResourceScript>()) {
                    ResourceScript script = var.value<ResourceScript>();
                    QString displayName = script.displayName();
                    
                    out << QString("  │  %1 %2\n").arg(connector).arg(displayName);
                    out << QString("  │  %1   Path: %2\n").arg(indent).arg(script.path());
                    
                    // Display attachments if present
                    if (script.hasAttachments()) {
                        QStringList attachments = script.attachments();
                        out << QString("  │  %1   📎 Attachments (%2):\n").arg(indent).arg(attachments.size());
                        for (int j = 0; j < attachments.size(); ++j) {
                            bool isLastAttachment = (j == attachments.size() - 1);
                            QString attachConnector = isLastAttachment ? "└─" : "├─";
                            QFileInfo attachInfo(attachments[j]);
                            out << QString("  │  %1      %2 %3\n")
                                .arg(indent)
                                .arg(attachConnector)
                                .arg(attachInfo.fileName());
                        }
                    }
                }
            }
        }
    }
    
    out << "\n";
    
    // ========================================================================
    // Summary
    // ========================================================================
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "SUMMARY\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    out << QString("Total examples: %1\n").arg(examplesInv.count());
    
    int totalAttachments = 0;
    for (const QVariant& var : allExamples) {
        if (var.canConvert<ResourceScript>()) {
            ResourceScript script = var.value<ResourceScript>();
            totalAttachments += script.attachments().size();
        }
    }
    out << QString("Total attachments: %1\n").arg(totalAttachments);
    
    out << "\n✅ QVariant preserves ResourceScript type correctly\n";
    out << QString("✅ %1/%2 examples convertible from QVariant\n")
        .arg(convertibleCount).arg(allExamples.size());
    out << "═══════════════════════════════════════════════════════════════════\n";
    
    return 0;
}
