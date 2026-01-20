/**
 * @file mainwindow.h
 * @brief Main window for the scadtemplates application
 */

#pragma once

#include <QMainWindow>
#include <resourceInventory/resourceItem.hpp>
#include <memory>

class QTextEdit;
class QLineEdit;
class QPlainTextEdit;
class QSettings;
class QPushButton;
class QVBoxLayout;
class QTreeView;

namespace resourceInventory {
class ResourceItem;
class ResourceTemplate;
class TemplatesInventory;
}

/**
 * @brief Main application window
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor
    * @param inventory Pre-built resource inventory model
     * @param parent Parent widget
     */
    explicit MainWindow(resourceInventory::TemplatesInventory* inventory, QWidget *parent = nullptr);
    
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
    void onInventoryItemSelected(const resourceInventory::ResourceTemplate& item);
    void onInventorySelectionChanged();

private:
    void setupUi();
    void setupMenus();
    void buildMenuBar();
    void buildFileMenu(QMenu* fileMenu);
    void buildEditMenu(QMenu* editMenu);
    void buildTemplatesMenu(QMenu* templatesMenu);
    void buildHelpMenu(QMenu* helpMenu);
    void updateWindowTitle();
    void updateTemplateButtons();
    void populateEditorFromSelection(const resourceInventory::ResourceTemplate& item);
    QString userTemplatesRoot() const;
    void applyFilterToTree(const QString& text);
    bool loadTemplatesFromFile(const QString& filePath);
    bool saveTemplatesToFile(const QString& filePath) const;
    bool saveTemplateToUser(const resourceInventory::ResourceTemplate& tmpl);
    void refreshInventory();  // TODO: Implement after save/delete work
    
    std::unique_ptr<QSettings> m_settings;
    resourceInventory::TemplatesInventory* m_inventory;  // Owned by QApplication
    
    // Template panel
    QVBoxLayout* m_inventoryLayout;
    QTreeView* m_templateTree;
    QLineEdit* m_prefixEdit;
    QTextEdit* m_bodyEdit;
    QTextEdit* m_descriptionEdit;
    QLineEdit* m_searchEdit;
    QLineEdit* m_sourceEdit; // non-editable provenance field
    QPushButton* m_newBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_copyBtn;
    QPushButton* m_editBtn;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
    
    // Main editor
    QPlainTextEdit* m_editor;
    QString m_currentFile;
    bool m_modified = false;
    bool m_editMode = false;
    resourceInventory::ResourceTemplate m_selectedItem;
};
