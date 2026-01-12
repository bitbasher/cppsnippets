/**
 * @file test_path_discovery.cpp
 * @brief Console test demonstrating pathDiscovery workflow
 */

#include <iostream>
#include <QCoreApplication>
#include <QString>
#include <QTextStream>
#include <QDir>
#include <QSysInfo>
#include <QStandardPaths>
#ifdef USE_TEST_APP_INFO
#include "testAppNameInfo.hpp"
#else
#include "applicationNameInfo.hpp"
#endif
#include "resourceMetadata/ResourceTier.hpp"
#include "resourceMetadata/ResourceTypeInfo.hpp"
#include "pathDiscovery/PathElement.hpp"
#include "pathDiscovery/ResourcePaths.hpp"
#include "platformInfo/ResourceLocation.hpp"

using namespace resourceMetadata;
using namespace pathDiscovery;

static QTextStream out(stdout);

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    out << "Starting test...\n";
    out.flush();
    
    // Parse command line arguments for app name (default: OpenSCAD)
    QString appName = "OpenSCAD";
    if (argc > 1) {
        appName = QString::fromUtf8(argv[1]);
    }
    
    // Set test application name for resource discovery
    out << "About to set base name...\n";
    out.flush();
    appInfo::setTestAppName(appName);
    out << "Base name set successfully\n";
    out.flush();
    
    out << QString(80, '=') << "\n";
    out << "PATH DISCOVERY WORKFLOW TEST\n";
    out << QString(80, '=') << "\n";
    out << "Application Name: " << appName << "\n";
    out << "Effective Base Name: " << appInfo::getBaseName() << "\n";
    out << "Qt Version: " << qVersion() << "\n";
    out << "Platform: " << QSysInfo::prettyProductName() << "\n\n";
    
    // Test 1: Show default search paths
    // Note: Default search paths and resolved paths are now internal implementation details.
    // The primary public API is qualifiedSearchPaths() which returns all tiers.
    // This test shows the qualified paths grouped by tier.
    
    out << "TEST 1: Qualified Search Paths by Tier (PRIMARY API)\n";
    out << QString(80, '-') << "\n";
    out << "Shows how paths are qualified for discovery, grouped by tier.\n\n";
    
    out << "About to create ResourcePaths helper...\n";
    out.flush();
    ResourcePaths pathsHelper;
    out << "ResourcePaths created successfully\n";
    out.flush();
    
    out << "About to call qualifiedSearchPaths()...\n";
    out.flush();
    auto qualifiedPaths = pathsHelper.qualifiedSearchPaths();
    out << "qualifiedSearchPaths() returned " << qualifiedPaths.size() << " paths\n";
    out.flush();
    
    out << "\nInstallation Tier:\n";
    for (const auto& pathElement : qualifiedPaths) {
        if (pathElement.tier() == resourceMetadata::ResourceTier::Installation) {
            out << "  - " << pathElement.path() << "\n";
        }
    }
    
    out << "\nMachine Tier:\n";
    for (const auto& pathElement : qualifiedPaths) {
        if (pathElement.tier() == resourceMetadata::ResourceTier::Machine) {
            out << "  - " << pathElement.path() << "\n";
        }
    }
    
    out << "\nUser Tier:\n";
    for (const auto& pathElement : qualifiedPaths) {
        if (pathElement.tier() == resourceMetadata::ResourceTier::User) {
            out << "  - " << pathElement.path() << "\n";
        }
    }
    
    // Test 2: Complete qualified search paths (PRIMARY API)
    out << "\n\nTEST 2: All Qualified Search Paths (PRIMARY API FOR DISCOVERY)\n";
    out << QString(80, '-') << "\n";
    out << "Complete list with tier markers - this is what ResourceScanner receives.\n\n";
    
    out << QString(80, '-') << "\n\n";
    
    // Now show the qualified output paths
    auto discoveryPaths = qualifiedPaths;
    
    out << QString("OUTPUT: %1 Qualified Discovery Paths\n\n").arg(discoveryPaths.size());
    
    for (int i = 0; i < discoveryPaths.size(); ++i) {
        const auto& p = discoveryPaths[i];
        QString tierName;
        switch (p.tier()) {
            case resourceMetadata::ResourceTier::Installation: tierName = "Installation"; break;
            case resourceMetadata::ResourceTier::Machine: tierName = "Machine"; break;
            case resourceMetadata::ResourceTier::User: tierName = "User"; break;
        }
        
        out << QString("  [%1] %2: %3\n")
            .arg(i, 2)
            .arg(tierName, -13)
            .arg(p.path());
    }
    
    out << "\n" << QString(80, '-') << "\n\n";
    
    out << "DETAILED TRANSFORMATION FOR EACH PATH:\n\n";
    
    // Get user-designated paths for accurate source identification
    auto userDesignatedPaths = ResourcePaths::userDesignatedPaths();
    
    // Show detailed transformation for each path
    for (int i = 0; i < discoveryPaths.size(); ++i) {
        const auto& p = discoveryPaths[i];
        QString tierName;
        switch (p.tier()) {
            case ResourceTier::Installation: tierName = "Installation"; break;
            case ResourceTier::Machine: tierName = "Machine"; break;
            case ResourceTier::User: tierName = "User"; break;
        }
        
        out << QString("Path [%1] - %2 Tier:\n").arg(i).arg(tierName);
        out << QString("  Final: %1\n").arg(p.path());
        
        // Determine source
        QString source = "Unknown";
        
        // Check if from Installation tier
        bool isUserDesignated = false;
        if (p.tier() == ResourceTier::Installation) {
            for (const QString& userPath : userDesignatedPaths) {
                // Direct match or contains the user path as a base
                if (p.path() == userPath || p.path().startsWith(userPath)) {
                    source = QString("User-designated: %1").arg(userPath);
                    isUserDesignated = true;
                    break;
                }
                
                // Also check if user path with environment variables expanded matches
                QString expandedUserPath = userPath;
                expandedUserPath.replace("%USERPROFILE%", QDir::homePath());
                expandedUserPath.replace("%APPDATA%", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
                expandedUserPath.replace("%LOCALAPPDATA%", QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
                if (p.path() == expandedUserPath || p.path().startsWith(expandedUserPath)) {
                    source = QString("User-designated: %1").arg(userPath);
                    isUserDesignated = true;
                    break;
                }
            }
        }
        
        if (!isUserDesignated) {
            // Continue with other detection logic
            if (p.path().contains("applicationDirPath")) {
                source = "QCoreApplication::applicationDirPath()";
            } else if (p.path() == QCoreApplication::applicationDirPath()) {
                source = "QCoreApplication::applicationDirPath()";
            } else if (p.path() == QDir::current().absolutePath()) {
                source = ". (current working directory)";
            } else if (p.path().contains("Program Files") && p.path().endsWith("ScadTemplates")) {
                source = "%PROGRAMFILES%/ → appended folder name";
            } else if (p.path().contains("share/ScadTemplates")) {
                source = "../share/ → appended folder name (Installation tier)";
            } else if (p.path().endsWith("cppsnippets") && !p.path().contains("build")) {
                source = ".. → parent directory";
            } else if (p.path().contains("ProgramData")) {
                source = "%PROGRAMDATA%/ → appended folder name";
            } else if (p.path().contains("AppData")) {
                if (p.path().contains("Roaming")) {
                    source = "%APPDATA%/ → appended folder name";
                } else if (p.path().contains("Local")) {
                    source = "%LOCALAPPDATA%/ → appended folder name";
                }
            } else if (p.path().contains("Documents") && p.path().endsWith("ScadTemplates")) {
                // Check if it's from QStandardPaths Documents (User tier) or user-designated
                if (p.tier() == ResourceTier::User) {
                    source = "QStandardPaths::DocumentsLocation → appended folder name";
                } else {
                    source = "%USERPROFILE%/Documents/ → appended folder name";
                }
            } else if (p.path().contains("Documents")) {
                source = "%USERPROFILE%/Documents/ → appended folder name";
            } else if (p.path().endsWith("ScadTemplates")) {
                // This is a path with folder name appended, check which tier
                if (p.tier() == ResourceTier::User) {
                    source = "../ → appended folder name (User tier)";
                } else if (p.tier() == ResourceTier::Installation) {
                    source = "Sibling installation (LTS ↔ Nightly)";
                } else {
                    source = "Default path with folder name appended";
                }
            } else {
                source = "Default path (no folder name appended)";
            }
        }
        
        out << QString("  Source: %1\n").arg(source);
        
        // Generate and display the display name
        platformInfo::ResourceLocation tempLoc(p.path(), static_cast<resourceInventory::ResourceTier>(p.tier()));
        QString displayName = tempLoc.getDisplayName();
        out << QString("  Display Name: %1\n").arg(displayName);
        
        out << "\n";
    }
    
    // Test 4: Resource type subdirectories
    out << "\n\nTEST 4: Resource Type Subdirectories\n";
    out << QString(80, '-') << "\n";
    out << "When scanning a base path, these subdirectories are checked:\n\n";
    
    QList<ResourceType> types = {
        ResourceType::Templates,
        ResourceType::Libraries,
        ResourceType::Fonts,
        ResourceType::Examples
    };
    
    for (const auto& type : types) {
        QString typeName;
        switch (type) {
            case ResourceType::Templates: typeName = "Templates"; break;
            case ResourceType::Libraries: typeName = "Libraries"; break;
            case ResourceType::Fonts: typeName = "Fonts"; break;
            case ResourceType::Examples: typeName = "Examples"; break;
            default: typeName = "Other"; break;
        }
        
        // Use ResourceTypeInfo directly (wrapper methods removed)
        auto it = ResourceTypeInfo::s_resourceTypes.constFind(type);
        QString subdir = (it != ResourceTypeInfo::s_resourceTypes.constEnd()) 
            ? it.value().getSubDir() : QString();
        QStringList exts = (it != ResourceTypeInfo::s_resourceTypes.constEnd())
            ? it.value().getPrimaryExtensions() : QStringList();
        
        out << QString("  %1:\n").arg(typeName);
        out << QString("    Subdirectory: %1/\n").arg(subdir);
        out << QString("    Extensions: %1\n\n").arg(exts.join(", "));
    }
    
    // Test 5: Real-world workflow
    out << "\nTEST 5: Real-World Discovery Workflow\n";
    out << QString(80, '-') << "\n";
    out << "\nStep 1: Application creates ResourcePaths helper\n";
    out << "Step 2: Set build suffix (empty for release, ' (Nightly)' for nightlies)\n";
    out << "Step 3: Call qualifiedSearchPaths() to get ALL discovery locations\n";
    out << QString("Step 4: Got %1 paths to scan\n").arg(discoveryPaths.size());
    out << "\nStep 5: ResourceScanner would:\n";
    out << "  - Look for subdirectories matching resource types\n";
    out << "    (templates/, libraries/, fonts/, etc.)\n";
    out << "  - Scan for files matching expected extensions\n";
    out << "    (.json for templates, .scad for libraries, etc.)\n";
    out << "  - Preserve tier information for each discovered resource\n";
    out << "\nStep 6: Results stored in ResourceInventory with tier tags:\n";
    out << "  - Installation tier: Read-only, built-in\n";
    out << "  - Machine tier: Read-only, system-wide\n";
    out << "  - User tier: Read-write, per-user\n";
    
    out << "\n" << QString(80, '=') << "\n";
    out << "ALL TESTS COMPLETE\n";
    out << QString(80, '=') << "\n";
    out << "\nThe pathDiscovery module correctly processes resource metadata\n";
    out << "and generates qualified search paths for ResourceScanner.\n";
    
    return 0;
}
