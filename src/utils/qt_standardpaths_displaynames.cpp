// Qt CLI utility to print displayName and paths for each QStandardPaths::StandardLocation
#include <QCoreApplication>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);

    // List of all QStandardPaths::StandardLocation enum values
    const QStandardPaths::StandardLocation locations[] = {
        QStandardPaths::DesktopLocation,
        QStandardPaths::DocumentsLocation,
        QStandardPaths::FontsLocation,
        QStandardPaths::ApplicationsLocation,
        QStandardPaths::MusicLocation,
        QStandardPaths::MoviesLocation,
        QStandardPaths::PicturesLocation,
        QStandardPaths::TempLocation,
        QStandardPaths::HomeLocation,
        QStandardPaths::AppLocalDataLocation,
        QStandardPaths::CacheLocation,
        QStandardPaths::GenericDataLocation,
        QStandardPaths::RuntimeLocation,
        QStandardPaths::ConfigLocation,
        QStandardPaths::DownloadLocation,
        QStandardPaths::GenericCacheLocation,
        QStandardPaths::GenericConfigLocation,
        QStandardPaths::AppDataLocation,
        QStandardPaths::AppConfigLocation,
        QStandardPaths::PublicShareLocation,
        QStandardPaths::TemplatesLocation,
        QStandardPaths::StateLocation,
        QStandardPaths::GenericStateLocation
    };

    out << "Qt StandardPaths on this platform:\n";
    out << QString("-").repeated(80) << "\n";
    
    for (QStandardPaths::StandardLocation loc : locations) {
        QString name = QStandardPaths::displayName(loc);
        QString path = QStandardPaths::writableLocation(loc);
        out << QString("%1: %2\n").arg(static_cast<int>(loc), 2).arg(name, -25);
        out << "    Path: " << (path.isEmpty() ? "(none)" : path) << "\n";
    }
    out.flush();
    return 0;
}
