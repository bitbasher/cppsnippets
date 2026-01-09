/**
 * @file library_discovery_test.cpp
 * @brief Console app to exercise library scanning against testFileStructure.
 */

#include <iostream>
#include <QApplication>
#include <QDir>
#include "resourceInventory/resourceScanner.hpp"
#include "resourceInventory/resourceTreeWidget.hpp"
#include "resourceInventory/resourceItem.hpp"
#include "platformInfo/ResourceLocation.hpp"

namespace ri = resInventory;
namespace pi = platformInfo;

static void printHeader(const QString& title)
{
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << title.toStdString() << std::endl;
    std::cout << std::string(80, '=') << std::endl;
}

static void printItem(const ri::ResourceItem& item)
{
    std::cout << "  - [" << resourceTypeToString(item.type()).toStdString() << "] "
              << item.displayName().toStdString()
              << " (" << item.path().toStdString() << ")" << std::endl;
    if (!item.category().isEmpty()) {
        std::cout << "      category: " << item.category().toStdString() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    printHeader("Library Discovery Test");

    // Locate testFileStructure from build/bin
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp(); // bin -> build
    dir.cdUp(); // build -> repo root
    const QString base = dir.absoluteFilePath("testFileStructure");
    std::cout << "testFileStructure: " << base.toStdString() << std::endl;

    // Nightly installation location with libraries
    pi::ResourceLocation nightly;
    nightly.path = QDir(base).absoluteFilePath("inst/OpenSCAD (Nightly)");
    nightly.displayName = "OpenSCAD (Nightly)";
    nightly.hasResourceFolders = true;
    nightly.exists = QDir(nightly.path).exists();
    nightly.isEnabled = true;

    if (!nightly.exists) {
        std::cout << "Nightly path not found, exiting." << std::endl;
        return 1;
    }

    ri::ResourceScanner scanner;
    ri::ResourceTreeWidget tree;
    tree.setResourceType(ri::ResourceType::Library);
    tree.setShowCategories(true);

    scanner.scanLibraries({nightly}, ri::ResourceTier::Installation, &tree);

    auto items = tree.allItems();
    std::cout << "\nDiscovered " << items.size() << " items" << std::endl;
    for (const auto& item : items) {
        printItem(item);
    }

    return 0;
}
