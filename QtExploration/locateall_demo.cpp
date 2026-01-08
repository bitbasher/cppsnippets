/**
 * @file locateall_demo.cpp
 * @brief Demonstrates QStandardPaths::locateAll() method
 * 
 * Searches for ALL folders named "templates" in:
 * - HomeLocation
 * - AppLocalDataLocation
 */

#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <iostream>

void searchAllForTemplates(QStandardPaths::StandardLocation location, const QString& locationName)
{
    std::cout << "\n========================================\n";
    std::cout << "Searching in: " << locationName.toStdString() << "\n";
    std::cout << "========================================\n";
    
    // Get the base paths first
    QStringList basePaths = QStandardPaths::standardLocations(location);
    std::cout << "Base paths for this location:\n";
    for (const QString& path : basePaths) {
        std::cout << "  - " << path.toStdString() << "\n";
    }
    std::cout << "\n";
    
    // locateAll() finds ALL matches
    QStringList results = QStandardPaths::locateAll(
        location,
        "templates",
        QStandardPaths::LocateDirectory
    );
    
    if (results.isEmpty()) {
        std::cout << "❌ No 'templates' folders found\n";
    } else {
        std::cout << "✅ Found " << results.size() << " 'templates' folder(s):\n\n";
        
        int index = 1;
        for (const QString& path : results) {
            std::cout << index++ << ". " << path.toStdString() << "\n";
            
            QFileInfo info(path);
            std::cout << "   Exists: " << (info.exists() ? "Yes" : "No") << "\n";
            std::cout << "   Is Directory: " << (info.isDir() ? "Yes" : "No") << "\n";
            std::cout << "   Readable: " << (info.isReadable() ? "Yes" : "No") << "\n";
            std::cout << "   Writable: " << (info.isWritable() ? "Yes" : "No") << "\n";
            std::cout << "\n";
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("ScadTemplates");
    app.setOrganizationName("jartisan");
    
    std::cout << "===========================================\n";
    std::cout << "QStandardPaths::locateAll() Demo\n";
    std::cout << "===========================================\n";
    std::cout << "App Name: " << app.applicationName().toStdString() << "\n";
    std::cout << "Organization: " << app.organizationName().toStdString() << "\n";
    
    // Search in HomeLocation
    searchAllForTemplates(QStandardPaths::HomeLocation, "HomeLocation");
    
    // Search in AppLocalDataLocation
    searchAllForTemplates(QStandardPaths::AppLocalDataLocation, "AppLocalDataLocation");
    
    std::cout << "\n===========================================\n";
    std::cout << "locateAll() returns ALL matches, not just first.\n";
    std::cout << "===========================================\n\n";
    
    return 0;
}
