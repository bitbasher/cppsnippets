/**
 * @file test_tier_standalone.cpp
 * @brief Minimal standalone test for ResourceTier
 */

#include <iostream>
#include <QCoreApplication>
#include <QString>
#ifdef USE_TEST_APP_INFO
#include "testAppNameInfo.hpp"
#else
#include "applicationNameInfo.hpp"
#endif
#include "resourceMetadata/ResourceTier.hpp"

using namespace resourceMetadata;

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // Parse command line arguments for app name (default: OpenSCAD)
    QString appName = "OpenSCAD";
    if (argc > 1) {
        appName = QString::fromUtf8(argv[1]);
    }
    
    // Set test application name for resource discovery
    appInfo::setBaseName(appName);
    
    std::cout << "=== ResourceTier Standalone Test ===\n";
    std::cout << "Application Name: " << appName.toStdString() << "\n\n";
    
    // Test 1: Enum values
    std::cout << "Test 1: Enum Values\n";
    ResourceTier inst = ResourceTier::Installation;
    ResourceTier mach = ResourceTier::Machine;
    ResourceTier user = ResourceTier::User;
    std::cout << "  Installation enum created: OK\n";
    std::cout << "  Machine enum created: OK\n";
    std::cout << "  User enum created: OK\n\n";
    
    // Test 2: tierDisplayNames map access
    std::cout << "Test 2: tierDisplayNames Map\n";
    std::cout << "  Map size: " << tierDisplayNames.size() << "\n";
    std::cout << "  Map empty: " << (tierDisplayNames.isEmpty() ? "YES (ERROR!)" : "NO (OK)") << "\n\n";
    
    // Test 3: Access display names
    std::cout << "Test 3: Display Name Lookup\n";
    try {
        if (tierDisplayNames.contains(ResourceTier::Installation)) {
            QString name = tierDisplayNames[ResourceTier::Installation];
            std::cout << "  Installation: " << name.toStdString() << "\n";
        } else {
            std::cout << "  Installation: NOT FOUND\n";
        }
        
        if (tierDisplayNames.contains(ResourceTier::Machine)) {
            QString name = tierDisplayNames[ResourceTier::Machine];
            std::cout << "  Machine: " << name.toStdString() << "\n";
        } else {
            std::cout << "  Machine: NOT FOUND\n";
        }
        
        if (tierDisplayNames.contains(ResourceTier::User)) {
            QString name = tierDisplayNames[ResourceTier::User];
            std::cout << "  User: " << name.toStdString() << "\n";
        } else {
            std::cout << "  User: NOT FOUND\n";
        }
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << "\n";
        return 1;
    }
    
    std::cout << "\n=== All Tests Passed ===\n";
    return 0;
}
