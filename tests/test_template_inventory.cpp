/**
 * @file test_template_inventory.cpp
 * @brief Display template inventory with location and source info
 * 
 * Scans all resource locations and displays templates grouped by location.
 * Shows display name, raw path, resolved path, and template details.
 */

#include <QCoreApplication>
#include <QTextStream>
#include <QDir>

#ifdef USE_TEST_APP_INFO
#include "testAppNameInfo.hpp"
#else
#include "applicationNameInfo.hpp"
#endif
#include "pathDiscovery/ResourcePaths.hpp"
#include "platformInfo/ResourceLocation.hpp"
#include "resourceDiscovery/templateScanner.hpp"
#include "resourceInventory/resourceItem.hpp"

using pathDiscovery::ResourcePaths;
using platformInfo::ResourceLocation;
using resourceInventory::ResourceTemplate;
using resourceInventory::ResourceTier;

QString tierToString(ResourceTier tier) {
    switch(tier) {
        case ResourceTier::Installation: return "Installation";
        case ResourceTier::Machine: return "Machine";
        case ResourceTier::User: return "User";
        default: return "Unknown";
    }
}

void displayTemplateInventory(QTextStream& out)
{
    out << "==========================================================\n";
    out << "Template Inventory Report\n";
    out << "==========================================================\n\n";
    
    // Phase 1: Discover and qualify paths
    ResourcePaths paths;
    auto qualifiedPaths = paths.qualifiedSearchPaths();
    
    out << "Discovery Summary:\n";
    out << "  Total Locations: " << qualifiedPaths.size() << "\n\n";
    
    // Create ResourceLocation objects from PathElements
    QVector<ResourceLocation> allLocations;
    
    for (const auto& pe : qualifiedPaths) {
        // Cast tier from resourceMetadata to resourceInventory enum
        auto tier = static_cast<ResourceTier>(pe.tier());
        
        ResourceLocation loc(
            pe.path(),          // resolved path
            tier,               // tier (cast to resourceInventory::ResourceTier)
            pe.path(),          // raw path (use same as resolved for now)
            QString(),          // displayName (auto-generated)
            QString()           // description
        );
        
        // Check if path exists and has templates folder
        QDir dir(pe.path());
        loc.setExists(dir.exists());
        loc.setWritable(dir.isReadable());
        
        QDir templatesDir(pe.path() + "/templates");
        loc.setHasResourceFolders(templatesDir.exists());
        
        allLocations.append(loc);
    }
    
    int totalTemplates = 0;
    int locationsWithTemplates = 0;
    
    // Phase 2: Scan each location for templates
    for (const auto& location : allLocations) {
        // Display location header
        out << "----------------------------------------------------------\n";
        out << "Location: " << location.displayName() << " [" << tierToString(location.tier()) << "]\n";
        
        if (!location.rawPath().isEmpty()) {
            out << "  Raw Path:      " << location.rawPath() << "\n";
        }
        out << "  Resolved Path: " << location.path() << "\n";
        out << "  Status:        ";
        
        if (!location.exists()) {
            out << "Does not exist\n\n";
            continue;
        }
        
        if (!location.hasResourceFolders()) {
            out << "No templates folder\n\n";
            continue;
        }
        
        out << "Active\n";
        
        // Scan for templates
        auto templates = TemplateScanner::scanLocation(location);
        
        if (templates.isEmpty()) {
            out << "  Templates:     (none found)\n\n";
            continue;
        }
        
        out << "  Templates:     " << templates.size() << " found\n";
        locationsWithTemplates++;
        totalTemplates += templates.size();
        
        // Display each template
        for (const auto& tmpl : templates) {
            out << "    • " << tmpl.name();
            
            if (!tmpl.category().isEmpty()) {
                out << " [" << tmpl.category() << "]";
            }
            
            if (!tmpl.source().isEmpty()) {
                out << " (source: " << tmpl.source() << ")";
            }
            
            out << "\n";
            
            // Show file path in verbose mode
            // out << "      File: " << tmpl.path() << "\n";
        }
        
        out << "\n";
    }
    
    // Summary
    out << "==========================================================\n";
    out << "Summary\n";
    out << "==========================================================\n";
    out << "  Total Locations:           " << allLocations.size() << "\n";
    out << "  Locations with Templates:  " << locationsWithTemplates << "\n";
    out << "  Total Templates Found:     " << totalTemplates << "\n";
    out << "\n";
}

int main(int argc, char *argv[])
{
    // Parse command line arguments for app name (default: OpenSCAD)
    QString appName = "OpenSCAD";
    if (argc > 1) {
        appName = QString::fromUtf8(argv[1]);
    }
    appInfo::setBaseName(appName);
    
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationVersion("1.0");
    
    QTextStream out(stdout);
    out << "Testing template discovery for application: " << appName << "\n\n";
    
    try {
        displayTemplateInventory(out);
        return 0;
    }
    catch (const std::exception& e) {
        out << "Error: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        out << "Unknown error occurred\n";
        return 1;
    }
}
