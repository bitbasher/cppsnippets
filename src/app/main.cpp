/**
 * @file main.cpp
 * @brief Main entry point for the cppsnippets Qt application
 */

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("CppSnippets");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CppSnippets");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
