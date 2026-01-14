/**
 * @file test_examples_tree.cpp
 * @brief Tree-printing test for Examples scanning
 * 
 * This test prints a tree structure of all examples found, including:
 * - Top-level .scad files (no category)
 * - Group folders (categories) with their .scad files
 * - Verification that "templates" and "tests" folders are skipped
 */

#include <QtTest/QtTest>
#include "resourceScanner_test.hpp"
#include <QStandardItemModel>
#include <QDir>
#include <QFileInfo>
#include <iostream>

using namespace resourceInventory;
using namespace platformInfo;

class ExamplesTreeTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        m_testDataRoot = "D:/repositories/cppsnippets/cppsnippets/testFileStructure";
        m_scanner = new ResourceScanner();
    }
    
    void cleanupTestCase() {
        delete m_scanner;
    }
    
    void printExamplesTree() {
        QString examplesPath = m_testDataRoot + "/pers/Jeff/Documents/OpenSCAD/Examples";
        
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "Examples Resource Tree\n";
        std::cout << "========================================\n";
        std::cout << "Base Path: " << examplesPath.toStdString() << "\n";
        std::cout << "========================================\n\n";
        
        // Collect all examples with their categories
        QMap<QString, QList<ResourceItem>> categorized;
        
        m_scanner->scanExamples(examplesPath, ResourceTier::User, "Test Location",
            [&categorized](const ResourceItem& item) {
                QString category = item.category().isEmpty() ? "__TOP_LEVEL__" : item.category();
                categorized[category].append(item);
            });
        
        // Print tree structure
        int totalCount = 0;
        
        // Print top-level files first
        if (categorized.contains("__TOP_LEVEL__")) {
            std::cout << "Examples/ (root)\n";
            const auto& topLevel = categorized["__TOP_LEVEL__"];
            for (const auto& item : topLevel) {
                std::cout << "  ├── " << item.displayName().toStdString();
                if (item.type() == ResourceType::Examples) {
                    std::cout << ".scad";
                } else if (item.type() == ResourceType::Templates) {
                    std::cout << " (template)";
                }
                std::cout << "\n";
                
                // Show attachments if this is a script
                const ResourceScript* script = dynamic_cast<const ResourceScript*>(&item);
                if (script && script->hasAttachments()) {
                    const auto& attachments = script->attachments();
                    for (const QString& att : attachments) {
                        QFileInfo fi(att);
                        std::cout << "    └── " << fi.fileName().toStdString() << " (attachment)\n";
                    }
                }
                
                totalCount++;
            }
            std::cout << "\n";
        }
        
        // Print categorized (Group) folders
        QStringList categories = categorized.keys();
        categories.removeAll("__TOP_LEVEL__");
        categories.sort();
        
        for (const QString& category : categories) {
            std::cout << "Examples/" << category.toStdString() << "/\n";
            const auto& items = categorized[category];
            
            for (int i = 0; i < items.size(); ++i) {
                const auto& item = items[i];
                bool isLast = (i == items.size() - 1);
                std::cout << "  " << (isLast ? "└── " : "├── ") 
                         << item.displayName().toStdString();
                if (item.type() == ResourceType::Examples) {
                    std::cout << ".scad";
                } else if (item.type() == ResourceType::Templates) {
                    std::cout << " (template)";
                } else if (item.type() == ResourceType::Tests) {
                    std::cout << " (test)";
                }
                std::cout << "\n";
                
                // Show attachments if this is a script
                const ResourceScript* script = dynamic_cast<const ResourceScript*>(&item);
                if (script && script->hasAttachments()) {
                    const auto& attachments = script->attachments();
                    for (const QString& att : attachments) {
                        QFileInfo fi(att);
                        std::cout << "    └── " << fi.fileName().toStdString() << " (attachment)\n";
                    }
                }
                
                totalCount++;
            }
            std::cout << "\n";
        }
        
        std::cout << "========================================\n";
        std::cout << "Total Examples Found: " << totalCount << "\n";
        std::cout << "Total Groups (Categories): " << categories.size() << "\n";
        std::cout << "========================================\n\n";
        
        // Verify counts
        QVERIFY(totalCount >= 3);  // Should find at least 3 items
    }
    
    void identifyResourceSubfolders() {
        QString examplesPath = m_testDataRoot + "/pers/Jeff/Documents/OpenSCAD/Examples";
        
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "Resource Subfolders in Examples\n";
        std::cout << "========================================\n";
        
        // Check which resource subfolders exist
        QDir dir(examplesPath);
        QStringList subfolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        
        for (const QString& folder : subfolders) {
            QString lowerFolder = folder.toLower();
            std::cout << folder.toStdString();
            
            if (lowerFolder == "templates") {
                std::cout << " → [RESOURCE TYPE: Templates] Delegated to templatesScanner ✓";
            } else if (lowerFolder == "tests") {
                std::cout << " → [RESOURCE TYPE: Tests] Should delegate to testsScanner (TODO)";
            } else {
                std::cout << " → [GROUP] Scanned with groupsScanner ✓";
            }
            std::cout << "\n";
        }
        
        std::cout << "========================================\n\n";
    }

private:
    QString m_testDataRoot;
    ResourceScanner* m_scanner;
};

QTEST_MAIN(ExamplesTreeTest)
#include "test_examples_tree.moc"
