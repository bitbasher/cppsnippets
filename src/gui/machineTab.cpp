#include "gui/machineTab.hpp"
#include "gui/resourceLocationWidget.hpp"

#include <QVBoxLayout>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>  // For IsUserAnAdmin
#pragma comment(lib, "shell32.lib")
#endif

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

MachineTab::MachineTab(QWidget* parent)
    : QWidget(parent)
{
    m_locationWidget = new ResourceLocationWidget(tr("Machine Resources"), true, true);
    connect(m_locationWidget, &ResourceLocationWidget::locationsChanged,
            this, &MachineTab::locationsChanged);
    
    // Disable editing for non-admin users
    if (!isUserAdmin()) {
        m_locationWidget->setInputVisible(false);
        m_locationWidget->setReadOnly(true);
    }
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_locationWidget);
    setLayout(mainLayout);
}

bool MachineTab::isUserAdmin()
{
#ifdef Q_OS_WIN
    // Use IsUserAnAdmin from shell32
    return IsUserAnAdmin() != FALSE;
#elif defined(Q_OS_UNIX)
    // On Unix, check if running as root
    return geteuid() == 0;
#else
    // Default to true on unknown platforms
    return true;
#endif
}

QString MachineTab::openscadPathEnv()
{
    return QString::fromLocal8Bit(qgetenv("OPENSCAD_PATH"));
}

platformInfo::ResourceLocation MachineTab::openscadPathLocation()
{
    platformInfo::ResourceLocation loc;
    QString envPath = openscadPathEnv();
    
    if (envPath.isEmpty()) {
        // OPENSCAD_PATH is not defined
        loc.path = QString();
        loc.displayName = QObject::tr("OPENSCAD_PATH (not set)");
        loc.description = QObject::tr("Set the OPENSCAD_PATH environment variable to add a custom search path");
        loc.isEnabled = false;
        loc.exists = false;
        loc.isWritable = false;
        loc.hasResourceFolders = false;
    } else {
        // OPENSCAD_PATH is defined
        loc.path = envPath;
        loc.displayName = QObject::tr("OPENSCAD_PATH");
        loc.description = QObject::tr("From environment variable: %1").arg(envPath);
        loc.isEnabled = true;  // User can toggle this
        loc.exists = QDir(envPath).exists();
        loc.isWritable = loc.exists;  // Simplified check
        
        // Check if it has resource folders
        if (loc.exists) {
            QDir dir(envPath);
            loc.hasResourceFolders = dir.exists(QStringLiteral("examples")) ||
                                      dir.exists(QStringLiteral("fonts")) ||
                                      dir.exists(QStringLiteral("libraries")) ||
                                      dir.exists(QStringLiteral("color-schemes"));
        }
    }
    
    return loc;
}

QString MachineTab::xdgDataDirsEnv()
{
    return QString::fromLocal8Bit(qgetenv("XDG_DATA_DIRS"));
}

QVector<platformInfo::ResourceLocation> MachineTab::xdgDataDirsLocations()
{
    QVector<platformInfo::ResourceLocation> locations;
    QString envValue = xdgDataDirsEnv();
    
#ifdef Q_OS_WIN
    // On Windows, only show if XDG_DATA_DIRS is actually defined
    if (envValue.isEmpty()) {
        return locations;  // Return empty - don't show on Windows if not defined
    }
#endif
    
    if (envValue.isEmpty()) {
        // On POSIX/Mac: show as disabled placeholder when not defined
        platformInfo::ResourceLocation loc;
        loc.path = QString();
        loc.displayName = QObject::tr("XDG_DATA_DIRS (not set)");
        loc.description = QObject::tr("Set the XDG_DATA_DIRS environment variable to add system-wide data paths");
        loc.isEnabled = false;
        loc.exists = false;
        loc.isWritable = false;
        loc.hasResourceFolders = false;
        locations.append(loc);
    } else {
        // XDG_DATA_DIRS is defined - parse the path list
        // Use ':' as separator on POSIX, ';' on Windows
#ifdef Q_OS_WIN
        QChar separator = QLatin1Char(';');
#else
        QChar separator = QLatin1Char(':');
#endif
        QStringList paths = envValue.split(separator, Qt::SkipEmptyParts);
        
        for (const QString& basePath : paths) {
            // Append /openscad to each path
            QString fullPath = QDir::cleanPath(basePath + QStringLiteral("/openscad"));
            
            platformInfo::ResourceLocation loc;
            loc.path = fullPath;
            loc.displayName = QObject::tr("XDG_DATA_DIRS: %1").arg(fullPath);
            loc.description = QObject::tr("From environment variable XDG_DATA_DIRS");
            loc.isEnabled = true;  // User can toggle
            loc.exists = QDir(fullPath).exists();
            loc.isWritable = false;  // System paths typically not writable
            
            // Check if it has resource folders
            if (loc.exists) {
                QDir dir(fullPath);
                loc.hasResourceFolders = dir.exists(QStringLiteral("examples")) ||
                                          dir.exists(QStringLiteral("fonts")) ||
                                          dir.exists(QStringLiteral("libraries")) ||
                                          dir.exists(QStringLiteral("color-schemes"));
            } else {
                loc.hasResourceFolders = false;
            }
            
            locations.append(loc);
        }
    }
    
    return locations;
}