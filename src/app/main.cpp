/**
 * @file main.cpp
 * @brief Main entry point for the ScadTemplates Qt application
 */

#include <QApplication>
#include <QDebug>
#include <QCoreApplication>
#include <QMap>
#include <functional>
#include "mainwindow.hpp"
#include "applicationNameInfo.hpp"
#include "platformInfo/ResourceLocation.hpp"
#include "pathDiscovery/ResourcePaths.hpp"
#include "pathDiscovery/PathElement.hpp"
#include "resourceInventory/TemplatesInventory.hpp"
#include "resourceInventory/ExamplesInventory.hpp"
#include "resourceInventory/UnknownInventory.hpp"
#include "resourceMetadata/ResourceTypeInfo.hpp"

// Global inventory instances (external linkage - accessible from other translation units)
resourceInventory::TemplatesInventory* g_templatesInventory = nullptr;
resourceInventory::ExamplesInventory* g_examplesInventory = nullptr;
resourceInventory::UnknownInventory* g_fontsInventory = nullptr;
resourceInventory::UnknownInventory* g_shadersInventory = nullptr;
resourceInventory::UnknownInventory* g_librariesInventory = nullptr;
resourceInventory::UnknownInventory* g_testsInventory = nullptr;
resourceInventory::UnknownInventory* g_translationsInventory = nullptr;
resourceInventory::UnknownInventory* g_colorSchemesInventory = nullptr;
resourceInventory::UnknownInventory* g_unknownInventory = nullptr;

/**
 * @brief Dispatch addFolder call based on resource type
 * @param resType The resource type to dispatch
 * @param dirEntry Directory entry being processed
 * @param location Resource location context
 * @return Number of resources added (0 for unimplemented types)
 */
int dispatchAddFolder(resourceMetadata::ResourceType resType,
                      const QDirListing::DirEntry& dirEntry,
                      const platformInfo::ResourceLocation& location) {
    using resourceMetadata::ResourceType;
    
    switch (resType) {
        case ResourceType::Templates:
            return g_templatesInventory->addFolder(dirEntry, location);
        case ResourceType::Examples:
            return g_examplesInventory->addFolder(dirEntry, location);
        case ResourceType::Fonts:
            return g_fontsInventory->addFolder(dirEntry, location);
        case ResourceType::Shaders:
            return g_shadersInventory->addFolder(dirEntry, location);
        case ResourceType::Libraries:
            return g_librariesInventory->addFolder(dirEntry, location);
        case ResourceType::Tests:
            return g_testsInventory->addFolder(dirEntry, location);
        case ResourceType::Translations:
            return g_translationsInventory->addFolder(dirEntry, location);
        case ResourceType::ColorSchemes:
            return g_colorSchemesInventory->addFolder(dirEntry, location);
        case ResourceType::Unknown:
        default:
            return g_unknownInventory->addFolder(dirEntry, location);
    }
}

/**
 * @brief Discover and scan all resource locations
 * @return 0 on success, 1 on failure
 */
int resourceManager() {
    try {
        // Track counts per resource type
        QMap<resourceMetadata::ResourceType, int> resourceCounts;
        using resourceMetadata::ResourceType;
        
        // Discover all qualified search paths
        pathDiscovery::ResourcePaths pathDiscovery;
        QList<pathDiscovery::PathElement> discoveredPaths = pathDiscovery.qualifiedSearchPaths();
        
        for (const auto& pathElem : discoveredPaths) {
            if( platformInfo::ResourceLocation::locationHasResource(pathElem) ) {
                const platformInfo::ResourceLocation location(pathElem);
                
                for (const auto &dirEntry : QDirListing(pathElem.path()
                    , resourceMetadata::s_allResourceFolders
                    , QDirListing::IteratorFlag::DirsOnly)) {
                    resourceMetadata::ResourceType resType = 
                        resourceMetadata::ResourceTypeInfo::getResourceTypeFromFolderName(dirEntry.fileName());
                    
                    int count = dispatchAddFolder(resType, dirEntry, location);
                    resourceCounts[resType] += count;
                }
            }
        }
        
        // Report findings
        qDebug() << "Resource discovery completed:";
        qDebug() << "  Templates:" << resourceCounts.value(ResourceType::Templates, 0);
        qDebug() << "  Examples:" << resourceCounts.value(ResourceType::Examples, 0);
        
        int totalResources = 0;
        for (int count : resourceCounts) {
            totalResources += count;
        }
        qDebug() << "Total resources discovered:" << totalResources;
        
        return 0;
    } catch (const std::exception& e) {
        qCritical() << "Resource discovery failed:" << e.what();
        return 1;
    } catch (...) {
        qCritical() << "Resource discovery failed with unknown error";
        return 1;
    }
}

int main(int argc, char *argv[]) {
    try {
        QApplication app(argc, argv);
        
        app.setApplicationName(appInfo::displayName);
        app.setApplicationVersion(appInfo::version);
        app.setOrganizationName(appInfo::organization);
        
        // Initialize global inventory instances
        g_templatesInventory = new resourceInventory::TemplatesInventory();
        g_examplesInventory = new resourceInventory::ExamplesInventory();
        
        // Initialize placeholder inventories for unimplemented resource types
        g_fontsInventory = new resourceInventory::UnknownInventory();
        g_shadersInventory = new resourceInventory::UnknownInventory();
        g_librariesInventory = new resourceInventory::UnknownInventory();
        g_testsInventory = new resourceInventory::UnknownInventory();
        g_translationsInventory = new resourceInventory::UnknownInventory();
        g_colorSchemesInventory = new resourceInventory::UnknownInventory();
        g_unknownInventory = new resourceInventory::UnknownInventory();
        
        int result = resourceManager();
        if (result != 0) {
            qCritical() << "Failed to build resource inventory";
            return result;
        }
        
        MainWindow window;
        window.show();
        
        return app.exec();
    } catch (const std::exception& e) {
        qCritical() << "EXCEPTION in main():" << e.what();
        return 1;
    } catch (...) {
        qCritical() << "UNKNOWN EXCEPTION in main()";
        return 1;
    }
}
