/**
 * @file test_sa_discovery_inventory.cpp
 * @brief Standalone test showing all resources discovered across all locations
 * 
 * Demonstrates resource discovery workflow using resourceManager():
 * 1. Get qualified search paths (with tier markers)
 * 2. Create ResourceLocations from valid paths
 * 3. For each location, iterate resource folders using QDirListing filter
 * 4. Call addFolder() methods which apply type-specific filtering
 * 5. Display populated inventories
 * 
 * This shows what actually gets populated into the inventories after discovery.
 */

#include <QCoreApplication>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QDirListing>

#ifdef USE_TEST_APP_INFO
#include "ApplicationNameInfo.hpp"
#else
#include "applicationNameInfo.hpp"
#endif

#include "pathDiscovery/PathElement.hpp"
#include "pathDiscovery/ResourcePaths.hpp"
#include "platformInfo/ResourceLocation.hpp"
#include "resourceMetadata/ResourceTier.hpp"
#include "resourceMetadata/ResourceTypeInfo.hpp"
#include "resourceInventory/TemplatesInventory.hpp"
#include "resourceInventory/ExamplesInventory.hpp"

using namespace pathDiscovery;
using namespace platformInfo;
using namespace resourceMetadata;
using namespace resourceInventory;

// Global inventory pointers (same as main.cpp)
static TemplatesInventory* g_templatesInventory = nullptr;
static ExamplesInventory* g_examplesInventory = nullptr;

void printUsage(QTextStream& out, const QString& progName) {
    out << "\nUSAGE: " << progName << " [OPTIONS] [appname]\n\n";
    out << "Demonstrates resource discovery and inventory population.\n\n";
    out << "ARGUMENTS:\n";
    out << "  appname           Application name for resource discovery (default: scadtemplates)\n\n";
    out << "OPTIONS:\n";
    out << "  -h, --help        Show this help message\n\n";
    out << "EXAMPLES:\n";
    out << "  " << progName << "\n";
    out << "  " << progName << " OpenSCAD\n";
    out << "  " << progName << " --help\n\n";
}

// Dispatch table: maps ResourceType to addFolder method
std::function<int(const QDirListing::DirEntry&, const ResourceLocation&)> 
getAddFolderFunction(ResourceType resType)
{
    static QMap<ResourceType, std::function<int(const QDirListing::DirEntry&, const ResourceLocation&)>> dispatchMap = {
        {ResourceType::Templates, [](const QDirListing::DirEntry& dirEntry, const ResourceLocation& loc) {
            return g_templatesInventory->addFolder(dirEntry, loc);
        }},
        {ResourceType::Examples, [](const QDirListing::DirEntry& dirEntry, const ResourceLocation& loc) {
            return g_examplesInventory->addFolder(dirEntry, loc);
        }},
    };
    
    auto it = dispatchMap.find(resType);
    if (it != dispatchMap.end()) {
        return it.value();
    }
    
    // Unknown type
    return [](const QDirListing::DirEntry&, const ResourceLocation&) {
        qWarning() << "Unknown resource type - no handler";
        return 0;
    };
}

int resourceDiscovery()
{
    try {
        ResourcePaths pathDiscovery;
        QList<PathElement> discoveredPaths = pathDiscovery.qualifiedSearchPaths();
        
        for (const auto& pathElem : discoveredPaths) {
            if (ResourceLocation::locationHasResource(pathElem)) {
                const ResourceLocation location(pathElem);
                for (const auto &dirEntry : QDirListing(pathElem.path()
                    , s_allResourceFolders
                    , QDirListing::IteratorFlag::DirsOnly)) {
                    ResourceType resType = ResourceTypeInfo::getResourceTypeFromFolderName(dirEntry.fileName());
                    getAddFolderFunction(resType)(dirEntry, location);
                }
            }
        }
        
        return 0;
    } catch (const std::exception& e) {
        qCritical() << "Resource discovery failed:" << e.what();
        return 1;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    
    QString progName = QString::fromUtf8(argv[0]);
    int lastSlash = progName.lastIndexOf('/');
    if (lastSlash >= 0) {
        progName = progName.mid(lastSlash + 1);
    }
    lastSlash = progName.lastIndexOf('\\');
    if (lastSlash >= 0) {
        progName = progName.mid(lastSlash + 1);
    }
    
    // Check for help options
    if (argc > 1) {
        QString arg1 = QString::fromUtf8(argv[1]).toLower();
        if (arg1 == "-h" || arg1 == "--help") {
            printUsage(out, progName);
            return 0;
        }
    }
    
    // Get app name from command line or use default
    QString appName = QStringLiteral("scadtemplates");
    if (argc > 1) {
        appName = QString::fromUtf8(argv[1]);
    }
    
    // Set test application name for resource discovery
#ifdef USE_TEST_APP_INFO
    appInfo::setTestAppName(appName);
#endif
    
    QCoreApplication::setOrganizationName(QStringLiteral("OpenSCAD"));
    QCoreApplication::setApplicationVersion(QStringLiteral("2.97.0"));
    
    // ========================================================================
    // HEADER
    // ========================================================================
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "DISCOVERY INVENTORY TEST\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    out << QString("Application: %1\n").arg(appName);
    out << "Using resourceManager() to discover and populate inventories\n\n";
    
    // ========================================================================
    // Initialize inventories
    // ========================================================================
    g_templatesInventory = new TemplatesInventory();
    g_examplesInventory = new ExamplesInventory();
    
    // ========================================================================
    // Run discovery
    // ========================================================================
    out << "STEP 1: Running Resource Discovery\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    out << "Code: resourceDiscovery() → getAddFolderFunction() → inventory.addFolder()\n";
    out << "      TemplatesInventory::addFolder() filters *.json via QDirListing\n";
    out << "      ExamplesInventory::addFolder() filters *.scad/*.csg via QDirListing\n\n";
    
    int result = resourceDiscovery();
    if (result != 0) {
        qCritical() << "Resource discovery failed";
        return result;
    }
    
    // ========================================================================
    // Display inventory contents
    // ========================================================================
    out << "STEP 2: Populated Inventories\n";
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "Code: inventory.getAll() → Returns only successfully added resources\n";
    out << "Note: Failed additions (validation errors, etc.) are NOT shown here\n";
    out << "      Check STEP 1 output for warning messages about failures\n\n";
    
    // Templates
    out << "📋 TEMPLATES INVENTORY\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    
    QList<QVariant> allTemplates = g_templatesInventory->getAll();
    if (allTemplates.isEmpty()) {
        out << "  (no templates found)\n";
    } else {
        out << QString("  Found: %1 template(s)\n\n").arg(allTemplates.size());
        for (const QVariant& var : allTemplates) {
            ResourceTemplate tmpl = var.value<ResourceTemplate>();
            out << QString("    • %1\n").arg(tmpl.displayName());
            out << QString("      ID: %1\n").arg(tmpl.uniqueID());
            out << QString("      Path: %1\n").arg(tmpl.path());
            out << QString("      Tier: %1\n\n").arg(tierToString(tmpl.tier()));
        }
    }
    
    // Examples
    out << "📋 EXAMPLES INVENTORY\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    
    QList<QVariant> allExamples = g_examplesInventory->getAll();
    if (allExamples.isEmpty()) {
        out << "  (no examples found)\n";
    } else {
        out << QString("  Found: %1 example(s)\n\n").arg(allExamples.size());
        for (const QVariant& var : allExamples) {
            ResourceScript example = var.value<ResourceScript>();
            out << QString("    • %1\n").arg(example.displayName());
            out << QString("      Path: %1\n").arg(example.path());
            out << QString("      Tier: %1\n").arg(tierToString(example.tier()));
            out << QString("      Category: %1\n\n").arg(example.category());
        }
    }
    
    // ========================================================================
    // SUMMARY
    // ========================================================================
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "SUMMARY\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    out << QString("Templates: %1\n").arg(allTemplates.size());
    out << QString("Examples:  %1\n").arg(allExamples.size());
    out << QString("Total:     %1\n\n").arg(allTemplates.size() + allExamples.size());
    
    // Cleanup
    delete g_templatesInventory;
    delete g_examplesInventory;
    
    return 0;
}
