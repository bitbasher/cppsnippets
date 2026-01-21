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
#include "resourceMetadata/ResourceTypeInfo.hpp"

// Global inventory instances
static resourceInventory::TemplatesInventory* g_templatesInventory = nullptr;
static resourceInventory::ExamplesInventory* g_examplesInventory = nullptr;

/**
 * @brief Placeholder for unimplemented resource types
 */
int unknownAddFolder(const QDirListing::DirEntry& dirEntry, const platformInfo::ResourceLocation& location) {
    qWarning() << "Unimplemented resource scanner for folder:" << dirEntry.fileName()
               << "at location:" << location.path();
    return 0;
}

/**
 * @brief Get function pointer for addFolder based on resource type
 * @param resType The resource type to dispatch
 * @return Function pointer to appropriate addFolder method, or unknown handler
 */
std::function<int(const QDirListing::DirEntry&, const platformInfo::ResourceLocation&)> 
getAddFolderFunction(resourceMetadata::ResourceType resType) {
    using resourceMetadata::ResourceType;
    
    static const QMap<ResourceType, std::function<int(const QDirListing::DirEntry&, const platformInfo::ResourceLocation&)>> dispatchMap = {
        { ResourceType::Templates, [](const QDirListing::DirEntry& de, const platformInfo::ResourceLocation& loc) {
            return g_templatesInventory->addFolder(de, loc);
        }},
        { ResourceType::Examples, [](const QDirListing::DirEntry& de, const platformInfo::ResourceLocation& loc) {
            return g_examplesInventory->addFolder(de, loc);
        }},
        { ResourceType::Fonts, unknownAddFolder },
        { ResourceType::Shaders, unknownAddFolder },
        { ResourceType::Libraries, unknownAddFolder },
        { ResourceType::Tests, unknownAddFolder },
        { ResourceType::Translations, unknownAddFolder },
        { ResourceType::ColorSchemes, unknownAddFolder },
        { ResourceType::EditorColors, unknownAddFolder },
        { ResourceType::RenderColors, unknownAddFolder },
        { ResourceType::Group, unknownAddFolder },
        { ResourceType::Unknown, unknownAddFolder }
    };
    
    auto it = dispatchMap.find(resType);
    return (it != dispatchMap.end()) ? *it : unknownAddFolder;
}

/**
 * @brief Discover and scan all resource locations
 * @return Populated TemplatesInventory model with discovered resources, or nullptr on failure
 */
int resourceManager() {
    int locationsBefore = 0;
    try {
        qDebug() << "Discovering resource locations...";
        
        // Discover all qualified search paths using implemented discovery
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
                    getAddFolderFunction(resType)(dirEntry, location);
                }
            }
        }
        
        qDebug() << "Resource discovery completed";
        
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
    qDebug() << "Starting" << appInfo::displayName << "application...";
    
    QApplication app(argc, argv);
    
    app.setApplicationName(appInfo::displayName);
    app.setApplicationVersion(appInfo::version);
    app.setOrganizationName(appInfo::organization);
    
    // Initialize global inventory instances
    g_templatesInventory = new resourceInventory::TemplatesInventory();
    g_examplesInventory = new resourceInventory::ExamplesInventory();
    
    qDebug() << "Building resource inventory...";
    int result = resourceManager();
    if (result != 0) {
        qCritical() << "Failed to build resource inventory";
        return result;
    }
    
    qDebug() << "Creating main window...";
    MainWindow window(g_templatesInventory);
    qDebug() << "Showing main window...";
    window.show();
    qDebug() << "Entering event loop...";
    
    return app.exec();
}
