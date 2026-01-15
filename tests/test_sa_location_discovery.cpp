/**
 * @file test_sa_location_discovery.cpp
 * @brief Standalone test demonstrating ResourceLocation discovery from qualified paths
 * 
 * Shows the next stage after path discovery:
 * 1. Get qualified search paths (PathElement list with tier info)
 * 2. Convert each to ResourceLocation (with existence checking)
 * 3. Display discovered locations grouped by tier
 * 
 * This demonstrates the workflow:
 *   ResourcePaths → PathElement list → ResourceLocation list
 */

#include <QCoreApplication>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>

#ifdef USE_TEST_APP_INFO
#include "testAppNameInfo.hpp"
#else
#include "applicationNameInfo.hpp"
#endif

#include "pathDiscovery/PathElement.hpp"
#include "pathDiscovery/ResourcePaths.hpp"
#include "platformInfo/ResourceLocation.hpp"
#include "resourceMetadata/ResourceTier.hpp"
#include "resourceMetadata/ResourceTypeInfo.hpp"

using namespace pathDiscovery;
using namespace platformInfo;
using namespace resourceMetadata;

void printUsage(QTextStream& out) {
    out << "\nUSAGE: test_sa_location_discovery [OPTIONS] [appname]\n\n";
    out << "Demonstrates ResourceLocation discovery from qualified paths.\n\n";
    out << "ARGUMENTS:\n";
    out << "  appname           Application name for resource discovery (default: TestDiscovery)\n\n";
    out << "OPTIONS:\n";
    out << "  -h, --help        Show this help message\n";
    out << "  --usage           Show this help message\n\n";
    out << "EXAMPLES:\n";
    out << "  test_sa_location_discovery\n";
    out << "  test_sa_location_discovery OpenSCAD\n";
    out << "  test_sa_location_discovery --help\n\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    
    // Check for help options
    if (argc > 1) {
        QString arg1 = QString::fromUtf8(argv[1]).toLower();
        if (arg1 == "-h" || arg1 == "--help" || arg1 == "--usage") {
            printUsage(out);
            return 0;
        }
    }
    
    // Set test application name for resource discovery
    // Override from testAppNameInfo.hpp when USE_TEST_APP_INFO is defined
#ifdef USE_TEST_APP_INFO
    // Accept app name from command line argument, default to "TestDiscovery"
    QString testAppName = (argc > 1) ? QString::fromUtf8(argv[1]) : QStringLiteral("TestDiscovery");
    appInfo::setTestAppName(testAppName);
#endif
    
    QCoreApplication::setOrganizationName(QStringLiteral("OpenSCAD"));
    QCoreApplication::setApplicationVersion(QStringLiteral("2.97.0"));
    
    // ========================================================================
    // HEADER
    // ========================================================================
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "RESOURCE LOCATION DISCOVERY TEST\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
#ifdef USE_TEST_APP_INFO
    out << QString("Application Name: %1\n").arg(appInfo::getBaseName());
    out << QString("(Override via: test_sa_location_discovery <appname>)\n\n");
#endif
    
    out << "Purpose: Demonstrate ResourceLocation discovery from qualified paths\n";
    out << "Input:   Qualified search paths (with tier markers)\n";
    out << "Output:  ResourceLocation objects grouped by tier\n";
    out << "         (Shows which locations exist and are discoverable)\n\n";
    
    // ========================================================================
    // STEP 1: Get Qualified Search Paths
    // ========================================================================
    out << "STEP 1: Getting Qualified Search Paths\n";
    out << "───────────────────────────────────────────────────────────────────\n\n";
    
    ResourcePaths rp;
    QList<PathElement> qualifiedPaths = rp.qualifiedSearchPaths();
    
    out << QString("Found %1 qualified search paths\n\n").arg(qualifiedPaths.size());
    
    // ========================================================================
    // STEP 2: Convert PathElements to ResourceLocations
    // ========================================================================
    out << "STEP 2: Converting to ResourceLocations\n";
    out << "───────────────────────────────────────────────────────────────────\n\n";
    
    QList<ResourceLocation> allLocations;
    
    for (const PathElement& pe : qualifiedPaths) {
        // Create ResourceLocation from PathElement
        ResourceLocation loc(pe.path(), pe.tier());
        
        // Only add if directory exists
        QFileInfo info(pe.path());
        if (info.exists() && info.isDir()) {
            allLocations.append(loc);
            out << QString("✓ %1\n").arg(pe.path());
        } else {
            out << QString("✗ %1 (not found)\n").arg(pe.path());
        }
    }
    
    out << QString("\nDiscovered %1 existing locations\n\n").arg(allLocations.size());
    
    // ========================================================================
    // STEP 3: Group by Tier and Display
    // ========================================================================
    out << "STEP 3: Locations Grouped by Tier\n";
    out << "───────────────────────────────────────────────────────────────────\n\n";
    
    // Group locations by tier using QMap
    QMap<ResourceTier, QList<ResourceLocation>> tierGroups;
    for (const ResourceLocation& loc : allLocations) {
        tierGroups[loc.tier()].append(loc);
    }
    
    for (ResourceTier tier : s_allTiersList) {
        const QList<ResourceLocation>& locations = tierGroups.value(tier);
        QString tierName = tierToString(tier).toUpper();
        
        out << QString("📁 %1 Tier (%2 locations)\n").arg(tierName).arg(locations.size());
        out << "───────────────────────────────────────────────────────────────────\n";

        for (const ResourceLocation& loc : locations) {
            out << QString("   • %1\n").arg(loc.path());
            out << QString("     Display: %1\n").arg(loc.getDisplayName());
            
            // Check for valid resource folders only
            QDir dir(loc.path());
            QStringList allSubDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            
            // Filter to only show known resource folders (case-sensitive)
            QStringList resourceFolders;
            QStringList wrongCaseFolders;
            
            for (const QString& subDir : allSubDirs) {
                if (s_allResourceFolders.contains(subDir)) {
                    // Exact match - correct folder name
                    resourceFolders.append(subDir);
                } else {
                    // Check for case-insensitive match (wrong case)
                    for (const QString& correctName : s_allResourceFolders) {
                        if (subDir.compare(correctName, Qt::CaseInsensitive) == 0) {
                            wrongCaseFolders.append(QString("%1 (should be: %2)").arg(subDir, correctName));
                            break;
                        }
                    }
                }
            }
            
            if (!resourceFolders.isEmpty()) {
                out << QString("     Resource folders: %1\n").arg(resourceFolders.join(", "));
            } else {
                out << QString("     (no resource folders found)\n");
            }
            
            if (!wrongCaseFolders.isEmpty()) {
                out << QString("     ⚠️  Wrong case detected: %1\n").arg(wrongCaseFolders.join(", "));
                out << QString("     Note: Folder names are case-sensitive and will be rejected by scanner\n");
            }
        }
        out << "\n";
    }
    
    // ========================================================================
    // SUMMARY
    // ========================================================================
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "SUMMARY\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    out << QString("Total Qualified Paths:    %1\n").arg(qualifiedPaths.size());
    out << QString("Existing Locations:       %1\n").arg(allLocations.size());
    
    for (ResourceTier tier : s_allTiersList) {
        QString tierName = tierToString(tier);
        out << QString("  %1 Tier:              %2\n")
               .arg(tierName, -14)
               .arg(tierGroups.value(tier).size());
    }
    
    out << "\nNEXT STEPS:\n";
    out << "• ResourceScanner should use this location list for inventory building\n";
    out << "• Each location can have multiple resource types (templates, examples, etc.)\n";
    out << "• Phase 5: Implement LibrariesInventory alongside ExamplesInventory/TemplatesInventory\n\n";
    
    return 0;
}
