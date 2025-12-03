#ifndef MACHINETAB_HPP
#define MACHINETAB_HPP

#include <QWidget>
#include <platformInfo/ResourceLocation.h>

class ResourceLocationWidget;

/**
 * @brief Machine resources tab - shows system-wide resource locations
 * 
 * This tab displays and allows editing of machine-wide (all users)
 * resource locations. On Windows, editing is disabled for non-admin users.
 * 
 * Also displays the OPENSCAD_PATH environment variable status:
 * - If undefined: shown as disabled placeholder
 * - If defined: shown with path, checkbox controls whether it's searched
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
     * @brief Get the OPENSCAD_PATH environment variable value
     * @return The path if set, empty string if not defined
     */
    static QString openscadPathEnv();
    
    /**
     * @brief Create a ResourceLocation for the OPENSCAD_PATH env var
     * @return ResourceLocation configured based on whether env var is set
     * 
     * If OPENSCAD_PATH is not defined:
     * - displayName: "OPENSCAD_PATH (not set)"
     * - isEnabled: false
     * - exists: false
     * 
     * If OPENSCAD_PATH is defined:
     * - path: the env var value
     * - displayName: "OPENSCAD_PATH"
     * - isEnabled: true (user can toggle)
     * - exists: checked at runtime
     */
    static platformInfo::ResourceLocation openscadPathLocation();
    
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
