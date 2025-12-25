/**
 * @file mainwindow.h
 * @brief Main window for the scadtemplates application
 */

#pragma once

#include <QMainWindow>
#include <scadtemplates/template.h>
#include <resInventory/resourceItem.h>
#include <memory>

class QTextEdit;
class QLineEdit;
class QPlainTextEdit;
class QSettings;
class QPushButton;
class QVBoxLayout;
class QTreeView;

namespace scadtemplates {
class TemplateManager;
class Template;
}

namespace platformInfo {
class ResourceLocationManager;
}

namespace resInventory {
class ResourceInventoryManager;
class ResourceTreeWidget;
class ResourceItem;
class ResourceStore;
class ResourceScannerDirListing;
class TemplateTreeModel;
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
    void onCopyTemplate();
    void onSaveTemplate();
    void onEditTemplate();
    void onCancelEdit();
    void onSearch(const QString& text);
    void onPreferences();
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveFileAs();
    void onInventoryItemSelected(const resInventory::ResourceItem& item);
    void onInventorySelectionChanged();

private:
    void setupUi();
    void setupMenus();
    void updateWindowTitle();
    void updateTemplateButtons();
    void refreshInventory();
    void populateEditorFromSelection(const resInventory::ResourceItem& item);
    QString userTemplatesRoot() const;
    bool saveTemplateToUser(const scadtemplates::Template& tmpl, const QString& version);
    QString incrementVersion(const QString& version) const;
    void applyFilterToTree(const QString& text);
    
    std::unique_ptr<scadtemplates::TemplateManager> m_templateManager;
    std::unique_ptr<platformInfo::ResourceLocationManager> m_resourceManager;
    std::unique_ptr<resInventory::ResourceInventoryManager> m_inventoryManager;
    std::unique_ptr<QSettings> m_settings;
    
    // New resource management
    std::unique_ptr<resInventory::ResourceStore> m_resourceStore;
    std::unique_ptr<resInventory::ResourceScannerDirListing> m_scanner;
    
    // Template panel
    QVBoxLayout* m_inventoryLayout;
    QTreeView* m_templateTree;
    resInventory::TemplateTreeModel* m_templateModel;
    QLineEdit* m_prefixEdit;
    QTextEdit* m_bodyEdit;
    QTextEdit* m_descriptionEdit;
    QLineEdit* m_searchEdit;
    QLineEdit* m_sourceEdit; // non-editable provenance field
    QLineEdit* m_versionEdit; // non-editable version field
    QPushButton* m_newBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_copyBtn;
    QPushButton* m_editBtn;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
    
    // Save destination for new/copied templates
    QString m_saveDestinationPath;
    
    // Main editor
    QPlainTextEdit* m_editor;
    QString m_currentFile;
    bool m_modified = false;
    bool m_editMode = false;
    resInventory::ResourceItem m_selectedItem;
};
