#ifndef USERTAB_HPP
#define USERTAB_HPP

#include <QWidget>
#include <platformInfo/ResourceLocation.hpp>

class ResourceLocationWidget;

/**
 * @brief User resources tab - shows user-specific resource locations
 * 
 * This tab displays and allows editing of user-specific (personal)
 * resource locations.
 * 
 * Also displays the OPENSCAD_PATH environment variable status:
 * - If undefined: shown as disabled placeholder
 * - If defined: shown with path, checkbox controls whether it's searched
 */
class UserTab : public QWidget {
    Q_OBJECT

public:
    explicit UserTab(QWidget* parent = nullptr);
    
    ResourceLocationWidget* locationWidget() const { return m_locationWidget; }
    
    /**
     * @brief Get the OPENSCAD_PATH environment variable value
     * @return The path if set, empty string if not defined
     * 
     * @note This is the same as MachineTab::openscadPathEnv() but provided
     *       here for convenience when working with the User tier.
     */
    static QString openscadPathEnv();
    
    /**
     * @brief Create a ResourceLocation for the OPENSCAD_PATH env var
     * @return ResourceLocation configured based on whether env var is set
     * 
     * @note This is the same as MachineTab::openscadPathLocation() but provided
     *       here for convenience when working with the User tier.
     */
    static platformInfo::ResourceLocation openscadPathLocation();
    
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
    static QList<platformInfo::ResourceLocation> xdgDataHomeLocations();

signals:
    void locationsChanged();

private:
    ResourceLocationWidget* m_locationWidget;
};

#endif // USERTAB_HPP
