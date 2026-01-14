/**
 * @file test_attachment_scanning.cpp  
 * @brief Test that attachments are properly scanned
 */

#include <QtTest/QtTest>
#include "resourceScanner_test.hpp"
#include <QDir>
#include <iostream>

using namespace resourceInventory;
using namespace platformInfo;

class AttachmentScanTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        m_scanner = new ResourceScanner();
    }
    
    void cleanupTestCase() {
        delete m_scanner;
    }
    
    void testParametricExampleHasAttachment() {
        QString examplesPath = "D:/repositories/cppsnippets/cppsnippets/testFileStructure/pers/Jeff/Documents/OpenSCAD/Examples";
        
        std::cout << "\n=== Testing Attachment Scanning ===\n";
        
        // Scan examples
        QList<ResourceItem> captured;
        m_scanner->scanExamples(examplesPath, ResourceTier::User, "Test",
            [&captured](const ResourceItem& item) {
                captured.append(item);
            });
        
        // Find customizer-all
        bool found = false;
        for (const auto& item : captured) {
            if (item.displayName() == "customizer-all") {
                found = true;
                std::cout << "Found: " << item.displayName().toStdString() << "\n";
                std::cout << "Type: " << static_cast<int>(item.type()) << "\n";
                std::cout << "Category: " << item.category().toStdString() << "\n";
                
                const ResourceScript* script = dynamic_cast<const ResourceScript*>(&item);
                if (script) {
                    std::cout << "Is ResourceScript: YES\n";
                    std::cout << "Has attachments: " << (script->hasAttachments() ? "YES" : "NO") << "\n";
                    std::cout << "Attachment count: " << script->attachments().size() << "\n";
                    
                    if (script->hasAttachments()) {
                        for (const QString& att : script->attachments()) {
                            std::cout << "  - " << att.toStdString() << "\n";
                        }
                    }
                    
                    QVERIFY(script->hasAttachments());
                    QVERIFY(script->attachments().size() >= 1);
                } else {
                    std::cout << "Is ResourceScript: NO (dynamic_cast failed)\n";
                    QFAIL("customizer-all should be a ResourceScript");
                }
                break;
            }
        }
        
        QVERIFY(found);
    }

private:
    ResourceScanner* m_scanner;
};

QTEST_MAIN(AttachmentScanTest)
#include "test_attachment_scanning.moc"
