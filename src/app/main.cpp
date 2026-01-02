/**
 * @file main.cpp
 * @brief Main entry point for the ScadTemplates Qt application
 */

#include <QApplication>
#include <QDebug>
#include "mainwindow.h"
#include "applicationNameInfo.hpp"

int main(int argc, char *argv[]) {
    qDebug() << "Starting" << appInfo::displayName << "application...";
    
    QApplication app(argc, argv);
    
    app.setApplicationName(appInfo::displayName);
    app.setApplicationVersion(appInfo::version);
    app.setOrganizationName(appInfo::organization);
    
    qDebug() << "Creating main window...";
    MainWindow window;
    qDebug() << "Showing main window...";
    window.show();
    qDebug() << "Entering event loop...";
    
    return app.exec();
}
