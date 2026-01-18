#include "gui/userTab.hpp"
#include "gui/resourceLocationWidget.hpp"
#include "gui/machineTab.hpp"  // For openscadPathEnv/openscadPathLocation

#include <QVBoxLayout>
#include <QDir>

UserTab::UserTab(QWidget* parent)
    : QWidget(parent)
{
    m_locationWidget = new ResourceLocationWidget(tr("User Resources"), true, true);
    connect(m_locationWidget, &ResourceLocationWidget::locationsChanged,
            this, &UserTab::locationsChanged);
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_locationWidget);
    setLayout(mainLayout);
}

QString UserTab::openscadPathEnv()
{
    // Delegate to MachineTab to avoid code duplication
    return MachineTab::openscadPathEnv();
}

platformInfo::ResourceLocation UserTab::openscadPathLocation()
{
    // Delegate to MachineTab to avoid code duplication
    return MachineTab::openscadPathLocation();
}

QString UserTab::xdgDataHomeEnv()
{
    return QString::fromLocal8Bit(qgetenv("XDG_DATA_HOME"));
}

// FIXME : check that this works correctly
QList<platformInfo::ResourceLocation> UserTab::xdgDataHomeLocations()
{
    QList<platformInfo::ResourceLocation> locations;
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
        loc.setPath(QString());
        // Display name auto-generated from path
        loc.setDescription(QObject::tr("Set the XDG_DATA_HOME environment variable to add user data paths"));
        loc.setHasResourceFolders(false);
        locations.append(loc);
    } else {
        // XDG_DATA_HOME is defined - parse the path list
        // Use ':' as separator on POSIX, ';' on Windows
#ifdef Q_OS_WIN
        QChar separator = QLatin1Char(';');
#else
        QChar separator = QLatin1Char(':'); // FIXME : use correct Qt method to get separator and get rid of #IFDEF block
#endif
        QStringList paths = envValue.split(separator, Qt::SkipEmptyParts);
        
        for (const QString& basePath : paths) {
            // Append /openscad to each path
            QString fullPath = QDir::cleanPath(basePath + QStringLiteral("/openscad"));
            
            platformInfo::ResourceLocation loc;
            loc.setPath(fullPath);
            // Display name auto-generated from path
            loc.setDescription(QObject::tr("From environment variable XDG_DATA_HOME"));
            
            // FIXME : nned a correct way to check for resource folders
            locations.append(loc);
        }
    }
    
    return locations;
}
