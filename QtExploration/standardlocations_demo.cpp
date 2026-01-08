/**
 * @file standardlocations_demo.cpp
 * @brief Demonstrates QStandardPaths::standardLocations() for all location types
 * 
 * Shows all standard locations with display names as headers
 */

#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <iostream>
#include <iomanip>

struct LocationInfo {
    QStandardPaths::StandardLocation type;
    const char* name;
};

// All standard location types
static const LocationInfo allLocations[] = {
    {QStandardPaths::DesktopLocation, "DesktopLocation"},
    {QStandardPaths::DocumentsLocation, "DocumentsLocation"},
    {QStandardPaths::FontsLocation, "FontsLocation"},
    {QStandardPaths::ApplicationsLocation, "ApplicationsLocation"},
    {QStandardPaths::MusicLocation, "MusicLocation"},
    {QStandardPaths::MoviesLocation, "MoviesLocation"},
    {QStandardPaths::PicturesLocation, "PicturesLocation"},
    {QStandardPaths::TempLocation, "TempLocation"},
    {QStandardPaths::HomeLocation, "HomeLocation"},
    {QStandardPaths::AppLocalDataLocation, "AppLocalDataLocation"},
    {QStandardPaths::CacheLocation, "CacheLocation"},
    {QStandardPaths::GenericDataLocation, "GenericDataLocation"},
    {QStandardPaths::RuntimeLocation, "RuntimeLocation"},
    {QStandardPaths::ConfigLocation, "ConfigLocation"},
    {QStandardPaths::DownloadLocation, "DownloadLocation"},
    {QStandardPaths::GenericCacheLocation, "GenericCacheLocation"},
    {QStandardPaths::GenericConfigLocation, "GenericConfigLocation"},
    {QStandardPaths::AppDataLocation, "AppDataLocation"},
    {QStandardPaths::AppConfigLocation, "AppConfigLocation"},
    {QStandardPaths::PublicShareLocation, "PublicShareLocation"},
    {QStandardPaths::TemplatesLocation, "TemplatesLocation"}
};

void printSeparator()
{
    std::cout << "=========================================================================\n";
}

void printLocation(QStandardPaths::StandardLocation type, const char* typeName) {
    printSeparator();
    
    // Get display name (human-readable)
    QString displayName = QStandardPaths::displayName(type);
    
    std::cout << "📁 " << displayName.toStdString() << "\n";
    std::cout << "   Enum: " << typeName << " (value: " << static_cast<int>(type) << ")\n";
    printSeparator();
    
    // Get all paths for this location
    QStringList paths = QStandardPaths::standardLocations(type);
    
    if (paths.isEmpty()) {
        std::cout << "   ⚠️  No paths available for this location type\n";
    } else {
        std::cout << "   " << paths.size() << " path(s):\n\n";
        
        int index = 1;
        for (const QString& path : paths) {
            std::cout << "   " << std::setw(2) << index++ << ". " << path.toStdString() << "\n";
            
            // Check if path exists
            QFileInfo info(path);
            if (info.exists()) {
                std::cout << "       ✅ Exists";
                if (info.isReadable()) {
                    std::cout << " | Readable";
                }
                if (info.isWritable()) {
                    std::cout << " | Writable";
                }
                std::cout << "\n";
            } else {
                std::cout << "       ❌ Does not exist (would be created on first use)\n";
            }
            std::cout << "\n";
        }
    }
    
    std::cout << "\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("OpenSCAD");
    app.setOrganizationName("jartisan");
    
    std::cout << "\n";
    printSeparator();
    std::cout << "           QStandardPaths::standardLocations() Demo\n";
    printSeparator();
    std::cout << "\n";
    std::cout << "Application Name: " << app.applicationName().toStdString() << "\n";
    std::cout << "Organization: " << app.organizationName().toStdString() << "\n";
    std::cout << "\n";
    std::cout << "This shows all paths for each StandardLocation type.\n";
    std::cout << "Paths marked with ✅ exist, ❌ would be created on first use.\n";
    std::cout << "\n\n";
    
    // Print all location types
    for (const auto& location : allLocations) {
        printLocation(location.type, location.name);
    }
    
    printSeparator();
    std::cout << "Demo Complete - Showed " << (sizeof(allLocations) / sizeof(allLocations[0])) << " location types\n";
    printSeparator();
    std::cout << "\n";
    
    return 0;
}
