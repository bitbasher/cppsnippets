/**
 * @file test_sa_examples_discovery.cpp
 * @brief Standalone test showing examples discovered via addFolder()
 * 
 * Demonstrates ExamplesInventory::addFolder() filtering:
 * - Scans for .scad and .csg files
 * - Auto-extracts category from folder name
 * - Detects and includes attachments (.txt, .md, etc.)
 * - Displays populated examples inventory
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
#include "resourceInventory/ExamplesInventory.hpp"

using namespace pathDiscovery;
using namespace platformInfo;
using namespace resourceMetadata;
using namespace resourceInventory;

// Global examples inventory
static ExamplesInventory* g_examplesInventory = nullptr;

void printUsage(QTextStream& out, const QString& progName) {
    out << "\nUSAGE: " << progName << " [OPTIONS] [appname]\n\n";
    out << "Demonstrates example discovery and filtering.\n\n";
    out << "ARGUMENTS:\n";
    out << "  appname           Application name for resource discovery (default: scadtemplates)\n\n";
    out << "OPTIONS:\n";
    out << "  -h, --help        Show this help message\n\n";
    out << "EXAMPLES:\n";
    out << "  " << progName << "\n";
    out << "  " << progName << " OpenSCAD\n";
    out << "  " << progName << " --help\n\n";
}

int discoverExamples()
{
    try {
        ResourcePaths pathDiscovery;
        QList<PathElement> discoveredPaths = pathDiscovery.qualifiedSearchPaths();
        
        for (const auto& pathElem : discoveredPaths) {
            if (ResourceLocation::locationHasResource(pathElem)) {
                const ResourceLocation location(pathElem);
                for (const auto &dirEntry : QDirListing(pathElem.path()
                    , QStringList{QStringLiteral("examples")}
                    , QDirListing::IteratorFlag::DirsOnly)) {
                    // Scan examples folder and add each example
                    g_examplesInventory->addFolder(dirEntry, location);
                }
            }
        }
        
        return 0;
    } catch (const std::exception& e) {
        qCritical() << "Examples discovery failed:" << e.what();
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
    out << "EXAMPLES DISCOVERY TEST\n";
    out << "═══════════════════════════════════════════════════════════════════\n\n";
    
    out << QString("Application: %1\n").arg(appName);
    out << "Discovers .scad/.csg example files via ExamplesInventory::addFolder()\n\n";
    
    // ========================================================================
    // Initialize inventory
    // ========================================================================
    g_examplesInventory = new ExamplesInventory();
    
    // ========================================================================
    // Run discovery
    // ========================================================================
    out << "STEP 1: Discovering Examples\n";
    out << "───────────────────────────────────────────────────────────────────\n";
    out << "Code: ExamplesInventory::addFolder() filters *.scad/*.csg via QDirListing\n";
    out << "      Auto-extracts category from folder name, scans for attachments\n\n";
    
    int result = discoverExamples();
    if (result != 0) {
        qCritical() << "Examples discovery failed";
        return result;
    }
    
    // ========================================================================
    // Display results
    // ========================================================================
    out << "STEP 2: Discovered Examples\n";
    out << "═══════════════════════════════════════════════════════════════════\n";
    out << "Code: inventory.getAll() → Returns only successfully added examples\n\n";
    
    QList<QVariant> allExamples = g_examplesInventory->getAll();
    
    // Group by tier
    QMap<ResourceTier, QList<QVariant>> tierGroups;
    for (const QVariant& var : allExamples) {
        ResourceScript example = var.value<ResourceScript>();
        tierGroups[example.tier()].append(var);
    }
    
    if (allExamples.isEmpty()) {
        out << "  (no examples found)\n";
    } else {
        out << QString("Found: %1 example(s)\n\n").arg(allExamples.size());
        
        for (ResourceTier tier : s_allTiersList) {
            const QList<QVariant>& examples = tierGroups.value(tier);
            if (examples.isEmpty()) {
                continue;
            }
            
            QString tierName = tierToString(tier).toUpper();
            out << QString("📁 %1 TIER (%2 examples)\n").arg(tierName).arg(examples.size());
            out << "───────────────────────────────────────────────────────────────────\n";
            
            for (const QVariant& var : examples) {
                ResourceScript example = var.value<ResourceScript>();
                out << QString("  • %1\n").arg(example.displayName());
                out << QString("    Category: %1\n").arg(example.category());
                out << QString("    Path: %1\n").arg(example.path());
                if (example.hasAttachments()) {
                    out << QString("    Attachments: yes\n");
                }
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
    
    out << QString("Total Examples: %1\n\n").arg(allExamples.size());
    
    for (ResourceTier tier : s_allTiersList) {
        const QList<QVariant>& examples = tierGroups.value(tier);
        QString tierName = tierToString(tier);
        out << QString("  %1 Tier: %2\n").arg(tierName, -14).arg(examples.size());
    }
    
    out << "\nFilter: .scad and .csg files\n";
    out << "Category: Auto-extracted from folder name\n";
    out << "Attachments: Automatically detected and included\n\n";
    
    // Cleanup
    delete g_examplesInventory;
    
    return 0;
}
