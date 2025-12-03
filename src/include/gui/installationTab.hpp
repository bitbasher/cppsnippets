#ifndef INSTALLATIONTAB_HPP
#define INSTALLATIONTAB_HPP

#include <QWidget>

class ResourceLocationWidget;

/**
 * @brief Installation resources tab - shows built-in and sibling installations
 * 
 * This tab displays resources from the application installation directory
 * and any sibling installations that may be detected.
 */
class InstallationTab : public QWidget {
    Q_OBJECT

public:
    explicit InstallationTab(QWidget* parent = nullptr);
    
    ResourceLocationWidget* locationWidget() const { return m_locationWidget; }
    
    /**
     * @brief Set whether fallback mode is active
     * 
     * In fallback mode, the input widget is shown to allow the user to
     * specify a custom installation path. This is used when no valid
     * installation is detected.
     */
    void setFallbackMode(bool fallback);
    bool isFallbackMode() const { return m_fallbackMode; }

signals:
    void locationsChanged();

private:
    ResourceLocationWidget* m_locationWidget;
    bool m_fallbackMode = false;
};

#endif // INSTALLATIONTAB_HPP
