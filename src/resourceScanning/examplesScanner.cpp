/**
 * @file examplesScanner.hpp
 * @brief Information about OpenSCAD resources
 *
 * Option 1: Manager/Coordinator Pattern (Your Switch/Case Idea)
 */

#include <QString>
#include <QDirIterator>
#include <QFileInfo>
#include <QDir>

#include "resourceInventory/ResourceInventory.hpp"
#include "resourceInventory/ResourceMetadata.hpp"
#include "examplesScanner.hpp"
#include "templatesScanner.hpp"
#include "testsScanner.hpp"
#include "colorSchemesScanner.hpp"
#include "groupcanner.hpp"


class ExamplesScanner {
public:
    void scanExamplesFolder(const QString& basePath, ResourceInventory* inventory) {
        
    using ItFlag = QDirListing::IteratorFlag;
    const auto dirflags = ItFlag::DirssOnly | ItFlag::NoDotAndDotDot;
    
    for (const auto &dirEntry : QDirListing( basePath, dirflags)) {
        
        QString folderName = dirEntry.fileName().toLower();
        
        switch( s_topLevelReverse.value(folderName, resourceMetadata::ResourceType::Unknown) ) {
        case(resourceMetadata::ResourceType::Templates
            // Delegate to templates scanner
            templatesInventory(dirEntry);
            break;
        case (resourceMetadata::ResourceType::Tests)
            testsInventory(dirEntry);
            break;
        case resourceMetadata::ResourceType::Unknown:
        default:
            // It is a non-resource folder, process as Group
            examplesInventory.addFolder(dirEntry, folderName);
        }
    }

    const auto fileflags = ItFlag::FilesOnly | ItFlag::NoDotAndDotDot;
    for (const auto &dirEntry : QDirListing( basePath, {"*.scad"}, fileflags)) {
        examplesInventory.addExample(dirEntry);
    }
    
private:
    void scanGroupFolder(const QString& path, 
                        const QString& category,
                        ResourceInventory* inventory) {
        // Scan .scad files in this group
        QDirIterator it(path, {"*.scad"}, QDir::Files);
        while (it.hasNext()) {
            scanExampleScript(it.next(), inventory, category);
        }
    }
};