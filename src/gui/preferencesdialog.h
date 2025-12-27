/**
 * @file preferencesdialog.h
 * @brief Preferences dialog for resource location management
 * 
 * Follows Qt TabDialog pattern where each tab is its own QWidget subclass.
 */

#pragma once

#include <QDialog>

class QTabWidget;

class PlatformInfoWidget;
class DialogButtonBar;
class InstallationTab;
class MachineTab;
class UserTab;
class EnvVarsTab;

namespace platformInfo {
class ResourceLocationManager;
}
namespace resInventory {
class ResourceInventoryManager;
}

/**
 * @brief Preferences dialog with resource location management
 * 
 * Assembles tab widgets for Installation, Machine, and User resource
 * locations following the Qt TabDialog pattern.
 */
class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(platformInfo::ResourceLocationManager* manager,
                               QWidget* parent = nullptr);
    ~PreferencesDialog() override;

    // Optional: set inventory manager to trigger rescan on accept
    void setInventoryManager(resInventory::ResourceInventoryManager* inventory);

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onRestoreDefaults();
    void onLocationsChanged();

private:
    void loadSettings();
    void saveSettings();
    
    platformInfo::ResourceLocationManager* m_manager;
    resInventory::ResourceInventoryManager* m_inventoryManager = nullptr;
    
    // Widgets
    PlatformInfoWidget* m_platformInfoWidget;
    QTabWidget* m_tabWidget;
    InstallationTab* m_installationTab;
    MachineTab* m_machineTab;
    UserTab* m_userTab;
    EnvVarsTab* m_envVarsTab;
    DialogButtonBar* m_buttonBar;
};
