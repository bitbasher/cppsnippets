/**
 * @file appname_expansion_test.cpp
 * @brief Test how Qt expands organization and application names in StandardPaths
 * 
 * Tests ALL StandardLocation types to verify which ones actually expand
 * <USER>, <APPNAME>, and <APPDIR> placeholder tags.
 */

#include <QCoreApplication>
#include <QStandardPaths>
#include <iostream>
#include <iomanip>

struct LocationTestInfo {
    QStandardPaths::StandardLocation type;
    const char* name;
    const char* expectedTags; // What placeholders SHOULD be in paths per Qt docs
};

// All location types with their expected placeholder tags
static const LocationTestInfo allLocations[] = {
    {QStandardPaths::DesktopLocation, "DesktopLocation", "<USER>"},
    {QStandardPaths::DocumentsLocation, "DocumentsLocation", "<USER>"},
    {QStandardPaths::FontsLocation, "FontsLocation", "none"},
    {QStandardPaths::ApplicationsLocation, "ApplicationsLocation", "<USER>"},
    {QStandardPaths::MusicLocation, "MusicLocation", "<USER>"},
    {QStandardPaths::MoviesLocation, "MoviesLocation", "<USER>"},
    {QStandardPaths::PicturesLocation, "PicturesLocation", "<USER>"},
    {QStandardPaths::TempLocation, "TempLocation", "<USER>"},
    {QStandardPaths::HomeLocation, "HomeLocation", "<USER>"},
    {QStandardPaths::AppLocalDataLocation, "AppLocalDataLocation", "<USER>, <APPNAME>, <APPDIR>"},
    {QStandardPaths::CacheLocation, "CacheLocation", "<USER>, <APPNAME>"},
    {QStandardPaths::GenericDataLocation, "GenericDataLocation", "<USER>, <APPDIR>"},
    {QStandardPaths::RuntimeLocation, "RuntimeLocation", "<USER>"},
    {QStandardPaths::ConfigLocation, "ConfigLocation", "<USER>, <APPNAME>"},
    {QStandardPaths::DownloadLocation, "DownloadLocation", "<USER>"},
    {QStandardPaths::GenericCacheLocation, "GenericCacheLocation", "<USER>"},
    {QStandardPaths::GenericConfigLocation, "GenericConfigLocation", "<USER>"},
    {QStandardPaths::AppDataLocation, "AppDataLocation", "<USER>, <APPNAME>, <APPDIR>"},
    {QStandardPaths::AppConfigLocation, "AppConfigLocation", "<USER>, <APPNAME>"},
    {QStandardPaths::PublicShareLocation, "PublicShareLocation", "none"},
    {QStandardPaths::TemplatesLocation, "TemplatesLocation", "<USER>"}
};

void printSeparator() {
    std::cout << "========================================================================\n";
}

void printLocationPaths(const LocationTestInfo& locInfo, const QString& orgName, const QString& appName) {
    std::cout << "\n" << std::left << std::setw(30) << locInfo.name;
    std::cout << " Expected: " << locInfo.expectedTags << "\n";
    
    QStringList paths = QStandardPaths::standardLocations(locInfo.type);
    
    if (paths.isEmpty()) {
        std::cout << "    (No paths returned)\n";
        return;
    }
    
    // Analyze paths for actual expansions
    bool hasUser = false;
    bool hasOrgApp = false;
    bool hasAppDir = false;
    
    for (const QString& path : paths) {
        // Check for user name (Jeff in this case)
        if (path.contains("Jeff", Qt::CaseInsensitive)) {
            hasUser = true;
        }
        
        // Check for org/app names
        if (!orgName.isEmpty() && path.contains(orgName, Qt::CaseSensitive)) {
            hasOrgApp = true;
        }
        if (!appName.isEmpty() && path.contains(appName, Qt::CaseSensitive)) {
            hasOrgApp = true;
        }
        
        // Check for executable directory path (APPDIR)
        if (path.contains("QtExploration", Qt::CaseSensitive) || 
            path.contains("/data", Qt::CaseSensitive)) {
            hasAppDir = true;
        }
    }
    
    // Print actual expansions found
    std::cout << "    Actual:   ";
    if (!hasUser && !hasOrgApp && !hasAppDir) {
        std::cout << "none";
    } else {
        bool first = true;
        if (hasUser) {
            std::cout << "<USER>";
            first = false;
        }
        if (hasOrgApp) {
            if (!first) std::cout << ", ";
            std::cout << "<ORG/APP>";
            first = false;
        }
        if (hasAppDir) {
            if (!first) std::cout << ", ";
            std::cout << "<APPDIR>";
        }
    }
    std::cout << "\n";
    
    // Print paths
    for (int i = 0; i < paths.size() && i < 3; ++i) {
        std::cout << "    [" << (i+1) << "] " << paths[i].toStdString() << "\n";
    }
    if (paths.size() > 3) {
        std::cout << "    ... (" << (paths.size() - 3) << " more paths)\n";
    }
}

void testScenario(const QString& orgName, const QString& appName) {
    printSeparator();
    std::cout << "TEST SCENARIO: Org=\"" << orgName.toStdString() 
              << "\" App=\"" << appName.toStdString() << "\"\n";
    printSeparator();
    
    // Set the values
    QCoreApplication::setOrganizationName(orgName);
    QCoreApplication::setApplicationName(appName);
    
    std::cout << "\nFormat: LocationName (Expected tags) -> Actual tags found\n";
    
    // Test all locations
    for (const auto& location : allLocations) {
        printLocationPaths(location, orgName, appName);
    }
    
    std::cout << "\n\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    std::cout << "\n";
    printSeparator();
    std::cout << "     COMPREHENSIVE QStandardPaths Placeholder Expansion Test\n";
    printSeparator();
    std::cout << "\n";
    std::cout << "This test verifies which StandardLocation types actually expand\n";
    std::cout << "placeholder tags: <USER>, <APPNAME> (as <ORG/APP>), <APPDIR>\n";
    std::cout << "\n";
    std::cout << "Expected: What Qt docs say should be in paths\n";
    std::cout << "Actual:   What we find in the returned paths\n";
    std::cout << "\n\n";
    
    // Test with both org and app name set
    testScenario("jartisan", "OpenSCAD");
    
    printSeparator();
    std::cout << "ANALYSIS COMPLETE\n";
    printSeparator();
    std::cout << "\nKey Insights:\n";
    std::cout << "1. <USER> expansion means the Windows username appears in path\n";
    std::cout << "2. <ORG/APP> expansion means organization and/or app name in path\n";
    std::cout << "3. <APPDIR> expansion means executable directory appears in path\n";
    std::cout << "4. GenericDataLocation SHOULD NOT expand <APPNAME> (it's generic!)\n";
    std::cout << "5. App-Specific locations (App*) SHOULD expand <APPNAME>\n";
    std::cout << "6. Compare Expected vs Actual to find Qt doc discrepancies\n";
    std::cout << "\n";
    
    return 0;
}
