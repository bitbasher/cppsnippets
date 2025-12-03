#ifndef PLATFORMINFOWIDGET_HPP
#define PLATFORMINFOWIDGET_HPP

#include <QWidget>

class QLabel;

namespace platformInfo {
class ResourceLocationManager;
}

/**
 * @brief Widget displaying platform information
 * 
 * Shows detected platform in a group box format.
 */
class PlatformInfoWidget : public QWidget {
    Q_OBJECT

public:
    explicit PlatformInfoWidget(QWidget* parent = nullptr);
    
    /**
     * @brief Update the display with information from the manager
     */
    void updateFromManager(platformInfo::ResourceLocationManager* manager);

private:
    QLabel* m_platformLabel;
};

#endif // PLATFORMINFOWIDGET_HPP
