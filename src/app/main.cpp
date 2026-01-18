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
 * @return Populated model with discovered resources, or nullptr on failure
 */
QStandardItemModel* resourceManager() {
    try {
        qDebug() << "Discovering resource locations...";
        
        // Discover all qualified search paths using implemented discovery
        pathDiscovery::ResourcePaths pathDiscovery;
        QList<pathDiscovery::PathElement> discoveredPaths = pathDiscovery.qualifiedSearchPaths();
        
        // Convert to ResourceLocation (adds display name, tier, status)
        QList<platformInfo::ResourceLocation> allLocations;
        for (const auto& pathElem : discoveredPaths) {
            allLocations.append(platformInfo::ResourceLocation(pathElem.path(), pathElem.tier()));
        }
        
        qDebug() << "Found" << allLocations.size() << "resource locations";
        
        // Scan locations and create populated model
        resourceScanning::ResourceScanner scanner;
        QStandardItemModel* model = scanner.scanToModel(allLocations);
        
        qDebug() << "Model populated with" << model->rowCount() << "items";
        
        return model;
    } catch (const std::exception& e) {
        qCritical() << "Resource discovery failed:" << e.what();
        return nullptr;
    } catch (...) {
        qCritical() << "Resource discovery failed with unknown error";
        return nullptr;
    }
}

int main(int argc, char *argv[]) {
    qDebug() << "Starting" << appInfo::displayName << "application...";
    
    QApplication app(argc, argv);
    
    app.setApplicationName(appInfo::displayName);
    app.setApplicationVersion(appInfo::version);
    app.setOrganizationName(appInfo::organization);
    
    qDebug() << "Building resource inventory...";
    QStandardItemModel* inventory = resourceManager();
    if (!inventory) {
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
