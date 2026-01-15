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

#include "pathDiscovery/PathElement.hpp"
#include "pathDiscovery/ResourcePaths.hpp"
#include "platformInfo/ResourceLocation.hpp"
#include "resourceMetadata/ResourceTier.hpp"

using namespace pathDiscovery;
using namespace platformInfo;
using namespace resourceMetadata;

// Helper to group locations by tier
struct TierLocations {
    ResourceTier tier;
    QList<ResourceLocation> locations;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set test application name for resource discovery
    // Override from applicationNameInfo.hpp via testAppNameInfo.hpp
    QCoreApplication::setApplicationName(QStringLiteral("ScadTemplates"));
    QCoreApplication::setOrganizationName(QStringLiteral("OpenSCAD"));
    QCoreApplication::setApplicationVersion(QStringLiteral("2.97.0"));
    
    QTextStream out(stdout);
    
    // ========================================================================
    // HEADER
    // ========================================================================
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "RESOURCE LOCATION DISCOVERY TEST\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
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
    
    // Separate by tier
    QList<TierLocations> tierGroups = {
        {ResourceTier::Installation, {}},
        {ResourceTier::Machine, {}},
        {ResourceTier::User, {}}
    };
    
    for (const ResourceLocation& loc : allLocations) {
        for (TierLocations& group : tierGroups) {
            if (loc.tier() == group.tier) {
                group.locations.append(loc);
                break;
            }
        }
    }
    
    // Display each tier
    for (const TierLocations& group : tierGroups) {
        QString tierName;
        switch (group.tier) {
            case ResourceTier::Installation: tierName = "INSTALLATION"; break;
            case ResourceTier::Machine:      tierName = "MACHINE"; break;
            case ResourceTier::User:         tierName = "USER"; break;
            default:                         tierName = "UNKNOWN"; break;
        }
        
        out << QString("📁 %1 Tier (%2 locations)\n").arg(tierName).arg(group.locations.size());
        out << "───────────────────────────────────────────────────────────────────\n";
        
        if (group.locations.isEmpty()) {
            out << "   (none found)\n";
        } else {
            for (const ResourceLocation& loc : group.locations) {
                out << QString("   • %1\n").arg(loc.path());
                out << QString("     Display: %1\n").arg(loc.getDisplayName());
                
                // Check for resource folders
                QDir dir(loc.path());
                QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                if (!subDirs.isEmpty()) {
                    out << QString("     Contains: %1\n").arg(subDirs.join(", "));
                }
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
    
    for (const TierLocations& group : tierGroups) {
        QString tierName;
        switch (group.tier) {
            case ResourceTier::Installation: tierName = "Installation"; break;
            case ResourceTier::Machine:      tierName = "Machine"; break;
            case ResourceTier::User:         tierName = "User"; break;
            default:                         tierName = "Unknown"; break;
        }
        out << QString("  %1 Tier:              %2\n")
               .arg(tierName, -14)
               .arg(group.locations.size());
    }
    
    out << "\nNEXT STEPS:\n";
    out << "• ResourceScanner should use this location list for inventory building\n";
    out << "• Each location can have multiple resource types (templates, examples, etc.)\n";
    out << "• Phase 5: Implement LibrariesInventory alongside ExamplesInventory/TemplatesInventory\n\n";
    
    return 0;
}
