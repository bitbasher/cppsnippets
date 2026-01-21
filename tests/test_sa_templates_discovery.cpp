/**
 * @file test_sa_templates_discovery.cpp
 * @brief Standalone test showing templates discovered via addFolder()
 * 
 * Demonstrates TemplatesInventory::addFolder() filtering:
 * - Scans for .json files only
 * - Validates JSON format
 * - Generates unique IDs
 * - Displays populated template inventory
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

using namespace pathDiscovery;
using namespace platformInfo;
using namespace resourceMetadata;
using namespace resourceInventory;

// Global templates inventory
static TemplatesInventory* g_templatesInventory = nullptr;

void printUsage(QTextStream& out, const QString& progName) {
    out << "\nUSAGE: " << progName << " [OPTIONS] [appname]\n\n";
    out << "Demonstrates template discovery and filtering.\n\n";
    out << "ARGUMENTS:\n";
    out << "  appname           Application name for resource discovery (default: scadtemplates)\n\n";
    out << "OPTIONS:\n";
    out << "  -h, --help        Show this help message\n\n";
    out << "EXAMPLES:\n";
    out << "  " << progName << "\n";
    out << "  " << progName << " OpenSCAD\n";
    out << "  " << progName << " --help\n\n";
}

int discoverTemplates()
{
    try {
        ResourcePaths pathDiscovery;
        QList<PathElement> discoveredPaths = pathDiscovery.qualifiedSearchPaths();
        
        for (const auto& pathElem : discoveredPaths) {
            if (ResourceLocation::locationHasResource(pathElem)) {
                const ResourceLocation location(pathElem);
                for (const auto &dirEntry : QDirListing(pathElem.path()
                    , QStringList{QStringLiteral("templates")}
                    , QDirListing::IteratorFlag::DirsOnly)) {
                    // Scan templates folder and add each template
                    g_templatesInventory->addFolder(dirEntry, location);
                }
            }
        }
        
        return 0;
    } catch (const std::exception& e) {
        qCritical() << "Template discovery failed:" << e.what();
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
    out << "TEMPLATES DISCOVERY TEST\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    out << QString("Application: %1\n").arg(appName);
    out << "Discovers .json template files via TemplatesInventory::addFolder()\n\n";
    
    // ========================================================================
    // Initialize inventory
    // ========================================================================
    g_templatesInventory = new TemplatesInventory();
    
    // ========================================================================
    // Run discovery
    // ========================================================================
    out << "STEP 1: Discovering Templates\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    out << "Code: TemplatesInventory::addFolder() filters *.json via QDirListing\n";
    out << "      Then calls addTemplate() which validates JSON format\n\n";
    
    int result = discoverTemplates();
    if (result != 0) {
        qCritical() << "Template discovery failed";
        return result;
    }
    
    // ========================================================================
    // Display results
    // ========================================================================
    out << "STEP 2: Discovered Templates\n";
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "Code: inventory.getAll() → Returns only successfully added templates\n";
    out << "Note: Files with validation errors are NOT shown (check STEP 1 warnings)\n\n";
    
    QList<QVariant> allTemplates = g_templatesInventory->getAll();
    
    // Group by tier
    QMap<ResourceTier, QList<QVariant>> tierGroups;
    for (const QVariant& var : allTemplates) {
        ResourceTemplate tmpl = var.value<ResourceTemplate>();
        tierGroups[tmpl.tier()].append(var);
    }
    
    if (allTemplates.isEmpty()) {
        out << "  (no templates found)\n";
    } else {
        out << QString("Found: %1 template(s)\n\n").arg(allTemplates.size());
        
        for (ResourceTier tier : s_allTiersList) {
            const QList<QVariant>& templates = tierGroups.value(tier);
            if (templates.isEmpty()) {
                continue;
            }
            
            QString tierName = tierToString(tier).toUpper();
            out << QString("📁 %1 TIER (%2 templates)\n").arg(tierName).arg(templates.size());
            out << "───────────────────────────────────────────────────────────────────\n";
            
            for (const QVariant& var : templates) {
                ResourceTemplate tmpl = var.value<ResourceTemplate>();
                out << QString("  • %1\n").arg(tmpl.displayName());
                out << QString("    ID: %1\n").arg(tmpl.uniqueID());
                out << QString("    Path: %1\n").arg(tmpl.path());
            }
            
            out << "\n";
        }
    }
    
    // ========================================================================
    // SUMMARY
    // ========================================================================
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "SUMMARY\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    out << QString("Total Templates: %1\n\n").arg(allTemplates.size());
    
    for (ResourceTier tier : s_allTiersList) {
        const QList<QVariant>& templates = tierGroups.value(tier);
        QString tierName = tierToString(tier);
        out << QString("  %1 Tier: %2\n").arg(tierName, -14).arg(templates.size());
    }
    
    out << "\nFilter: .json files only\n";
    out << "Validation: JSON schema and format checking applied\n\n";
    
    // Cleanup
    delete g_templatesInventory;
    
    return 0;
}
