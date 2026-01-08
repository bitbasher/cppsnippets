/**
 * @file locate_demo.cpp
 * @brief Demonstrates QStandardPaths::locate() method
 * 
 * Searches for folders named "templates" in:
 * - HomeLocation
 * - AppLocalDataLocation
 */

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <iostream>

void searchForTemplates(QStandardPaths::StandardLocation location, const QString& locationName)
{
    std::cout << "\n========================================\n";
    std::cout << "Searching in: " << locationName.toStdString() << "\n";
    std::cout << "========================================\n";
    
    // locate() finds first match only
    // LocateDirectory option looks for directories instead of files
    QString result = QStandardPaths::locate(
        location,
        "templates",
        QStandardPaths::LocateDirectory
    );
    
    if (result.isEmpty()) {
        std::cout << "❌ No 'templates' folder found\n";
    } else {
        std::cout << "✅ Found: " << result.toStdString() << "\n";
        
        // Check if it actually exists and is a directory
        QFileInfo info(result);
        std::cout << "   Exists: " << (info.exists() ? "Yes" : "No") << "\n";
        std::cout << "   Is Directory: " << (info.isDir() ? "Yes" : "No") << "\n";
        std::cout << "   Readable: " << (info.isReadable() ? "Yes" : "No") << "\n";
        std::cout << "   Writable: " << (info.isWritable() ? "Yes" : "No") << "\n";
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("test_locate_templates");
    app.setOrganizationName("jartisan");
    
    std::cout << "===========================================\n";
    std::cout << "QStandardPaths::locate() Demo\n";
    std::cout << "===========================================\n";
    std::cout << "App Name: " << app.applicationName().toStdString() << "\n";
    std::cout << "Organization: " << app.organizationName().toStdString() << "\n";
    
    // Search in HomeLocation
    searchForTemplates(QStandardPaths::HomeLocation, "HomeLocation");
    
    // Search in AppLocalDataLocation
    searchForTemplates(QStandardPaths::AppLocalDataLocation, QStandardPaths::displayName(QStandardPaths::AppLocalDataLocation) );

    searchForTemplates(QStandardPaths::DocumentsLocation, "DocumentsLocation");
    
    std::cout << "\n===========================================\n";
    std::cout << "Note: locate() returns FIRST match only.\n";
    std::cout << "Use locateAll() to find all matches.\n";
    std::cout << "===========================================\n\n";
    
    return 0;
}
