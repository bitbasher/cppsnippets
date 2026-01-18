/**
 * @file test_sa_scanner_output.cpp
 * @brief Simple test to verify scanner discovers templates
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QStandardItemModel>

#include "../src/pathDiscovery/ResourcePaths.hpp"
#include "../src/pathDiscovery/PathElement.hpp"
#include "../src/platformInfo/ResourceLocation.hpp"
#include "../src/resourceScanning/resourceScanner.hpp"
#include "../src/resourceMetadata/ResourceTier.hpp"
#include "applicationNameInfo.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Command-line argument parsing
    QCommandLineParser parser;
    parser.setApplicationDescription("Resource scanner test - shows discovered resources");
    parser.addHelpOption();
    
    QCommandLineOption appNameOption(QStringList() << "a" << "appname",
        "Application name to use for discovery (default: OpenSCAD)",
        "name",
        "OpenSCAD");
    parser.addOption(appNameOption);
    
    parser.process(app);
    
    QString appName = parser.value(appNameOption);
    
    // CRITICAL: Set test app name BEFORE creating ResourcePaths
    appInfo::setTestAppName(appName);
    
    qDebug() << "=== Template Scanner Test ===";
    qDebug() << "Application name:" << appName << "\n";
    
    // Step 1: Discover paths
    qDebug() << "Step 1: Discovering resource locations...";
    pathDiscovery::ResourcePaths pathDiscovery;
    QList<pathDiscovery::PathElement> discoveredPaths = pathDiscovery.qualifiedSearchPaths();
    qDebug() << "Found" << discoveredPaths.size() << "qualified paths\n";
    
    // Step 2: Convert to ResourceLocations
    qDebug() << "Step 2: Converting to ResourceLocations...";
    QList<platformInfo::ResourceLocation> allLocations;
    for (const auto& pathElem : discoveredPaths) {
        allLocations.append(platformInfo::ResourceLocation(pathElem.path(), pathElem.tier()));
        qDebug() << "  -" << pathElem.path() 
                 << "[" << resourceMetadata::tierToString(pathElem.tier()) << "]";
    }
    qDebug() << "";
    
    // Step 3: Scan with ResourceScanner
    qDebug() << "Step 3: Scanning for templates...";
    resourceScanning::ResourceScanner scanner;
    QStandardItemModel* model = scanner.scanToModel(allLocations);
    
    // Step 4: Display results
    qDebug() << "\n=== Inventory Counts ===";
    qDebug() << "Examples:     " << scanner.examplesCount();
    qDebug() << "Templates:    " << scanner.templatesCount();
    qDebug() << "Fonts:        " << scanner.fontsCount();
    qDebug() << "Shaders:      " << scanner.shadersCount();
    qDebug() << "Translations: " << scanner.translationsCount();
    qDebug() << "Tests:        " << scanner.testsCount();
    
    if (model) {
        qDebug() << "\n=== Templates Model ===";
        qDebug() << "Model has" << model->rowCount() << "rows," << model->columnCount() << "columns";
        qDebug() << "Headers:" << model->horizontalHeaderItem(0)->text() 
                 << "," << model->horizontalHeaderItem(1)->text()
                 << "," << model->horizontalHeaderItem(2)->text();
        
        qDebug() << "\nTemplates found:";
        for (int row = 0; row < model->rowCount(); ++row) {
            QString name = model->item(row, 0)->text();
            QString tier = model->item(row, 1)->text();
            QString path = model->item(row, 2)->text();
            qDebug() << "  " << row+1 << ":" << name << "[" << tier << "]";
            qDebug() << "      " << path;
        }
        
        delete model;
    } else {
        qDebug() << "\n!!! No templates found - model is null !!!";
    }
    
    qDebug() << "\n=== Test Complete ===";
    
    return 0;
}
