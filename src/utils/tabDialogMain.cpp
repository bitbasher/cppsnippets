#include "tabDialog.hpp"

#include <QApplication>

// ============================================================================
// Main - example entry point
// ============================================================================
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    QString fileName = (argc > 1) ? argv[1] : QApplication::applicationFilePath();
    
    TabDialog dialog(fileName);
    return dialog.exec();
}