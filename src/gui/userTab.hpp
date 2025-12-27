#ifndef USERTAB_HPP
#define USERTAB_HPP

#include <QWidget>
#include <resInventory/ResourceLocation.h>

class ResourceLocationWidget;

/**
 * @brief User resources tab - shows user-specific resource locations
 * 
 * This tab displays and allows editing of user-specific (personal)
 * resource locations.
 */
class UserTab : public QWidget {
    Q_OBJECT

public:
    explicit UserTab(QWidget* parent = nullptr);
    
    ResourceLocationWidget* locationWidget() const { return m_locationWidget; }
    
    /**
     * @brief Get the XDG_DATA_HOME environment variable value
     * @return The path list if set, empty string if not defined
     */
    static QString xdgDataHomeEnv();
    
    /**
     * @brief Create ResourceLocations for the XDG_DATA_HOME env var paths
     * @return Vector of ResourceLocations, one per path in XDG_DATA_HOME
     * 
     * Behavior differs by platform:
     * - Windows: Returns empty if XDG_DATA_HOME is not defined
     * - POSIX/Mac: Returns placeholder if not defined, paths if defined
     * 
     * Each path has "/openscad" appended and is checked for existence.
     */
    static QVector<platformInfo::ResourceLocation> xdgDataHomeLocations();

signals:
    void locationsChanged();

private:
    ResourceLocationWidget* m_locationWidget;
};

#endif // USERTAB_HPP
