#ifndef MACHINETAB_HPP
#define MACHINETAB_HPP

#include <QWidget>
#include <resInventory/ResourceLocation.h>

class ResourceLocationWidget;

/**
 * @brief Machine resources tab - shows system-wide resource locations
 * 
 * This tab displays and allows editing of machine-wide (all users)
 * resource locations. On Windows, editing is disabled for non-admin users.
 */
class MachineTab : public QWidget {
    Q_OBJECT

public:
    explicit MachineTab(QWidget* parent = nullptr);
    
    ResourceLocationWidget* locationWidget() const { return m_locationWidget; }
    
    /**
     * @brief Check if the current user has admin privileges
     * @return true if running as admin (Windows) or root (Unix)
     */
    static bool isUserAdmin();
    
    /**
     * @brief Get the XDG_DATA_DIRS environment variable value
     * @return The path list if set, empty string if not defined
     */
    static QString xdgDataDirsEnv();
    
    /**
     * @brief Create ResourceLocations for the XDG_DATA_DIRS env var paths
     * @return Vector of ResourceLocations, one per path in XDG_DATA_DIRS
     * 
     * Behavior differs by platform:
     * - Windows: Returns empty if XDG_DATA_DIRS is not defined
     * - POSIX/Mac: Returns placeholder if not defined, paths if defined
     * 
     * Each path has "/openscad" appended and is checked for existence.
     */
    static QVector<platformInfo::ResourceLocation> xdgDataDirsLocations();

signals:
    void locationsChanged();

private:
    ResourceLocationWidget* m_locationWidget;
};

#endif // MACHINETAB_HPP
