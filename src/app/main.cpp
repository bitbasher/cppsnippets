/**
 * @file main.cpp
 * @brief Main entry point for the ScadTemplates Qt application
 */

#include <QApplication>
#include <QDebug>
#include <QStandardItemModel>
#include <QCoreApplication>
#include "mainwindow.hpp"
#include "applicationNameInfo.hpp"
#include "platformInfo/ResourceLocation.hpp"
#include "pathDiscovery/ResourcePaths.hpp"
#include "pathDiscovery/PathElement.hpp"
#include "resourceScanning/resourceScanner.hpp"

/**
 * @brief Discover and scan all resource locations
 * @param model The model to populate with discovered resources
 * @return true on success, false on failure
 */
bool resourceManager(QStandardItemModel* model) {
    try {
        qDebug() << "Discovering resource locations...";
        
        // Discover all qualified search paths using implemented discovery
        pathDiscovery::ResourcePaths pathDiscovery;
        QList<pathDiscovery::PathElement> discoveredPaths = pathDiscovery.qualifiedSearchPaths();
        
        // Convert to ResourceLocation (adds status: exists, writable, hasResourceFolders)
        QList<platformInfo::ResourceLocation> allLocations;
        for (const auto& pathElem : discoveredPaths) {
            allLocations.append(platformInfo::ResourceLocation(pathElem.path(), pathElem.tier()));
        }
        
        qDebug() << "Found" << allLocations.size() << "resource locations";
        
        // Scan and populate model
        resourceScanning::ResourceScanner scanner;
        scanner.scanToModel(model, allLocations);
        
        qDebug() << "Model populated with" << model->rowCount() << "items";
        
        return true;
    } catch (const std::exception& e) {
        qCritical() << "Resource discovery failed:" << e.what();
        return false;
    } catch (...) {
        qCritical() << "Resource discovery failed with unknown error";
        return false;
    }
}

int main(int argc, char *argv[]) {
    qDebug() << "Starting" << appInfo::displayName << "application...";
    
    QApplication app(argc, argv);
    
    app.setApplicationName(appInfo::displayName);
    app.setApplicationVersion(appInfo::version);
    app.setOrganizationName(appInfo::organization);
    
    qDebug() << "Building resource inventory...";
    QStandardItemModel* inventory = new QStandardItemModel();
    if (!resourceManager(inventory)) {
        qCritical() << "Failed to build resource inventory - exiting";
        return 1;
    }
    
    qDebug() << "Creating main window...";
    MainWindow window(inventory);
    qDebug() << "Showing main window...";
    window.show();
    qDebug() << "Entering event loop...";
    
    return app.exec();
}
