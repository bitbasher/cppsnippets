#ifndef DIALOGBUTTONBAR_HPP
#define DIALOGBUTTONBAR_HPP

#include <QWidget>

class QPushButton;
class QDialogButtonBox;

/**
 * @brief Widget containing dialog action buttons
 * 
 * Provides Restore Defaults, OK and Cancel buttons in a
 * horizontal layout with proper spacing.
 */
class DialogButtonBar : public QWidget {
    Q_OBJECT

public:
    explicit DialogButtonBar(QWidget* parent = nullptr);

signals:
    void restoreDefaultsClicked();
    void accepted();
    void rejected();

private:
    QPushButton* m_restoreDefaultsButton;
    QDialogButtonBox* m_buttonBox;
};

#endif // DIALOGBUTTONBAR_HPP
