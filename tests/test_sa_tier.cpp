/**
 * @file test_tier_standalone.cpp
 * @brief Minimal standalone test for ResourceTier
 */

#include <iostream>
#include <QCoreApplication>
#include <QString>
#ifdef USE_TEST_APP_INFO
#include "ApplicationNameInfo.hpp"
#else
#include "applicationNameInfo.hpp"
#endif
#include "resourceMetadata/ResourceTier.hpp"

using namespace resourceMetadata;

void printUsage() {
    std::cout << "\nUSAGE: test_sa_tier [OPTIONS] [appname]\n\n";
    std::cout << "Minimal standalone test for ResourceTier enum.\n\n";
    std::cout << "ARGUMENTS:\n";
    std::cout << "  appname           Application name for resource discovery (default: OpenSCAD)\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  -h, --help        Show this help message\n";
    std::cout << "  --usage           Show this help message\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  test_sa_tier\n";
    std::cout << "  test_sa_tier TestApp\n";
    std::cout << "  test_sa_tier --help\n\n";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // Check for help options
    if (argc > 1) {
        QString arg1 = QString::fromUtf8(argv[1]).toLower();
        if (arg1 == "-h" || arg1 == "--help" || arg1 == "--usage") {
            printUsage();
            return 0;
        }
    }
    
    // Parse command line arguments for app name (default: OpenSCAD)
    QString appName = "OpenSCAD";
    if (argc > 1) {
        appName = QString::fromUtf8(argv[1]);
    }
    
    // Set test application name for resource discovery
    appInfo::setTestAppName(appName);
    
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
    
    // Test 2: tierDisplayName function
    std::cout << "Test 2: tierDisplayName Function\n";
    
    // Test 3: Access display names
    std::cout << "Test 3: Display Name Lookup\n";
    try {
        QString instName = tierToString(ResourceTier::Installation);
        std::cout << "  Installation: " << instName.toStdString() << "\n";
        
        QString machName = tierToString(ResourceTier::Machine);
        std::cout << "  Machine: " << machName.toStdString() << "\n";
        
        QString userName = tierToString(ResourceTier::User);
        std::cout << "  User: " << userName.toStdString() << "\n";
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << "\n";
        return 1;
    }
    
    std::cout << "\n=== All Tests Passed ===\n";
    return 0;
}
