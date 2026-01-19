/**
 * @file preferencesdialog.h
 * @brief Preferences dialog for resource location management
 * 
 * Follows Qt TabDialog pattern where each tab is its own QWidget subclass.
 */

#pragma once

#include <QDialog>

class PlatformInfoWidget;
class DialogButtonBar;

/**
 * @brief Preferences dialog - STUBBED
 * 
 * ResourceLocationManager was removed. This is a minimal stub until
 * the dialog is reimplemented with the new architecture.
 */
class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget* parent = nullptr);
    ~PreferencesDialog() override;

public slots:
    void accept() override;
    void reject() override;

private:
    PlatformInfoWidget* m_platformInfoWidget;
    DialogButtonBar* m_buttonBar;
};
