/**
 * @file main.cpp
 * @brief Main entry point for the cppsnippets Qt application
 */

#include <QApplication>
#include <QDebug>
#include <QLoggingCategory>
#include <cstdio>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif
#include "mainwindow.h"
#include <scadtemplates/scadtemplates.h>

// Route Qt logging to stderr so it is visible in a parent console or debugger
static void installConsoleLogging()
{
#if defined(Q_OS_WIN)
    // Attach to the parent console (if launched from a terminal) so stderr/stdout are visible
    AttachConsole(ATTACH_PARENT_PROCESS);
    FILE* dummy = nullptr;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
#endif

    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext&, const QString& msg) {
        const char* level = "[INFO]";
        switch (type) {
        case QtDebugMsg: level = "[DEBUG]"; break;
        case QtInfoMsg:  level = "[INFO ]"; break;
        case QtWarningMsg: level = "[WARN ]"; break;
        case QtCriticalMsg: level = "[ERROR]"; break;
        case QtFatalMsg: level = "[FATAL]"; break;
        }
        fprintf(stderr, "%s %s\n", level, msg.toLocal8Bit().constData());
        fflush(stderr);
    });
}

int main(int argc, char *argv[]) {
    installConsoleLogging();
    qDebug() << "Starting OpenSCAD application...";
    
    QApplication app(argc, argv);
    
    app.setApplicationName("OpenSCAD");
    app.setApplicationVersion(scadtemplates::getVersion());
    app.setOrganizationName("OpenSCAD");
    
    qDebug() << "Creating main window...";
    MainWindow window;
    qDebug() << "Showing main window...";
    window.show();
    qDebug() << "Entering event loop...";
    
    return app.exec();
}
