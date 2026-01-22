#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QDebug>

#include "mainwindow.hpp"
#include "app/Inventories.hpp"
#include "resourceInventory/TemplatesInventory.hpp"
#include "resourceInventory/ExamplesInventory.hpp"
#include "resourceInventory/UnknownInventory.hpp"
#include "applicationNameInfo.hpp"

// Global inventory pointers - declare but don't call resourceManager
resourceInventory::TemplatesInventory* g_templatesInventory = nullptr;
resourceInventory::ExamplesInventory* g_examplesInventory = nullptr;
resourceInventory::UnknownInventory* g_fontsInventory = nullptr;
resourceInventory::UnknownInventory* g_shadersInventory = nullptr;
resourceInventory::UnknownInventory* g_librariesInventory = nullptr;
resourceInventory::UnknownInventory* g_testsInventory = nullptr;
resourceInventory::UnknownInventory* g_translationsInventory = nullptr;
resourceInventory::UnknownInventory* g_colorSchemesInventory = nullptr;
resourceInventory::UnknownInventory* g_unknownInventory = nullptr;

int main(int argc, char *argv[]) {
    qDebug() << "\n========== TEST: Examples Tab Builder ==========\n";
    
    QApplication app(argc, argv);
    
    app.setApplicationName(appInfo::displayName);
    app.setApplicationVersion(appInfo::version);
    app.setOrganizationName(appInfo::organization);
    
    qDebug() << "Step 1: Creating empty inventories (no resource discovery)...";
    g_templatesInventory = new resourceInventory::TemplatesInventory();
    g_examplesInventory = new resourceInventory::ExamplesInventory();
    g_fontsInventory = new resourceInventory::UnknownInventory();
    g_shadersInventory = new resourceInventory::UnknownInventory();
    g_librariesInventory = new resourceInventory::UnknownInventory();
    g_testsInventory = new resourceInventory::UnknownInventory();
    g_translationsInventory = new resourceInventory::UnknownInventory();
    g_colorSchemesInventory = new resourceInventory::UnknownInventory();
    g_unknownInventory = new resourceInventory::UnknownInventory();
    qDebug() << "✓ Inventories created (empty)\n";
    
    qDebug() << "Step 2: Creating test window...";
    QWidget window;
    window.setWindowTitle("TEST: Examples Tab Only");
    window.resize(1200, 800);
    
    QVBoxLayout* layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0, 0, 0, 0);
    qDebug() << "✓ Test window created\n";
    
    qDebug() << "Step 3: Creating MainWindow...";
    QWidget* tempParent = new QWidget(&window);
    MainWindow* mainWindow = new MainWindow(tempParent);
    qDebug() << "✓ MainWindow instance created\n";
    
    qDebug() << "Step 4: Creating dummy container for examples tab...";
    QTabWidget* testTabs = new QTabWidget(&window);
    qDebug() << "✓ Test container created\n";
    
    qDebug() << "Step 5: Calling setupExamplesTab()...\n";
    try {
        mainWindow->setupExamplesTab(testTabs);
        qDebug() << "\n✓✓✓ setupExamplesTab() SUCCEEDED! ✓✓✓\n";
    } catch (const std::exception& e) {
        qCritical() << "\n✗✗✗ EXCEPTION in setupExamplesTab():" << e.what() << "\n";
        return 1;
    } catch (...) {
        qCritical() << "\n✗✗✗ UNKNOWN EXCEPTION in setupExamplesTab()\n";
        return 1;
    }
    
    layout->addWidget(testTabs);
    window.show();
    
    qDebug() << "Step 6: Window displayed. Entering event loop...\n";
    
    return app.exec();
}
