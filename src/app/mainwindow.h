/**
 * @file mainwindow.h
 * @brief Main window for the scadtemplates application
 */

#pragma once

#include <QMainWindow>
#include <memory>

class QListWidget;
class QTextEdit;
class QLineEdit;
class QPlainTextEdit;
class QSettings;

namespace scadtemplates {
class TemplateManager;
}

namespace platformInfo {
class ResourceLocationManager;
}

namespace resInventory {
class ResourceInventoryManager;
}

/**
 * @brief Main application window
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~MainWindow() override;

private slots:
    void onNewTemplate();
    void onDeleteTemplate();
    void onSaveTemplate();
    void onTemplateSelected();
    void onSearch(const QString& text);
    void onPreferences();
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveFileAs();

private:
    void setupUi();
    void setupMenus();
    void refreshTemplateList();
    void updateWindowTitle();
    
    std::unique_ptr<scadtemplates::TemplateManager> m_templateManager;
    std::unique_ptr<platformInfo::ResourceLocationManager> m_resourceManager;
    std::unique_ptr<resInventory::ResourceInventoryManager> m_inventoryManager;
    std::unique_ptr<QSettings> m_settings;
    
    // Template panel
    QListWidget* m_templateList;
    QLineEdit* m_prefixEdit;
    QTextEdit* m_bodyEdit;
    QLineEdit* m_descriptionEdit;
    QLineEdit* m_searchEdit;
    
    // Main editor
    QPlainTextEdit* m_editor;
    QString m_currentFile;
    bool m_modified = false;
};
