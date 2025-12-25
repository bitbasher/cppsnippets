/**
 * @file envVarsTab.h
 * @brief Environment variables editor tab for preferences dialog
 */

#pragma once

#include <QWidget>

class QListWidget;
class QLineEdit;
class QLabel;
class QPushButton;
class QDataWidgetMapper;

namespace platformInfo {
class ResourceLocationManager;
class ResourcePaths;
}

/**
 * @brief Tab for editing user-configured environment variables
 * 
 * Provides UI for managing environment variables used in resource path templates.
 * Changes are saved to QSettings and loaded on application startup.
 */
class EnvVarsTab : public QWidget {
    Q_OBJECT

public:
    explicit EnvVarsTab(platformInfo::ResourceLocationManager* manager, QWidget* parent = nullptr);
    ~EnvVarsTab() override;

    /**
     * @brief Load environment variables from ResourceLocationManager
     */
    void loadEnvVars();
    
    /**
     * @brief Save environment variables to ResourceLocationManager and settings
     */
    void saveEnvVars();
    
    /**
     * @brief Check if there are unsaved changes
     */
    bool hasUnsavedChanges() const { return m_hasChanges; }

signals:
    void envVarsChanged();

private slots:
    void onListSelectionChanged();
    void onNameChanged(const QString& text);
    void onValueChanged(const QString& text);
    void onCopyClicked();
    void onSaveClicked();
    void onCancelClicked();
    void onRevertVarClicked();
    void onRestoreDefaultsClicked();

private:
    void setupUI();
    void updatePreview();
    void updateButtonStates();
    bool validateEnvVar(const QString& name, const QString& value, QString& errorMsg);
    QString expandPreviewPath(const QString& path) const;
    
    platformInfo::ResourceLocationManager* m_manager;
    
    // Temporary storage for editing
    QMap<QString, QString> m_editingEnvVars;
    QString m_currentEditName;
    bool m_hasChanges;
    
    // Widgets
    QListWidget* m_listWidget;
    QLineEdit* m_nameEdit;
    QLineEdit* m_valueEdit;
    QLabel* m_previewLabel;
    QPushButton* m_copyButton;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    QPushButton* m_revertButton;  // Revert changes to this variable
    QPushButton* m_restoreDefaultsButton;
};
