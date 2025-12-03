/**
 * @file preferencesdialog.h
 * @brief Preferences dialog for resource location management
 */

#pragma once

#include <QDialog>
#include <QVector>
#include <QWidget>
#include <memory>

#include <platformInfo/ResourceLocation.h>

class QTabWidget;
class QListWidget;
class QLineEdit;
class QPushButton;
class QLabel;
class QCheckBox;
class QGroupBox;

namespace platformInfo {
class ResourceLocationManager;
}

/**
 * @brief Widget for displaying and editing resource locations for a single tier
 */
class ResourceLocationWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResourceLocationWidget(const QString& title, 
                                     bool allowAdd = true,
                                     bool allowRemove = true,
                                     QWidget* parent = nullptr);
    
    void setLocations(const QVector<platformInfo::ResourceLocation>& locations);
    QVector<platformInfo::ResourceLocation> locations() const;
    QStringList enabledPaths() const;
    
    void setReadOnly(bool readOnly);

signals:
    void locationsChanged();

private slots:
    void onAddLocation();
    void onRemoveLocation();
    void onBrowse();
    void onItemChanged();
    void onSelectionChanged();

private:
    void setupUi(const QString& title, bool allowAdd, bool allowRemove);
    void updateButtons();
    
    QListWidget* m_listWidget;
    QLineEdit* m_pathEdit;
    QLineEdit* m_nameEdit;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QPushButton* m_browseButton;
    bool m_readOnly = false;
    
    QVector<platformInfo::ResourceLocation> m_locations;
};

/**
 * @brief Preferences dialog with resource location management
 */
class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(platformInfo::ResourceLocationManager* manager,
                               QWidget* parent = nullptr);
    ~PreferencesDialog() override;

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onApply();
    void onRestoreDefaults();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    void updatePlatformInfo();
    
    platformInfo::ResourceLocationManager* m_manager;
    
    // Platform info display
    QLabel* m_platformLabel;
    QLabel* m_installDirLabel;
    QLabel* m_folderNameLabel;
    
    // Location widgets for each tier
    ResourceLocationWidget* m_installationWidget;
    ResourceLocationWidget* m_machineWidget;
    ResourceLocationWidget* m_userWidget;
    
    // Buttons
    QPushButton* m_applyButton;
    QPushButton* m_restoreDefaultsButton;
    
    QTabWidget* m_tabWidget;
};
