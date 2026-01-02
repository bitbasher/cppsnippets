/**
 * @file main.cpp
 * @brief Main entry point for the cppsnippets Qt application
 */

#include <QApplication>
#include <QDebug>
#include "mainwindow.h"
#include <scadtemplates/scadtemplates.hpp>

int main(int argc, char *argv[]) {
    qDebug() << "Starting CppSnippets application...";
    
    QApplication app(argc, argv);
    
    app.setApplicationName("ScadTemplates");
    app.setApplicationVersion(scadtemplates::getVersion());
    app.setOrganizationName("CppSnippets");
    
    qDebug() << "Creating main window...";
    MainWindow window;
    qDebug() << "Showing main window...";
    window.show();
    qDebug() << "Entering event loop...";
    
    return app.exec();
}
