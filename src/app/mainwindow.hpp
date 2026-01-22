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
class QTabWidget;
class QSplitter;
class QGroupBox;
class QMenu;
class QLabel;

namespace resourceInventory {
class ResourceItem;
class ResourceTemplate;
class ResourceScript;
class TemplatesInventory;
class ExamplesInventory;
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

    // Public for testing/factoring purposes
    void setupTemplatesTab(QWidget* parentContainer);
    void setupExamplesTab(QWidget* parentContainer);
    void setupMainEditor(QWidget* parentContainer);

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
    void onResourceTabChanged(int index);

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
    void setEditorFieldsEnabled(bool enabled);
    void populateEditorFromSelection(const resourceInventory::ResourceTemplate& item);
    QString userTemplatesRoot() const;
    void applyFilterToTree(const QString& text);
    bool loadTemplatesFromFile(const QString& filePath);
    bool saveTemplatesToFile(const QString& filePath) const;
    bool saveTemplateToUser(const resourceInventory::ResourceTemplate& tmpl);
    bool saveTemplateToPath(const QString& filePath, const QString& prefix, const QString& body, const QString& description);
    void refreshInventory();  // TODO: Implement after save/delete work
    
    std::unique_ptr<QSettings> m_settings;
    resourceInventory::TemplatesInventory* m_inventory;  // Owned by QApplication
    
    // Tab widget for resource type selection
    QTabWidget* m_resourceTabs = nullptr;
    
    // Templates tab components
    QTreeView* m_templateTree = nullptr;
    QSplitter* m_templatesSplitter = nullptr;  // Vertical splitter: tree + editor
    
    // Examples tab components
    QTreeView* m_exampleTree = nullptr;
    
    // Template editor (small panel on templates tab)
    QGroupBox* m_editorGroup = nullptr;
    QLineEdit* m_prefixEdit = nullptr;
    
    // Main editor
    QPlainTextEdit* m_editor = nullptr;
    QString m_currentFile;
    bool m_modified = false;
    bool m_editMode = false;
    resourceInventory::ResourceTemplate m_selectedItem;
    
    // Search and buttons (Templates tab)
    QLineEdit* m_searchEdit = nullptr;
    QPushButton* m_newBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QPushButton* m_copyBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
    
    // Editor fields (Templates tab)
    QTextEdit* m_bodyEdit = nullptr;
    QTextEdit* m_descriptionEdit = nullptr;
    QLineEdit* m_sourceEdit = nullptr;  // non-editable provenance field
};
