/**
 * @file mainwindow.cpp
 * @brief Main window implementation with text editor and preferences
 */

#include "mainwindow.hpp"
#include "Inventories.hpp"
#include "gui/preferencesdialog.hpp"
#include "gui/aboutDialog.hpp"
#include "applicationNameInfo.hpp"
#include <resourceInventory/TemplatesInventory.hpp>
#include <resourceInventory/inventoryOperations.hpp>
#include <scadtemplates/template_manager.hpp>
#include <platformInfo/ResourceLocation.hpp>
#include <resourceInventory/resourceItem.hpp>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QDir>
#include <QHeaderView>
#include <QTabWidget>
#include <resourceInventory/ExamplesInventory.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_settings(std::make_unique<QSettings>(QStringLiteral("OpenSCAD"), QStringLiteral("ScadTemplates")))
    , m_inventory(g_templatesInventory)
{
    setupUi();
    setupMenus();
    
    updateWindowTitle();
    resize(1200, 800);
    
    // Restore window geometry
    restoreGeometry(m_settings->value(QStringLiteral("MainWindow/geometry")).toByteArray());
    restoreState(m_settings->value(QStringLiteral("MainWindow/state")).toByteArray());
    
    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow() {
    // Save window geometry
    m_settings->setValue(QStringLiteral("MainWindow/geometry"), saveGeometry());
    m_settings->setValue(QStringLiteral("MainWindow/state"), saveState());
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    
    // Create main horizontal splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // ========== LEFT PANEL: Tab widget for resource types ==========
    m_resourceTabs = new QTabWidget(this);
    
    // Build each tab
    setupTemplatesTab(m_resourceTabs);
    setupExamplesTab(m_resourceTabs);
    
    // Connect tab change signal
    connect(m_resourceTabs, QOverload<int>::of(&QTabWidget::currentChanged),
            this, &MainWindow::onResourceTabChanged);
    
    mainSplitter->addWidget(m_resourceTabs);
    
    // Setup main editor panel
    setupMainEditor(mainSplitter);
    
    // Set initial sizes (1:2 ratio for left:right)
    mainSplitter->setSizes({400, 800});
    
    mainLayout->addWidget(mainSplitter);
}

void MainWindow::setupTemplatesTab(QWidget* parentContainer) {
    QWidget* templatesTabWidget = new QWidget();
    QVBoxLayout* templatesTabLayout = new QVBoxLayout(templatesTabWidget);
    templatesTabLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create vertical splitter: tree (top) + editor (bottom)
    m_templatesSplitter = new QSplitter(Qt::Vertical, this);
    
    // Search + tree section
    QGroupBox* listGroup = new QGroupBox(tr("Templates"), this);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search templates..."));
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearch);
    listLayout->addWidget(m_searchEdit);
    
    // Create tree view with the pre-built model
    m_templateTree = new QTreeView(this);
    m_templateTree->setModel(m_inventory);
    m_templateTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_templateTree->setAlternatingRowColors(true);
    m_templateTree->setRootIsDecorated(true);
    m_templateTree->setSortingEnabled(true);
    m_templateTree->setUniformRowHeights(true);
    // Configure columns
    m_templateTree->header()->setStretchLastSection(false);
    m_templateTree->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_templateTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_templateTree->setColumnWidth(0, 300);
    m_templateTree->expandAll();
    connect(m_templateTree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onInventorySelectionChanged);
    listLayout->addWidget(m_templateTree);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_newBtn = new QPushButton(tr("New"), this);
    m_deleteBtn = new QPushButton(tr("Delete"), this);
    m_copyBtn = new QPushButton(tr("Copy"), this);
    m_editBtn = new QPushButton(tr("Edit"), this);
    m_saveBtn = new QPushButton(tr("Save"), this);
    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    
    connect(m_newBtn, &QPushButton::clicked, this, &MainWindow::onNewTemplate);
    connect(m_deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteTemplate);
    connect(m_copyBtn, &QPushButton::clicked, this, &MainWindow::onCopyTemplate);
    connect(m_editBtn, &QPushButton::clicked, this, &MainWindow::onEditTemplate);
    connect(m_saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveTemplate);
    connect(m_cancelBtn, &QPushButton::clicked, this, &MainWindow::onCancelEdit);
    
    buttonLayout->addWidget(m_newBtn);
    buttonLayout->addWidget(m_deleteBtn);
    buttonLayout->addWidget(m_copyBtn);
    buttonLayout->addWidget(m_editBtn);
    listLayout->addLayout(buttonLayout);
    
    updateTemplateButtons();
    
    m_templatesSplitter->addWidget(listGroup);
    
    // Small editor panel (bottom of Templates tab)
    m_editorGroup = new QGroupBox(tr("Template Editor"), this);
    QVBoxLayout* editorLayout = new QVBoxLayout(m_editorGroup);
    
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel(tr("Name:"), this));
    m_prefixEdit = new QLineEdit(this);
    nameLayout->addWidget(m_prefixEdit);
    editorLayout->addLayout(nameLayout);
    
    QHBoxLayout* sourceLayout = new QHBoxLayout();
    sourceLayout->addWidget(new QLabel(tr("Source:"), this));
    m_sourceEdit = new QLineEdit(this);
    m_sourceEdit->setReadOnly(true);
    sourceLayout->addWidget(m_sourceEdit);
    editorLayout->addLayout(sourceLayout);
    
    editorLayout->addWidget(new QLabel(tr("Description:"), this));
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setMaximumHeight(80);
    editorLayout->addWidget(m_descriptionEdit);
    
    editorLayout->addWidget(new QLabel(tr("Body:"), this), 0, Qt::AlignTop);
    m_bodyEdit = new QTextEdit(this);
    m_bodyEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_bodyEdit->setMaximumHeight(150);
    editorLayout->addWidget(m_bodyEdit);
    
    QHBoxLayout* editorButtonLayout = new QHBoxLayout();
    editorButtonLayout->addWidget(m_saveBtn);
    editorButtonLayout->addWidget(m_cancelBtn);
    editorLayout->addLayout(editorButtonLayout);
    
    m_templatesSplitter->addWidget(m_editorGroup);
    m_templatesSplitter->setSizes({300, 150});
    
    templatesTabLayout->addWidget(m_templatesSplitter);
    
    // Cast to QTabWidget and add tab
    QTabWidget* tabs = qobject_cast<QTabWidget*>(parentContainer);
    if (tabs) {
        tabs->addTab(templatesTabWidget, tr("Templates"));
    }
}

void MainWindow::setupExamplesTab(QWidget* parentContainer) {
    QWidget* examplesTabWidget = new QWidget();
    QVBoxLayout* examplesTabLayout = new QVBoxLayout(examplesTabWidget);
    examplesTabLayout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* examplesGroup = new QGroupBox(tr("Examples"), this);
    QVBoxLayout* examplesLayout = new QVBoxLayout(examplesGroup);
    
    m_exampleTree = new QTreeView(this);
    if (g_examplesInventory) {
        m_exampleTree->setModel(g_examplesInventory);
    }
    m_exampleTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_exampleTree->setAlternatingRowColors(true);
    m_exampleTree->setRootIsDecorated(true);
    m_exampleTree->setSortingEnabled(false);
    m_exampleTree->setUniformRowHeights(true);
    m_exampleTree->expandAll();
    examplesLayout->addWidget(m_exampleTree);
    
    examplesTabLayout->addWidget(examplesGroup);
    
    // Cast to QTabWidget and add tab
    QTabWidget* tabs = qobject_cast<QTabWidget*>(parentContainer);
    if (tabs) {
        tabs->addTab(examplesTabWidget, tr("Examples"));
    }
}

void MainWindow::setupMainEditor(QWidget* parentContainer) {
    QWidget* rightPanel = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* mainEditorGroup = new QGroupBox(tr("Editor"), this);
    QVBoxLayout* mainEditorLayout = new QVBoxLayout(mainEditorGroup);
    
    m_editor = new QPlainTextEdit(this);
    m_editor->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_editor->setTabStopDistance(40);
    connect(m_editor, &QPlainTextEdit::textChanged, this, [this]() {
        if (!m_modified) {
            m_modified = true;
            updateWindowTitle();
        }
    });
    mainEditorLayout->addWidget(m_editor);
    
    rightLayout->addWidget(mainEditorGroup);
    
    // Cast to QSplitter and add widget
    QSplitter* splitter = qobject_cast<QSplitter*>(parentContainer);
    if (splitter) {
        splitter->addWidget(rightPanel);
    }
}

void MainWindow::onResourceTabChanged(int index) {
    qDebug() << "Resource tab changed to:" << index;
    // Tab 0 = Templates, Tab 1 = Examples
    // Can add syntax highlighting switching here in Phase 5
}

void MainWindow::setupMenus() {
    buildMenuBar();
}

void MainWindow::buildMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    buildFileMenu(fileMenu);
    
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    buildEditMenu(editMenu);
    
    QMenu* templatesMenu = menuBar()->addMenu(tr("&Templates"));
    buildTemplatesMenu(templatesMenu);
    
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    buildHelpMenu(helpMenu);
}

void MainWindow::buildFileMenu(QMenu* fileMenu) {
    QAction* newAction = fileMenu->addAction(tr("&New"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewFile);
    
    QAction* openAction = fileMenu->addAction(tr("&Open..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    
    QAction* saveAction = fileMenu->addAction(tr("&Save"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);
    
    QAction* saveAsAction = fileMenu->addAction(tr("Save &As..."));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::onSaveFileAs);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
}

void MainWindow::buildEditMenu(QMenu* editMenu) {
    QAction* undoAction = editMenu->addAction(tr("&Undo"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, m_editor, &QPlainTextEdit::undo);
    
    QAction* redoAction = editMenu->addAction(tr("&Redo"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, m_editor, &QPlainTextEdit::redo);
    
    editMenu->addSeparator();
    
    QAction* cutAction = editMenu->addAction(tr("Cu&t"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, m_editor, &QPlainTextEdit::cut);
    
    QAction* copyAction = editMenu->addAction(tr("&Copy"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, m_editor, &QPlainTextEdit::copy);
    
    QAction* pasteAction = editMenu->addAction(tr("&Paste"));
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, m_editor, &QPlainTextEdit::paste);
    
    editMenu->addSeparator();
    
    QAction* selectAllAction = editMenu->addAction(tr("Select &All"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, m_editor, &QPlainTextEdit::selectAll);
    
    editMenu->addSeparator();
    
    QAction* preferencesAction = editMenu->addAction(tr("&Preferences..."));
    preferencesAction->setShortcut(QKeySequence::Preferences);
    connect(preferencesAction, &QAction::triggered, this, &MainWindow::onPreferences);
}

void MainWindow::buildTemplatesMenu(QMenu* templatesMenu) {
    QAction* loadTemplatesAction = templatesMenu->addAction(tr("&Load Templates..."));
    connect(loadTemplatesAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Templates File"), QString(),
            tr("JSON Files (*.json);;All Files (*)"));
        if (!fileName.isEmpty()) {
            if (loadTemplatesFromFile(fileName)) {
                // TODO: Add loaded templates to model display
                statusBar()->showMessage(tr("Loaded templates from %1").arg(fileName));
            } else {
                QMessageBox::warning(this, tr("Error"), 
                    tr("Failed to load templates from %1").arg(fileName));
            }
        }
    });
    
    QAction* saveTemplatesAction = templatesMenu->addAction(tr("&Save Templates..."));
    connect(saveTemplatesAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Templates File"), QString(),
            tr("JSON Files (*.json);;All Files (*)"));
        if (!fileName.isEmpty()) {
            if (saveTemplatesToFile(fileName)) {
                statusBar()->showMessage(tr("Saved templates to %1").arg(fileName));
            } else {
                QMessageBox::warning(this, tr("Error"),
                    tr("Failed to save templates to %1").arg(fileName));
            }
        }
    });
}

void MainWindow::buildHelpMenu(QMenu* helpMenu) {
    QAction* aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QString resourceDir = userTemplatesRoot();
        QString platform = QStringLiteral("Windows (MSVC)");
        
        AboutDialog* dialog = new AboutDialog(this, appInfo::version, platform, resourceDir);
        dialog->exec();
        delete dialog;
    });
}

void MainWindow::updateWindowTitle() {
    QString title = QStringLiteral("ScadTemplates v") + appInfo::version;
    
    if (!m_currentFile.isEmpty()) {
        title += QStringLiteral(" - ") + QFileInfo(m_currentFile).fileName();
    } else {
        title += QStringLiteral(" - Untitled");
    }
    
    if (m_modified) {
        title += QStringLiteral(" *");
    }
    
    setWindowTitle(title);
}

void MainWindow::onSearch(const QString& text) {
    applyFilterToTree(text);
}

void MainWindow::onNewTemplate() {
    m_editMode = true;
    m_prefixEdit->clear();
    m_bodyEdit->clear();
    m_descriptionEdit->clear();
    m_sourceEdit->setText(QStringLiteral("cppsnippet-made"));  // Source tag for new templates
    m_prefixEdit->setFocus();
    m_templateTree->clearSelection();
    m_prefixEdit->setReadOnly(false);
    m_descriptionEdit->setReadOnly(false);
    updateTemplateButtons();
}

void MainWindow::onDeleteTemplate() {
    if (m_selectedItem.path().isEmpty()) {
        return;
    }
    
    // Check if template is read-only (Installation or Machine tier)
    if (m_selectedItem.tier() != resourceInventory::ResourceTier::User) {
        QString tierName = (m_selectedItem.tier() == resourceInventory::ResourceTier::Installation) 
            ? tr("Installation") : tr("Machine");
        QMessageBox::warning(this, tr("Cannot Delete"),
            tr("Template '%1' is in the %2 tier and is read-only.\n\n"
               "Only User tier templates can be deleted.").arg(m_selectedItem.name(), tierName));
        return;
    }
    
    if (QMessageBox::question(this, tr("Delete Template"),
            tr("Delete template '%1'?").arg(m_selectedItem.name()),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        // Delete the file
        QFile file(m_selectedItem.path());
        if (file.remove()) {
            statusBar()->showMessage(tr("Deleted template: %1").arg(m_selectedItem.name()));
            // Clear selection and editor
            m_selectedItem = resourceInventory::ResourceTemplate();
            m_prefixEdit->clear();
            m_bodyEdit->clear();
            m_descriptionEdit->clear();
            m_sourceEdit->clear();
            updateTemplateButtons();
            refreshInventory();
        } else {
            QMessageBox::warning(this, tr("Delete Failed"),
                tr("Could not delete file: %1").arg(m_selectedItem.path()));
        }
    }
}

void MainWindow::onCopyTemplate() {
    if (m_selectedItem.path().isEmpty()) {
        return;
    }
    
    m_editMode = true;
    m_prefixEdit->setText(m_selectedItem.name() + tr("-copy"));
    m_sourceEdit->setText(QStringLiteral("cppsnippet-made"));
    m_descriptionEdit->setText(m_selectedItem.description());
    // Body is already loaded from selection
    m_prefixEdit->setFocus();
    m_prefixEdit->setReadOnly(false);
    m_descriptionEdit->setReadOnly(false);
    m_templateTree->clearSelection();
    updateTemplateButtons();
}

void MainWindow::onEditTemplate() {
    // Check if template is read-only (Installation or Machine tier)
    if (m_selectedItem.tier() != resourceInventory::ResourceTier::User) {
        QString tierName = (m_selectedItem.tier() == resourceInventory::ResourceTier::Installation) 
            ? tr("Installation") : tr("Machine");
        QMessageBox::warning(this, tr("Cannot Edit"),
            tr("Template '%1' is in the %2 tier and is read-only.\n\n"
               "Use Copy to create an editable copy in your User folder.").arg(m_selectedItem.name(), tierName));
        return;
    }
    
    m_editMode = true;
    m_prefixEdit->setReadOnly(false);
    m_prefixEdit->setEnabled(true);
    m_descriptionEdit->setReadOnly(false);
    m_descriptionEdit->setEnabled(true);
    m_bodyEdit->setReadOnly(false);
    m_bodyEdit->setEnabled(true);
    updateTemplateButtons();
}

void MainWindow::onSaveTemplate() {
    QString prefix = m_prefixEdit->text().trimmed();
    QString body = m_bodyEdit->toPlainText();
    QString description = m_descriptionEdit->toPlainText().trimmed();
    
    if (prefix.isEmpty() || body.isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
            tr("Name and body are required."));
        return;
    }
    
    // If editing an existing template that is not new/copy
    if (!m_selectedItem.path().isEmpty()) {
        // Check if original is read-only (Installation or Machine tier)
        if (m_selectedItem.tier() != resourceInventory::ResourceTier::User) {
            QString tierName = (m_selectedItem.tier() == resourceInventory::ResourceTier::Installation) 
                ? tr("Installation") : tr("Machine");
            QMessageBox::warning(this, tr("Cannot Save"),
                tr("Template '%1' is in the %2 tier and is read-only.\n\n"
                   "Use Copy to create an editable copy in your User folder.").arg(m_selectedItem.name(), tierName));
            return;
        }
        
        // Save to original location for User tier templates
        if (m_selectedItem.tier() == resourceInventory::ResourceTier::User) {
            if (saveTemplateToPath(m_selectedItem.path(), prefix, body, description)) {
                m_editMode = false;
                setEditorFieldsEnabled(false);
                statusBar()->showMessage(tr("Saved template: %1").arg(prefix));
                updateTemplateButtons();
                return;
            }
        }
    }
    
    // New template or copy - save to user folder
    ResourceTemplate tmpl;
    tmpl.setPrefix(prefix);
    tmpl.setBody(body);
    tmpl.setDescription(description);
    tmpl.setName(prefix);
    tmpl.setFormat(QStringLiteral("text/scad.template"));
    tmpl.setSource(QStringLiteral("cppsnippet-made"));
    
    if (saveTemplateToUser(tmpl)) {
        m_editMode = false;
        setEditorFieldsEnabled(false);
        refreshInventory();
        statusBar()->showMessage(tr("Saved template: %1").arg(prefix));
        updateTemplateButtons();
    }
}

void MainWindow::onCancelEdit() {
    m_editMode = false;
    setEditorFieldsEnabled(false);
    if (!m_selectedItem.path().isEmpty()) {
        populateEditorFromSelection(m_selectedItem);
    } else {
        m_prefixEdit->clear();
        m_bodyEdit->clear();
        m_descriptionEdit->clear();
        m_sourceEdit->clear();
    }
    updateTemplateButtons();
}

void MainWindow::onInventoryItemSelected(const resourceInventory::ResourceTemplate& item) {
    m_selectedItem = item;
    // Show tier name in source field
    QString tierName;
    switch (item.tier()) {
        case resourceInventory::ResourceTier::Installation: tierName = tr("Installation"); break;
        case resourceInventory::ResourceTier::Machine: tierName = tr("Machine"); break;
        case resourceInventory::ResourceTier::User: tierName = tr("User"); break;
    }
    m_sourceEdit->setText(tierName);
    populateEditorFromSelection(item);
}

void MainWindow::onInventorySelectionChanged() {
    QModelIndexList selected = m_templateTree->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }
    
    QModelIndex index = selected.first();
    QVariant itemData = m_inventory->data(index, Qt::UserRole);
    
    resourceInventory::ResourceTemplate tmpl = itemData.value<resourceInventory::ResourceTemplate>();
    if (tmpl.path().isEmpty()) {
        return;
    }
    
    onInventoryItemSelected(tmpl);
}

void MainWindow::populateEditorFromSelection(const resourceInventory::ResourceTemplate& item) {
    m_prefixEdit->setText(item.name());
    
    // Disable editing by default - will be enabled by Edit button
    setEditorFieldsEnabled(false);

    // Prefer structured parse via TemplateParser/JSON to extract body/description/source
    QFile file(item.path());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray data = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject root = doc.object();

            // Legacy format: { "key": "...", "content": "..." }
            if (root.contains("key") && root.contains("content")) {
                QString key = root.value("key").toString();
                QString content = root.value("content").toString();
                // Unescape legacy markers
                content.replace("\\n", "\n");
                content.replace("^~^", "$0");
                m_prefixEdit->setText(key);
                m_bodyEdit->setPlainText(content);
                m_descriptionEdit->setText(item.description());
                m_sourceEdit->setText(QStringLiteral("legacy-converted"));
            } else {
                // Modern VSCode snippet format: { "name": { "prefix": ..., "body": [...], "description": ..., "_source": ... } }
                // Choose the first entry or the one matching item.name()
                QString selectedKey;
                QJsonObject selectedObj;
                for (auto it = root.begin(); it != root.end(); ++it) {
                    if (it.key().startsWith('_')) continue; // skip metadata at root
                    if (!it.value().isObject()) continue;
                    QJsonObject obj = it.value().toObject();
                    QString prefix = obj.value("prefix").toString(it.key());
                    if (prefix == item.name() || selectedKey.isEmpty()) {
                        selectedKey = it.key();
                        selectedObj = obj;
                    }
                }

                if (!selectedObj.isEmpty()) {
                    // Body can be array or string
                    QString bodyText;
                    if (selectedObj.value("body").isArray()) {
                        QJsonArray arr = selectedObj.value("body").toArray();
                        QStringList lines;
                        for (const QJsonValue& v : arr) {
                            lines << v.toString();
                        }
                        bodyText = lines.join('\n');
                    } else {
                        bodyText = selectedObj.value("body").toString();
                    }

                    QString prefix = selectedObj.value("prefix").toString(selectedKey);
                    QString desc = selectedObj.value("description").toString(item.description());
                    QString sourceTag = selectedObj.value("_source").toString(item.source());

                    m_prefixEdit->setText(prefix);
                    m_bodyEdit->setPlainText(bodyText);
                    m_descriptionEdit->setText(desc);
                    m_sourceEdit->setText(sourceTag);
                } else {
                    // Fallback: treat as plain text
                    m_bodyEdit->setPlainText(QString::fromUtf8(data));
                    m_descriptionEdit->setText(item.description());
                    m_sourceEdit->setText(item.source());
                }
            }
        } else {
            // Non-JSON: treat as plain text template
            m_bodyEdit->setPlainText(QString::fromUtf8(data));
            m_descriptionEdit->setText(item.description());
            m_sourceEdit->setText(item.source());
        }
    } else {
        m_bodyEdit->setPlainText("");
        m_descriptionEdit->setText(item.description());
        m_sourceEdit->setText(item.source());
    }

    updateTemplateButtons();
}

QString MainWindow::userTemplatesRoot() const {
    // Use QStandardPaths to get Documents folder, append folder name
    // This matches what ResourcePaths discovers for User tier
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return documentsPath + QStringLiteral("/ScadTemplates");
}

bool MainWindow::saveTemplateToUser(const ResourceTemplate& tmpl) {
    // Ensure user templates directory exists
    QString userRoot = userTemplatesRoot() + "/templates";
    QDir dir;
    if (!dir.mkpath(userRoot)) {
        QMessageBox::warning(this, tr("Save Failed"),
            tr("Could not create user templates directory: %1").arg(userRoot));
        return false;
    }
    
    // Build filename from prefix (sanitize it)
    QString fileName = m_prefixEdit->text().trimmed();
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, tr("Save Failed"),
            tr("Template name (prefix) cannot be empty"));
        return false;
    }
    
    // Sanitize filename - remove invalid characters
    fileName.replace(QRegularExpression("[<>:\"/\\\\|?*]"), "_");
    QString filePath = userRoot + "/" + fileName + ".code-snippets";
    
    // Build JSON in VS Code snippet format
    QJsonObject snippetContent;
    
    // Body as array of lines
    QStringList bodyLines = m_bodyEdit->toPlainText().split('\n');
    QJsonArray bodyArray;
    for (const QString& line : bodyLines) {
        bodyArray.append(line);
    }
    snippetContent["body"] = bodyArray;
    snippetContent["prefix"] = m_prefixEdit->text();
    snippetContent["description"] = m_descriptionEdit->toPlainText();
    snippetContent["_source"] = QStringLiteral("user");
    
    // Wrap in outer object with name as key
    QJsonObject root;
    root[fileName] = snippetContent;
    
    // Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Save Failed"),
            tr("Could not write to file: %1").arg(filePath));
        return false;
    }
    
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    statusBar()->showMessage(tr("Template saved to: %1").arg(filePath), 3000);
    return true;
}

bool MainWindow::saveTemplateToPath(const QString& filePath, const QString& prefix, const QString& body, const QString& description) {
    // Build JSON in VS Code snippet format
    QJsonObject snippetContent;
    
    // Body as array of lines
    QStringList bodyLines = body.split('\n');
    QJsonArray bodyArray;
    for (const QString& line : bodyLines) {
        bodyArray.append(line);
    }
    snippetContent["body"] = bodyArray;
    snippetContent["prefix"] = prefix;
    snippetContent["description"] = description;
    snippetContent["_source"] = QStringLiteral("user");
    
    // Use prefix as key
    QJsonObject root;
    root[prefix] = snippetContent;
    
    // Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Save Failed"),
            tr("Could not write to file: %1").arg(filePath));
        return false;
    }
    
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

void MainWindow::setEditorFieldsEnabled(bool enabled) {
    m_prefixEdit->setReadOnly(!enabled);
    m_prefixEdit->setEnabled(enabled || !m_selectedItem.path().isEmpty()); // Always show content
    m_descriptionEdit->setReadOnly(!enabled);
    m_descriptionEdit->setEnabled(enabled || !m_selectedItem.path().isEmpty());
    m_bodyEdit->setReadOnly(!enabled);
    m_bodyEdit->setEnabled(enabled || !m_selectedItem.path().isEmpty());
    
    // Visual feedback - dim when disabled
    QString style = enabled ? QString() : QStringLiteral("background-color: #f0f0f0;");
    m_prefixEdit->setStyleSheet(style);
    m_descriptionEdit->setStyleSheet(style);
    m_bodyEdit->setStyleSheet(style);
}

void MainWindow::applyFilterToTree(const QString& text) {
    // TODO: Filter tree based on search text
    Q_UNUSED(text);
}

void MainWindow::refreshInventory() {
    // For MVP, just inform user to restart to see changes
    // TODO: Implement live refresh by rescanning and rebuilding model
    statusBar()->showMessage(tr("Template saved. Restart app to see updates in list."), 5000);
}

void MainWindow::updateTemplateButtons() {
    bool hasSelection = !m_selectedItem.path().isEmpty();
    bool isEditing = m_editMode;
    
    m_newBtn->setEnabled(!isEditing);
    m_deleteBtn->setEnabled(!isEditing && hasSelection);
    m_copyBtn->setEnabled(!isEditing && hasSelection);
    m_editBtn->setEnabled(!isEditing && hasSelection);
    m_saveBtn->setEnabled(isEditing);
    m_cancelBtn->setEnabled(isEditing);
    
    // Update tooltips
    m_saveBtn->setToolTip(isEditing ? tr("Save changes") : tr("Save only active in Edit mode"));
    m_cancelBtn->setToolTip(isEditing ? tr("Cancel changes") : tr("Cancel only active in Edit mode"));
}

void MainWindow::onPreferences() {
    PreferencesDialog dialog(this);
    dialog.exec();
}

void MainWindow::onNewFile() {
    if (m_modified) {
        auto result = QMessageBox::question(this, tr("Unsaved Changes"),
            tr("Do you want to save changes before creating a new file?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (result == QMessageBox::Save) {
            onSaveFile();
        } else if (result == QMessageBox::Cancel) {
            return;
        }
    }
    
    m_editor->clear();
    m_currentFile.clear();
    m_modified = false;
    updateWindowTitle();
    statusBar()->showMessage(tr("New file created"));
}

void MainWindow::onOpenFile() {
    if (m_modified) {
        auto result = QMessageBox::question(this, tr("Unsaved Changes"),
            tr("Do you want to save changes before opening another file?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (result == QMessageBox::Save) {
            onSaveFile();
        } else if (result == QMessageBox::Cancel) {
            return;
        }
    }
    
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), QString(),
        tr("OpenSCAD Files (*.scad);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_editor->setPlainText(QString::fromUtf8(file.readAll()));
            m_currentFile = fileName;
            m_modified = false;
            updateWindowTitle();
            statusBar()->showMessage(tr("Opened %1").arg(fileName));
        } else {
            QMessageBox::warning(this, tr("Error"),
                tr("Could not open file: %1").arg(file.errorString()));
        }
    }
}

void MainWindow::onSaveFile() {
    if (m_currentFile.isEmpty()) {
        onSaveFileAs();
        return;
    }
    
    QFile file(m_currentFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(m_editor->toPlainText().toUtf8());
        m_modified = false;
        updateWindowTitle();
        statusBar()->showMessage(tr("Saved %1").arg(m_currentFile));
    } else {
        QMessageBox::warning(this, tr("Error"),
            tr("Could not save file: %1").arg(file.errorString()));
    }
}

void MainWindow::onSaveFileAs() {
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save File"), m_currentFile,
        tr("OpenSCAD Files (*.scad);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        m_currentFile = fileName;
        onSaveFile();
    }
}

bool MainWindow::loadTemplatesFromFile(const QString& filePath) {
    QString errorMsg;
    if (!resourceInventory::loadTemplatesFromFile(m_inventory, filePath, &errorMsg)) {
        // Error message will be shown by caller
        return false;
    }
    return true;
}

bool MainWindow::saveTemplatesToFile(const QString& filePath) const {
    QString errorMsg;
    if (!resourceInventory::saveTemplatesToFile(m_inventory, filePath, &errorMsg)) {
        // Error message will be shown by caller
        return false;
    }
    return true;
}
