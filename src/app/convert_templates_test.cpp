/**
 * @file convert_templates_test.cpp
 * @brief Console app to test legacy template conversion
 */

#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <iostream>
#include <scadtemplates/legacy_template_converter.h>
#include <platformInfo/resourceLocationManager.h>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    
    std::cout << "========================================\n";
    std::cout << "Legacy Template Converter Test\n";
    std::cout << "========================================\n\n";
    
    // Initialize resource manager
    platformInfo::ResourceLocationManager resourceManager;
    resourceManager.setApplicationPath(QCoreApplication::applicationDirPath());
    
    // Show locations
    std::cout << "Resource Locations:\n";
    auto installLocs = resourceManager.findSiblingInstallations();
    auto machineLocs = resourceManager.enabledMachineLocations();
    auto userLocs = resourceManager.enabledUserLocations();
    
    std::cout << "  Installation: " << installLocs.size() << " locations\n";
    for (const auto& loc : installLocs) {
        std::cout << "    - " << loc.path.toStdString() << "\n";
    }
    
    std::cout << "  Machine: " << machineLocs.size() << " locations\n";
    for (const auto& loc : machineLocs) {
        std::cout << "    - " << loc.path.toStdString() << "\n";
    }
    
    std::cout << "  User: " << userLocs.size() << " locations\n";
    for (const auto& loc : userLocs) {
        std::cout << "    - " << loc.path.toStdString() << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "Scanning for Legacy Templates\n";
    std::cout << "========================================\n\n";
    
    // Convert templates
    QString outputDir = QStringLiteral("./converted_templates");
    auto results = scadtemplates::LegacyTemplateConverter::discoverAndConvertTemplates(
        resourceManager, outputDir);
    
    std::cout << "Found " << results.size() << " template files\n\n";
    
    int successCount = 0;
    int failureCount = 0;
    
    for (const auto& result : results) {
        if (result.success) {
            successCount++;
            std::cout << "[OK] " << result.sourceFilePath.toStdString() << "\n";
            std::cout << "  -> Prefix: " << result.convertedTemplate.getPrefix() << "\n";
            std::cout << "  -> Body length: " << result.convertedTemplate.getBody().length() << " chars\n";
        } else {
            failureCount++;
            std::cout << "[FAIL] " << result.sourceFilePath.toStdString() << "\n";
            std::cout << "  -> Error: " << result.errorMessage.toStdString() << "\n";
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "Summary\n";
    std::cout << "========================================\n";
    std::cout << "  Successful: " << successCount << "\n";
    std::cout << "  Failed: " << failureCount << "\n";
    std::cout << "  Total: " << results.size() << "\n";
    
    return 0;
}
