#include "gui/userTab.hpp"
#include "gui/resourceLocationWidget.hpp"

#include <QVBoxLayout>
#include <QDir>

UserTab::UserTab(QWidget* parent)
    : QWidget(parent)
{
    m_locationWidget = new ResourceLocationWidget(tr("User Locations"), true, true);
    connect(m_locationWidget, &ResourceLocationWidget::locationsChanged,
            this, &UserTab::locationsChanged);
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_locationWidget);
    setLayout(mainLayout);
}

QString UserTab::xdgDataHomeEnv()
{
    return QString::fromLocal8Bit(qgetenv("XDG_DATA_HOME"));
}

QVector<platformInfo::ResourceLocation> UserTab::xdgDataHomeLocations()
{
    QVector<platformInfo::ResourceLocation> locations;
    QString envValue = xdgDataHomeEnv();
    
#ifdef Q_OS_WIN
    // On Windows, only show if XDG_DATA_HOME is actually defined
    if (envValue.isEmpty()) {
        return locations;  // Return empty - don't show on Windows if not defined
    }
#endif
    
    if (envValue.isEmpty()) {
        // On POSIX/Mac: show as disabled placeholder when not defined
        platformInfo::ResourceLocation loc;
        loc.path = QString();
        loc.displayName = QObject::tr("XDG_DATA_HOME (not set)");
        loc.description = QObject::tr("Set the XDG_DATA_HOME environment variable to add user data paths");
        loc.isEnabled = false;
        loc.exists = false;
        loc.isWritable = false;
        loc.hasResourceFolders = false;
        locations.append(loc);
    } else {
        // XDG_DATA_HOME is defined - parse the path list
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
            loc.displayName = QObject::tr("XDG_DATA_HOME: %1").arg(fullPath);
            loc.description = QObject::tr("From environment variable XDG_DATA_HOME");
            loc.isEnabled = true;  // User can toggle
            loc.exists = QDir(fullPath).exists();
            loc.isWritable = loc.exists;  // User paths typically writable
            
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
