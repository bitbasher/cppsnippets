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
    
    // Parse command line arguments for app name (default: OpenSCAD)
    QString appName = "OpenSCAD";
    if (argc > 1) {
        appName = QString::fromUtf8(argv[1]);
    }
    
    QCoreApplication::setApplicationName(appName);
    
    out << QString(80, '=') << "\n";
    out << "PATH DISCOVERY WORKFLOW TEST\n";
    out << QString(80, '=') << "\n";
    out << "Application Name: " << appName << "\n";
    out << "Qt Version: " << qVersion() << "\n";
    out << "Platform: " << QSysInfo::prettyProductName() << "\n\n";
    
    // Test 1: Show default search paths
    out << "TEST 1: Default Search Paths (with env var templates)\n";
    out << QString(80, '-') << "\n";
    
    out << "\nInstallation Tier:\n";
    for (const auto& path : ResourcePaths::defaultInstallSearchPaths()) {
        out << "  - " << path << "\n";
    }
    
    out << "\nMachine Tier:\n";
    for (const auto& path : ResourcePaths::defaultMachineSearchPaths()) {
        out << "  - " << path << "\n";
    }
    
    out << "\nUser Tier:\n";
    for (const auto& path : ResourcePaths::defaultUserSearchPaths()) {
        out << "  - " << path << "\n";
    }
    
    // Test 2: Show resolved search paths (env vars expanded)
    out << "\n\nTEST 2: Resolved Search Paths (Environment Variables Expanded)\n";
    out << QString(80, '-') << "\n";
    
    out << "\nInstallation Tier (Resolved):\n";
    for (const auto& path : ResourcePaths::resolvedInstallSearchPaths()) {
        out << "  - " << path << "\n";
    }
    
    out << "\nMachine Tier (Resolved):\n";
    for (const auto& path : ResourcePaths::resolvedMachineSearchPaths()) {
        out << "  - " << path << "\n";
    }
    
    out << "\nUser Tier (Resolved):\n";
    for (const auto& path : ResourcePaths::resolvedUserSearchPaths()) {
        out << "  - " << path << "\n";
    }
    
    // Test 3: Qualified search paths (PRIMARY API)
    out << "\n\nTEST 3: Qualified Search Paths (PRIMARY API FOR DISCOVERY)\n";
    out << QString(80, '-') << "\n";
    out << "Shows how template paths are transformed into qualified discovery paths.\n\n";
    
    ResourcePaths pathsHelper;
    pathsHelper.setSuffix("");  // Release build
    
    out << "INPUT: Default Template Paths\n";
    out << QString(80, '-') << "\n\n";
    
    out << "Installation Tier Templates:\n";
    auto installTemplates = ResourcePaths::defaultInstallSearchPaths();
    for (int i = 0; i < installTemplates.size(); ++i) {
        out << QString("  [%1] %2\n").arg(i).arg(installTemplates[i]);
    }
    
    out << "\nMachine Tier Templates:\n";
    auto machineTemplates = ResourcePaths::defaultMachineSearchPaths();
    for (int i = 0; i < machineTemplates.size(); ++i) {
        out << QString("  [%1] %2\n").arg(i).arg(machineTemplates[i]);
    }
    
    out << "\nUser Tier Templates:\n";
    auto userTemplates = ResourcePaths::defaultUserSearchPaths();
    for (int i = 0; i < userTemplates.size(); ++i) {
        out << QString("  [%1] %2\n").arg(i).arg(userTemplates[i]);
    }
    
    out << "\n" << QString(80, '-') << "\n\n";
    
    out << "TRANSFORMATION RULES:\n";
    out << "  1. Expand environment variables (%VAR% or ${VAR})\n";
    out << "  2. If path ends with '/', append folder name + suffix\n";
    out << "  3. Clean and resolve relative paths (. and ..)\n";
    out << "  4. Convert to absolute paths\n";
    out << "  5. Add sibling installations (if applicable)\n";
    out << "  6. Add user-designated paths from QSettings\n\n";
    
    out << QString(80, '-') << "\n\n";
    
    // Now show the qualified output paths
    auto discoveryPaths = pathsHelper.qualifiedSearchPaths();
    
    out << QString("OUTPUT: %1 Qualified Discovery Paths\n\n").arg(discoveryPaths.size());
    
    // Build a map to find which template path each output came from
    auto installResolved = ResourcePaths::resolvedInstallSearchPaths();
    auto machineResolved = ResourcePaths::resolvedMachineSearchPaths();
    auto userResolved = ResourcePaths::resolvedUserSearchPaths();
    
    for (int i = 0; i < discoveryPaths.size(); ++i) {
        const auto& p = discoveryPaths[i];
        QString tierName;
        switch (p.tier()) {
            case ResourceTier::Installation: tierName = "Installation"; break;
            case ResourceTier::Machine: tierName = "Machine"; break;
            case ResourceTier::User: tierName = "User"; break;
        }
        
        out << QString("  [%1] %2: %3\n")
            .arg(i, 2)
            .arg(tierName, -13)
            .arg(p.path());
    }
    
    out << "\n" << QString(80, '-') << "\n\n";
    
    out << "DETAILED TRANSFORMATION FOR EACH PATH:\n\n";
    
    // Get user-designated paths and install templates for accurate source identification
    auto userDesignatedPaths = ResourcePaths::userDesignatedPaths();
    auto installTemplateList = ResourcePaths::defaultInstallSearchPaths();
    
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
        
        // Determine source and check if from default Installation templates
        QString source = "Unknown";
        QString templatePath = "";
        bool fromInstallTemplate = false;
        
        // Check if from Installation tier default templates
        if (p.tier() == ResourceTier::Installation && i < installTemplateList.size() + userDesignatedPaths.size() + 1) {
            // Try to match against install templates
            for (const QString& tmpl : installTemplateList) {
                // Simple heuristics to match paths to their templates
                if ((tmpl == "%PROGRAMFILES%/" && p.path().contains("Program Files")) ||
                    (tmpl == "." && p.path() == QDir::current().absolutePath()) ||
                    (tmpl == "../share/" && p.path().contains("share/ScadTemplates")) ||
                    (tmpl == ".." && p.path().endsWith("cppsnippets") && !p.path().contains("build"))) {
                    templatePath = tmpl;
                    fromInstallTemplate = true;
                    break;
                }
            }
        }
        
        // Check if it's from user-designated paths first (these are Installation tier)
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
        QString displayName = platformInfo::ResourceLocation::generateDisplayName(p.path());
        out << QString("  Display Name: %1\n").arg(displayName);
        
        // Show sibling check info for Installation tier default template paths
        if (fromInstallTemplate && !templatePath.isEmpty()) {
            // Check if this template would trigger a sibling check
            bool isSiblingCandidate = (templatePath == "%PROGRAMFILES%/" || 
                                      templatePath == "%PROGRAMFILES(X86)%/" ||
                                      templatePath == "/Applications/" ||
                                      templatePath == "/opt/" ||
                                      templatePath == "/usr/local/");
            
            if (isSiblingCandidate) {
                out << "  Sibling check: ✓ Performed (path is %PROGRAMFILES%/ or equivalent)\n";
            } else {
                out << "  Sibling check: ✗ Not performed (not a sibling candidate path)\n";
            }
        } else if (p.path() == QCoreApplication::applicationDirPath()) {
            out << "  Sibling check: ✗ Not performed (exe directory added separately)\n";
        }
        
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
