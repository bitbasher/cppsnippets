/**
 * @file inventory_test.cpp
 * @brief Console test app for ResourceInventoryManager and ResourceScanner
 * 
 * This test app exercises the resource scanning functionality without
 * requiring the full Qt GUI. It scans configured locations and reports
 * what resources are found.
 * 
 * Note: Uses QApplication (not QCoreApplication) because ResourceTreeWidget
 * inherits from QTreeWidget which requires QApplication.
 */

#include <QApplication>
#include <QDebug>
#include <QDir>

#include "platformInfo/resourceLocationManager.hpp"
#include "platformInfo/resourcePaths.hpp"
#include "resInventory/resourceScanner.hpp"
#include "resInventory/resourceTreeWidget.hpp"
#include "resInventory/resourceItem.hpp"
#include "gui/machineTab.hpp"
#include "gui/userTab.hpp"

// Use explicit namespace prefixes to avoid conflicts
namespace pi = platformInfo;
namespace ri = resInventory;

void printSeparator(const QString& title = QString())
{
    qDebug() << "";
    qDebug() << "========================================";
    if (!title.isEmpty()) {
        qDebug() << title;
        qDebug() << "========================================";
    }
}

void printLocations(const QString& tierName, const QVector<pi::ResourceLocation>& locations)
{
    qDebug() << "";
    qDebug() << QString("--- %1 Locations (%2) ---").arg(tierName).arg(locations.size());
    
    for (const auto& loc : locations) {
        QString status;
        if (!loc.exists) {
            status = "[MISSING]";
        } else if (!loc.isEnabled) {
            status = "[DISABLED]";
        } else if (!loc.hasResourceFolders) {
            status = "[NO RESOURCES]";
        } else {
            status = "[OK]";
        }
        
        qDebug() << QString("  %1 %2").arg(status, -14).arg(loc.path);
        if (!loc.displayName.isEmpty() && loc.displayName != loc.path) {
            qDebug() << QString("    Display: %1").arg(loc.displayName);
        }
    }
}

void printResourceItems(const QString& typeName, ri::ResourceTreeWidget* tree)
{
    if (!tree) {
        qDebug() << QString("  %1: (null tree)").arg(typeName);
        return;
    }
    
    QList<ri::ResourceItem> items = tree->allItems();
    qDebug() << QString("  %1: %2 items found").arg(typeName).arg(items.size());
    
    for (const auto& item : items) {
        QString tierStr;
        switch (item.tier()) {
            case ri::ResourceTier::Installation: tierStr = "Install"; break;
            case ri::ResourceTier::Machine: tierStr = "Machine"; break;
            case ri::ResourceTier::User: tierStr = "User"; break;
        }
        
        QString categoryStr = item.category().isEmpty() ? "" : QString(" [%1]").arg(item.category());
        
        qDebug() << QString("    [%1] %2%3")
                    .arg(tierStr, -7)
                    .arg(item.displayName())
                    .arg(categoryStr);
        qDebug() << QString("           Path: %1").arg(item.path());
    }
}

int main(int argc, char *argv[])
{
    // Need QApplication (not QCoreApplication) for QTreeWidget-based classes
    QApplication app(argc, argv);
    app.setApplicationName("InventoryTest");
    
    printSeparator("Resource Inventory Test");
    qDebug() << "Application path:" << QCoreApplication::applicationDirPath();
    
    // ========================================
    // 1. Show default search paths
    // ========================================
    printSeparator("Default Search Paths (compile-time)");
    
    qDebug() << "Installation paths:";
    for (const QString& path : pi::ResourcePaths::defaultInstallSearchPaths()) {
        qDebug() << "  " << path;
    }
    
    qDebug() << "";
    qDebug() << "Machine paths:";
    for (const QString& path : pi::ResourcePaths::defaultMachineSearchPaths()) {
        qDebug() << "  " << path;
    }
    
    qDebug() << "";
    qDebug() << "User paths:";
    for (const QString& path : pi::ResourcePaths::defaultUserSearchPaths()) {
        qDebug() << "  " << path;
    }
    
    // ========================================
    // 2. Show environment variables
    // ========================================
    printSeparator("Environment Variables");
    
    QString openscadPath = MachineTab::openscadPathEnv();
    qDebug() << "OPENSCAD_PATH:" << (openscadPath.isEmpty() ? "(not set)" : openscadPath);
    
    QString xdgDataDirs = MachineTab::xdgDataDirsEnv();
    qDebug() << "XDG_DATA_DIRS:" << (xdgDataDirs.isEmpty() ? "(not set)" : xdgDataDirs);
    
    QString xdgDataHome = UserTab::xdgDataHomeEnv();
    qDebug() << "XDG_DATA_HOME:" << (xdgDataHome.isEmpty() ? "(not set)" : xdgDataHome);
    
    // ========================================
    // 3. Create ResourceLocationManager and get locations
    // ========================================
    printSeparator("ResourceLocationManager");
    
    pi::ResourceLocationManager locMgr;
    locMgr.setApplicationPath(QCoreApplication::applicationDirPath());
    
    // Installation: use effective installation path
    QVector<pi::ResourceLocation> installLocs;
    QString installPath = locMgr.effectiveInstallationPath();
    if (!installPath.isEmpty()) {
        pi::ResourceLocation installLoc;
        installLoc.path = installPath;
        installLoc.displayName = "Installation";
        installLoc.exists = QDir(installPath).exists();
        installLoc.isEnabled = true;
        installLoc.hasResourceFolders = true;  // Assume valid by definition
        installLocs.append(installLoc);
    }
    
    // Also add sibling installations
    installLocs.append(locMgr.findSiblingInstallations());
    
    // Machine and User locations from manager
    QVector<pi::ResourceLocation> machineLocs = locMgr.availableMachineLocations();
    QVector<pi::ResourceLocation> userLocs = locMgr.availableUserLocations();
    
    // Inject a test path for development testing
    {
        pi::ResourceLocation testLoc;
        testLoc.path = QStringLiteral("D:/repositories/openscad-newexamples");
        testLoc.displayName = QStringLiteral("Test: openscad-newexamples");
        testLoc.exists = QDir(testLoc.path).exists();
        testLoc.isEnabled = true;
        testLoc.hasResourceFolders = testLoc.exists;
        userLocs.prepend(testLoc);
    }
    
    // Add environment variable locations
    machineLocs.append(MachineTab::openscadPathLocation());
    machineLocs.append(MachineTab::xdgDataDirsLocations());
    
    userLocs.append(UserTab::openscadPathLocation());
    userLocs.append(UserTab::xdgDataHomeLocations());
    
    printLocations("Installation", installLocs);
    printLocations("Machine", machineLocs);
    printLocations("User", userLocs);
    
    // ========================================
    // 4. Create ResourceInventoryManager and scan
    // ========================================
    printSeparator("Resource Scanning");
    
    ri::ResourceInventoryManager inventoryMgr;
    inventoryMgr.setInstallLocations(installLocs);
    inventoryMgr.setMachineLocations(machineLocs);
    inventoryMgr.setUserLocations(userLocs);
    
    // Connect to signals for progress
    QObject::connect(&inventoryMgr, &ri::ResourceInventoryManager::inventoryBuilt,
                     [](ri::ResourceType type, int count) {
        qDebug() << QString("  Built inventory for %1: %2 items")
                    .arg(ri::resourceTypeToString(type))
                    .arg(count);
    });
    
    qDebug() << "";
    qDebug() << "Scanning for resources...";
    qDebug() << "";
    
    // Scan for each resource type
    QList<ri::ResourceType> types = {
        ri::ResourceType::RenderColors,
        ri::ResourceType::EditorColors,
        ri::ResourceType::Font,
        ri::ResourceType::Example,
        ri::ResourceType::Template,
        ri::ResourceType::Library,
        ri::ResourceType::Translation
    };
    
    for (ri::ResourceType type : types) {
        ri::ResourceTreeWidget* tree = inventoryMgr.buildInventory(type);
        Q_UNUSED(tree);
    }
    
    // ========================================
    // 5. Display results
    // ========================================
    printSeparator("Discovered Resources");
    
    for (ri::ResourceType type : types) {
        ri::ResourceTreeWidget* tree = inventoryMgr.inventory(type);
        printResourceItems(ri::resourceTypeToString(type), tree);
        qDebug() << "";
    }
    
    // ========================================
    // 6. Summary
    // ========================================
    printSeparator("Summary");
    
    int totalResources = 0;
    for (ri::ResourceType type : types) {
        ri::ResourceTreeWidget* tree = inventoryMgr.inventory(type);
        if (tree) {
            int count = tree->allItems().size();
            totalResources += count;
            qDebug() << QString("  %1: %2")
                        .arg(ri::resourceTypeToString(type), -12)
                        .arg(count);
        }
    }
    qDebug() << QString("  %1: %2").arg("TOTAL", -12).arg(totalResources);
    
    printSeparator();
    qDebug() << "Test complete.";
    
    return 0;
}
