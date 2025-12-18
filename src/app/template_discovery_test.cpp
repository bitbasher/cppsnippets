/**
 * @file template_discovery_test.cpp
 * @brief Console test app for discovering templates in testFileStructure
 * 
 * This app scans testFileStructure folders for templates and displays
 * what was discovered, demonstrating the test data organization.
 */

#include <iostream>
#include <QString>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDirIterator>

void printSeparator(const QString& title = QString()) {
    if (!title.isEmpty()) {
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << title.toStdString() << std::endl;
        std::cout << std::string(80, '=') << std::endl;
    } else {
        std::cout << std::string(80, '-') << std::endl;
    }
}

void printTemplateMetadata(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("description")) {
                std::cout << "       Description: " << obj["description"].toString().toStdString() << std::endl;
            }
            if (obj.contains("prefix")) {
                std::cout << "       Prefix: " << obj["prefix"].toString().toStdString() << std::endl;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    printSeparator("Template Discovery in testFileStructure");
    
    // Determine test file structure path
    QDir currentDir(QCoreApplication::applicationDirPath());
    currentDir.cdUp();  // Go from bin to build-fresh
    currentDir.cdUp();  // Go from build-fresh to project root
    QString testStructurePath = currentDir.absoluteFilePath("testFileStructure");
    
    std::cout << "Test Structure Path: " << testStructurePath.toStdString() << std::endl;
    
    if (!QDir(testStructurePath).exists()) {
        std::cout << "ERROR: testFileStructure not found at: " << testStructurePath.toStdString() << std::endl;
        return 1;
    }
    
    int totalDiscovered = 0;
    
    // Scan Installation tier
    {
        printSeparator("Installation Tier");
        QDir instDir(QDir(testStructurePath).absoluteFilePath("inst"));
        QStringList installations = instDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        
        for (const QString& install : installations) {
            QString templatesPath = instDir.absoluteFilePath(install + "/templates");
            if (!QDir(templatesPath).exists()) {
                continue;
            }
            
            std::cout << "\n" << install.toStdString() << std::endl;
            std::cout << "  Path: " << templatesPath.toStdString() << std::endl;
            
            QDirIterator it(templatesPath, {"*.json"}, QDir::Files);
            int count = 0;
            while (it.hasNext()) {
                QString templateFile = it.next();
                QString fileName = QFileInfo(templateFile).fileName();
                std::cout << "  ✓ " << fileName.toStdString() << std::endl;
                printTemplateMetadata(templateFile);
                count++;
                totalDiscovered++;
            }
            
            std::cout << "  Found " << count << " template(s)" << std::endl;
        }
    }
    
    // Scan User tier
    {
        printSeparator("User Tier");
        QDir persDir(QDir(testStructurePath).absoluteFilePath("pers"));
        QStringList users = persDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        
        for (const QString& user : users) {
            if (user == "appdata") continue;  // Skip appdata folder
            
            QString templatesPath = persDir.absoluteFilePath(user + "/Documents/OpenSCAD/templates");
            if (!QDir(templatesPath).exists()) {
                continue;
            }
            
            std::cout << "\n" << user.toStdString() << "'s Templates" << std::endl;
            std::cout << "  Path: " << templatesPath.toStdString() << std::endl;
            
            QDirIterator it(templatesPath, {"*.json"}, QDir::Files);
            int count = 0;
            while (it.hasNext()) {
                QString templateFile = it.next();
                QString fileName = QFileInfo(templateFile).fileName();
                std::cout << "  ✓ " << fileName.toStdString() << std::endl;
                printTemplateMetadata(templateFile);
                count++;
                totalDiscovered++;
            }
            
            std::cout << "  Found " << count << " template(s)" << std::endl;
        }
    }
    
    printSeparator("Discovery Summary");
    std::cout << "Total templates discovered: " << totalDiscovered << std::endl;
    std::cout << "\nExpected templates:" << std::endl;
    std::cout << "  Installation (OpenSCAD): 2 templates" << std::endl;
    std::cout << "  Installation (Nightly):   2 templates" << std::endl;
    std::cout << "  User (Jeff):              2 templates" << std::endl;
    std::cout << "  TOTAL EXPECTED:           6 templates" << std::endl;
    
    if (totalDiscovered == 6) {
        std::cout << "\n✓ SUCCESS: All templates discovered!" << std::endl;
        return 0;
    } else {
        std::cout << "\n✗ MISMATCH: Expected 6 but found " << totalDiscovered << std::endl;
        return 1;
    }
}
