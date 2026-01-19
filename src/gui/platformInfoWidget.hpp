#ifndef PLATFORMINFOWIDGET_HPP
#define PLATFORMINFOWIDGET_HPP

#include <QWidget>

class QLabel;

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
     * @brief Update the display with platform information
     */
    void updateFromManager();

private:
    QLabel* m_platformLabel;
};

#endif // PLATFORMINFOWIDGET_HPP
