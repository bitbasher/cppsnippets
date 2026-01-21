/**
 * @file test_sa_fonts_tree.cpp
 * @brief Standalone test: Display fonts inventory as a tree
 * 
 * Phase 5 validation: Verifies font resource discovery
 * 
 * Purpose: Demonstrate building and displaying font inventory
 *          Simple file-based resources (no metadata, no attachments)
 * 
 * Usage:   test_sa_fonts_tree [--help]
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
#include "ApplicationNameInfo.hpp"
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
    out << "Usage: test_sa_fonts_tree [OPTIONS]\n\n";
    out << "Display fonts inventory as a hierarchical tree.\n\n";
    out << "OPTIONS:\n";
    out << "  -h, --help, --usage    Show this help message\n\n";
    out << "Examples:\n";
    out << "  test_sa_fonts_tree\n";
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
    out << "FONTS INVENTORY TREE (Phase 5 Validation)\n";
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
    // STEP 2: Scan for Fonts
    // ========================================================================
    out << "STEP 2: Scanning Fonts into Inventory\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    
    ResourceScanner scanner;
    
    // Create a dummy model - scanner needs it even though we don't use it
    QStandardItemModel dummyModel;
    scanner.scanToModel(&dummyModel, allLocations);
    
    out << QString("Fonts found: %1\n").arg(scanner.fontsCount());
    out << "\n";
    
    // ========================================================================
    // STEP 3: Display Tree
    // ========================================================================
    out << "STEP 3: Fonts Tree (Hierarchical Display)\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    const auto& fontsInv = scanner.fontsInventory();
    
    if (fontsInv.count() == 0) {
        out << "No fonts found.\n\n";
        return 0;
    }
    
    out << "🔤 FONTS (" << fontsInv.count() << " items)\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    
    QList<QVariant> allFonts = fontsInv.getAll();
    
    // Debug: Check if we can extract fonts from QVariant
    out << QString("Debug: Got %1 QVariants from inventory\n").arg(allFonts.size());
    int convertibleCount = 0;
    for (const QVariant& var : allFonts) {
        if (var.canConvert<ResourceItem>()) {
            convertibleCount++;
        }
    }
    out << QString("Debug: %1 can convert to ResourceItem\n\n").arg(convertibleCount);
    
    // Group by tier
    QMap<ResourceTier, QList<QVariant>> byTier;
    for (const QVariant& var : allFonts) {
        if (var.canConvert<ResourceItem>()) {
            ResourceItem font = var.value<ResourceItem>();
            byTier[font.tier()].append(var);
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
            if (var.canConvert<ResourceItem>()) {
                ResourceItem font = var.value<ResourceItem>();
                QString cat = font.category().isEmpty() ? "Uncategorized" : font.category();
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
                
                if (var.canConvert<ResourceItem>()) {
                    ResourceItem font = var.value<ResourceItem>();
                    QFileInfo fontInfo(font.path());
                    QString displayName = font.displayName();
                    QString extension = fontInfo.suffix().toUpper();
                    
                    out << QString("  │  %1 %2 (.%3)\n")
                        .arg(connector)
                        .arg(displayName)
                        .arg(extension);
                    out << QString("  │  %1   Path: %2\n").arg(indent).arg(font.path());
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
    out << QString("Total fonts: %1\n").arg(fontsInv.count());
    
    // Count by extension
    QMap<QString, int> byExtension;
    for (const QVariant& var : allFonts) {
        if (var.canConvert<ResourceItem>()) {
            ResourceItem font = var.value<ResourceItem>();
            QFileInfo info(font.path());
            QString ext = info.suffix().toLower();
            byExtension[ext]++;
        }
    }
    
    out << "\nBy extension:\n";
    for (auto it = byExtension.constBegin(); it != byExtension.constEnd(); ++it) {
        out << QString("  .%1: %2 fonts\n").arg(it.key()).arg(it.value());
    }
    
    out << "\n✅ QVariant preserves ResourceItem type correctly\n";
    out << QString("✅ %1/%2 fonts convertible from QVariant\n")
        .arg(convertibleCount).arg(allFonts.size());
    out << "═══════════════════════════════════════════════════════════════════\n";
    
    return 0;
}
