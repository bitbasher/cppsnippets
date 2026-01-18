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
class QStandardItemModel;
class QTreeView;

namespace resourceInventory {
class ResourceTreeWidget;
class ResourceItem;

using ResourceTemplate;
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
    explicit MainWindow(QStandardItemModel* inventory, QWidget *parent = nullptr);
    
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
    void onInventoryItemSelected(const resourceInventory::ResourceItem& item);
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
    void populateEditorFromSelection(const resourceInventory::ResourceItem& item);
    QString userTemplatesRoot() const;
    bool saveTemplateToUser(const ResourceTemplate& tmpl);
    void applyFilterToTree(const QString& text);
    bool loadTemplatesFromFile(const QString& filePath);
    bool saveTemplatesToFile(const QString& filePath) const;
    
    std::unique_ptr<QSettings> m_settings;
    QStandardItemModel* m_inventory;  // Owned by QApplication
    
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
    resourceInventory::ResourceItem m_selectedItem;
};
