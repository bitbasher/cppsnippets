/**
 * @file mainwindow.cpp
 * @brief Main window implementation with text editor and preferences
 */

#include "mainwindow.h"
#include "gui/preferencesdialog.h"
#include <scadtemplates/scadtemplates.h>
#include <scadtemplates/template_parser.h>
#include <platformInfo/resourceLocationManager.h>
#include <resInventory/resourceScanner.h>
#include <resInventory/resourceTreeWidget.h>
#include <resInventory/resourceStore.h>
#include <resInventory/resourceScannerDirListing.h>
#include <resInventory/templateTreeModel.h>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QTreeView>
#include <QHeaderView>
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
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QTextBrowser>
#include <QDialog>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_templateManager(std::make_unique<scadtemplates::TemplateManager>())
    , m_resourceManager(std::make_unique<platformInfo::ResourceLocationManager>())
    , m_inventoryManager(std::make_unique<resInventory::ResourceInventoryManager>())
    , m_settings(std::make_unique<QSettings>(QStringLiteral("OpenSCAD"), QStringLiteral("ScadTemplates")))
    , m_resourceStore(std::make_unique<resInventory::ResourceStore>())
    , m_scanner(std::make_unique<resInventory::ResourceScannerDirListing>())
    , m_scannerLoggingEnabled(true)
{
    // Enable scanner logging for debugging (initially on)
    resInventory::ResourceScannerDirListing::enableLogging(m_scannerLoggingEnabled);
    
    // Set application path for resource manager
    m_resourceManager->setApplicationPath(QCoreApplication::applicationDirPath());
    
    // Build resource inventory from all known locations
    m_inventoryManager->buildInventory(*m_resourceManager);
    
    setupUi();
    setupMenus();
    
    // Perform initial template scan
    refreshInventory();
    
    updateWindowTitle();
    resize(1200, 800);
    
    // Restore window geometry
    restoreGeometry(m_settings->value(QStringLiteral("MainWindow/geometry")).toByteArray());
    restoreState(m_settings->value(QStringLiteral("MainWindow/state")).toByteArray());
    
    statusBar()->showMessage(tr("Ready"));

    // Ensure initial button states after setup and scanning
    updateTemplateButtons();
    m_newBtn->setEnabled(true);
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
    
    // Create a splitter for resizable panels
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    // Left panel - template browser
    QWidget* leftPanel = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* listGroup = new QGroupBox(tr("Templates"), this);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName("m_searchEdit");
    m_searchEdit->setPlaceholderText(tr("Search templates..."));
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearch);
    listLayout->addWidget(m_searchEdit);
    
    // Create QTreeView with TemplateTreeModel
    m_templateTree = new QTreeView(this);
    m_templateTree->setObjectName("m_templateTree");
    m_templateModel = new resInventory::TemplateTreeModel(this);
    m_templateModel->setResourceStore(m_resourceStore.get());
    m_templateTree->setModel(m_templateModel);
    m_templateTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_templateTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_templateTree->setAlternatingRowColors(true);
    m_templateTree->setUniformRowHeights(true);
    m_templateTree->setAnimated(true);
    m_templateTree->header()->setStretchLastSection(false);
        m_templateTree->header()->setSectionResizeMode(0, QHeaderView::Interactive); // Name
        m_templateTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Category
        m_templateTree->header()->setSectionResizeMode(2, QHeaderView::Stretch); // Description
        // Hide Category for now (templates don't use it yet)
        m_templateTree->setColumnHidden(1, true);
    connect(m_templateTree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onInventorySelectionChanged);
    listLayout->addWidget(m_templateTree);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_newBtn = new QPushButton(tr("New"), this);
    m_newBtn->setObjectName("m_newBtn");
    m_deleteBtn = new QPushButton(tr("Delete"), this);
    m_deleteBtn->setObjectName("m_deleteBtn");
    m_copyBtn = new QPushButton(tr("Copy"), this);
    m_copyBtn->setObjectName("m_copyBtn");
    m_editBtn = new QPushButton(tr("Edit"), this);
    m_editBtn->setObjectName("m_editBtn");
    m_saveBtn = new QPushButton(tr("Save"), this);
    m_saveBtn->setObjectName("m_saveBtn");
    m_cancelBtn = new QPushButton(tr("Cancel"), this);
    m_cancelBtn->setObjectName("m_cancelBtn");
    
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
    
    // Initialize button visibility
    updateTemplateButtons();
    
    leftLayout->addWidget(listGroup);
    
    // Template editor section
    QGroupBox* editorGroup = new QGroupBox(tr("Template Editor"), this);
    QVBoxLayout* editorLayout = new QVBoxLayout(editorGroup);
    
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel(tr("Name:"), this));
    m_prefixEdit = new QLineEdit(this);
    m_prefixEdit->setObjectName("m_prefixEdit");
    m_prefixEdit->setReadOnly(true);  // Start as read-only for viewing
    nameLayout->addWidget(m_prefixEdit);
    connect(m_prefixEdit, &QLineEdit::textChanged, this, &MainWindow::onTemplateFieldChanged);
    editorLayout->addLayout(nameLayout);
    
    QHBoxLayout* sourceLayout = new QHBoxLayout();
    sourceLayout->addWidget(new QLabel(tr("Source:"), this));
    m_sourceEdit = new QLineEdit(this);
    m_sourceEdit->setObjectName("m_sourceEdit");
    m_sourceEdit->setReadOnly(true);
    sourceLayout->addWidget(m_sourceEdit);
    sourceLayout->addSpacing(12);
    sourceLayout->addWidget(new QLabel(tr("Version:"), this));
    m_versionEdit = new QLineEdit(this);
    m_versionEdit->setObjectName("m_versionEdit");
    m_versionEdit->setReadOnly(true);
    sourceLayout->addWidget(m_versionEdit);
    editorLayout->addLayout(sourceLayout);
    
    editorLayout->addWidget(new QLabel(tr("Description:"), this));
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setObjectName("m_descriptionEdit");
    m_descriptionEdit->setMaximumHeight(80);
    m_descriptionEdit->setTabStopDistance(20);  // 2 characters at fixed font width
    m_descriptionEdit->setReadOnly(true);  // Start as read-only for viewing
    connect(m_descriptionEdit, &QTextEdit::textChanged, this, &MainWindow::onTemplateFieldChanged);
    editorLayout->addWidget(m_descriptionEdit);
    
    editorLayout->addWidget(new QLabel(tr("Body:"), this), 0, Qt::AlignTop);
    m_bodyEdit = new QTextEdit(this);
    m_bodyEdit->setObjectName("m_bodyEdit");
    m_bodyEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_bodyEdit->setMaximumHeight(150);
    m_bodyEdit->setTabStopDistance(20);  // 2 characters at fixed font width
    m_bodyEdit->setReadOnly(true);  // Start as read-only for viewing
    connect(m_bodyEdit, &QTextEdit::textChanged, this, &MainWindow::onTemplateFieldChanged);
    editorLayout->addWidget(m_bodyEdit);
    
    QHBoxLayout* editorButtonLayout = new QHBoxLayout();
    editorButtonLayout->addWidget(m_saveBtn);
    editorButtonLayout->addWidget(m_cancelBtn);
    editorLayout->addLayout(editorButtonLayout);
    
    leftLayout->addWidget(editorGroup);
    
    // Enable drag and drop for template files
    leftPanel->setAcceptDrops(true);
    leftPanel->installEventFilter(this);
    
    splitter->addWidget(leftPanel);
    
    // Right panel - main text editor
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
    
    splitter->addWidget(rightPanel);
    
    // Set initial sizes (1:2 ratio)
    splitter->setSizes({400, 800});
    
    mainLayout->addWidget(splitter);
}

void MainWindow::setupMenus() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    
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
    
    // Edit menu
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    
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
    
    // Templates menu
    QMenu* templatesMenu = menuBar()->addMenu(tr("&Templates"));
    
    QAction* loadTemplatesAction = templatesMenu->addAction(tr("&Load Templates..."));
    connect(loadTemplatesAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Templates File"), QString(),
            tr("JSON Files (*.json);;All Files (*)"));
        if (!fileName.isEmpty()) {
            if (m_templateManager->loadFromFile(fileName.toStdString())) {
                refreshInventory();
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
            if (m_templateManager->saveToFile(fileName.toStdString())) {
                statusBar()->showMessage(tr("Saved templates to %1").arg(fileName));
            } else {
                QMessageBox::warning(this, tr("Error"),
                    tr("Failed to save templates to %1").arg(fileName));
            }
        }
    });
    
    // Diagnostics menu
    QMenu* diagnosticsMenu = menuBar()->addMenu(tr("&Diagnostics"));
    
    QAction* loggingAction = diagnosticsMenu->addAction(tr("Toggle Scanner &Logging"));
    loggingAction->setCheckable(true);
    loggingAction->setChecked(m_scannerLoggingEnabled);
    connect(loggingAction, &QAction::triggered, this, &MainWindow::onToggleScannerLogging);
    
    QAction* siblingAction = diagnosticsMenu->addAction(tr("Toggle &Sibling Installations"));
    siblingAction->setCheckable(true);
    siblingAction->setChecked(true);
    connect(siblingAction, &QAction::triggered, this, &MainWindow::onToggleSiblingInstallations);
    
    diagnosticsMenu->addSeparator();
    
    // Help menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    
    QAction* aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QString resourceDir = m_resourceManager->findInstallationResourceDir();
        if (resourceDir.isEmpty()) {
            resourceDir = tr("(not found)");
        }
        
        QString resourceCounts = m_inventoryManager->countSummary();
        
        QMessageBox::about(this, tr("About ScadTemplates"),
            tr("ScadTemplates v%1\n\n"
               "A code template and resource management tool for OpenSCAD.\n\n"
               "Platform: %2\n"
               "Resource Directory: %3\n\n"
               "Resources Found:\n%4\n\n"
               "Copyright (c) 2025\n"
               "MIT License")
            .arg(scadtemplates::getVersion())
            .arg(m_resourceManager->folderName())
            .arg(resourceDir)
            .arg(resourceCounts));
    });
}

void MainWindow::updateWindowTitle() {
    QString title = QStringLiteral("ScadTemplates v") + scadtemplates::getVersion();
    
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
    m_versionEdit->setText(QStringLiteral("0"));  // New templates start at version 0
    m_prefixEdit->setFocus();
    m_templateTree->clearSelection();
    m_prefixEdit->setReadOnly(false);
    m_descriptionEdit->setReadOnly(false);
    m_bodyEdit->setReadOnly(false);
    
    // Set save destination to first available user location
    auto userLocs = m_resourceManager->availableUserLocations();
    if (!userLocs.isEmpty() && userLocs[0].exists) {
        m_saveDestinationPath = userLocs[0].path;
    }
    
    updateTemplateButtons();
}

void MainWindow::onDeleteTemplate() {
    if (m_selectedItem.path().isEmpty()) {
        return;
    }
    
    if (QMessageBox::question(this, tr("Delete Template"),
            tr("Delete template '%1'?").arg(m_selectedItem.name()),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        // Delete the template file
        QFile file(m_selectedItem.path());
        if (!file.remove()) {
            QMessageBox::warning(this, tr("Error"),
                tr("Failed to delete template file: %1").arg(m_selectedItem.path()));
            return;
        }
        
        // Clear editor and selection
        m_prefixEdit->clear();
        m_bodyEdit->clear();
        m_descriptionEdit->clear();
        m_sourceEdit->clear();
        m_versionEdit->clear();
        m_selectedItem = resInventory::ResourceItem();
        m_templateTree->clearSelection();
        
        // Refresh inventory to update tree view
        refreshInventory();
        
        statusBar()->showMessage(tr("Deleted template: %1").arg(m_selectedItem.name()));
        updateTemplateButtons();
    }
}

void MainWindow::onCopyTemplate() {
    if (m_selectedItem.path().isEmpty()) {
        return;
    }
    
    // Save the body and description before entering edit mode
    QString bodyText = m_bodyEdit->toPlainText();
    QString descText = m_descriptionEdit->toPlainText();
    
    m_editMode = true;
    // Base the copy name on the current prefix field (fallback to selected item name)
    QString baseName = m_prefixEdit->text().trimmed();
    if (baseName.isEmpty()) {
        baseName = m_selectedItem.name();
    }
    m_prefixEdit->setText(baseName + tr("-copy"));
    m_bodyEdit->setPlainText(bodyText);
    m_descriptionEdit->setPlainText(descText);
    m_sourceEdit->setText(QStringLiteral("cppsnippet-made"));
    m_versionEdit->setText(QStringLiteral("0"));
    m_prefixEdit->setFocus();
    m_prefixEdit->setReadOnly(false);
    m_descriptionEdit->setReadOnly(false);
    m_bodyEdit->setReadOnly(false);
    m_templateTree->clearSelection();
    
    // Set save destination to first available user location
    auto userLocs = m_resourceManager->availableUserLocations();
    if (!userLocs.isEmpty() && userLocs[0].exists) {
        // Prefer the templates subfolder within the user location
        QDir locDir(userLocs[0].path);
        m_saveDestinationPath = locDir.filePath(QStringLiteral("templates"));
    }
    
    updateTemplateButtons();
}

void MainWindow::onEditTemplate() {
    m_editMode = true;
    m_prefixEdit->setReadOnly(false);
    m_descriptionEdit->setReadOnly(false);
    m_bodyEdit->setReadOnly(false);
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
    
    // Increment version
    QString currentVersion = m_versionEdit->text();
    QString newVersion = incrementVersion(currentVersion);
    m_versionEdit->setText(newVersion);
    
    scadtemplates::Template tmpl(prefix.toStdString(), 
                                  body.toStdString(),
                                  description.toStdString());
    
    if (saveTemplateToUser(tmpl, newVersion)) {
        m_editMode = false;
        refreshInventory();
        statusBar()->showMessage(tr("Saved template: %1 (version %2)").arg(prefix).arg(newVersion));
        updateTemplateButtons();
    }
}

void MainWindow::onCancelEdit() {
    m_editMode = false;
    m_prefixEdit->setReadOnly(true);
    m_descriptionEdit->setReadOnly(true);
    m_bodyEdit->setReadOnly(true);
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

void MainWindow::onTemplateFieldChanged() {
    // When in edit mode, enable Save button on any field changes
    if (m_editMode) {
        // Re-evaluate buttons based on current selection and edit state
        updateTemplateButtons();
    }
}

void MainWindow::onInventoryItemSelected(const resInventory::ResourceItem& item) {
    m_selectedItem = item;
    // Show tier name in source field
    QString tierName;
    switch (item.tier()) {
        case resInventory::ResourceTier::Installation: tierName = tr("Installation"); break;
        case resInventory::ResourceTier::Machine: tierName = tr("Machine"); break;
        case resInventory::ResourceTier::User: tierName = tr("User"); break;
    }
    m_sourceEdit->setText(tierName);
        m_versionEdit->clear();
    populateEditorFromSelection(item);
}

void MainWindow::onInventorySelectionChanged() {
    QModelIndexList indexes = m_templateTree->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        // Don't clear fields if we're in edit mode (New/Copy/Edit in progress)
        if (!m_editMode) {
            m_prefixEdit->clear();
            m_bodyEdit->clear();
            m_descriptionEdit->clear();
            m_sourceEdit->clear();
        }
        m_selectedItem = resInventory::ResourceItem();
        updateTemplateButtons();
        return;
    }
    
    QModelIndex index = indexes.first();
    
    // Only handle template leaf nodes
    auto nodeType = m_templateModel->data(index, resInventory::TemplateTreeModel::NodeTypeRole).toInt();
    if (nodeType != static_cast<int>(resInventory::TemplateTreeNode::NodeType::Template)) {
        // Clear selection state for non-template nodes (folders/locations)
        m_selectedItem = resInventory::ResourceItem();
        updateTemplateButtons();
        return;
    }
    
    // Get the full DiscoveredResource
    QVariant resourceVar = m_templateModel->data(index, resInventory::TemplateTreeModel::ResourceRole);
    if (!resourceVar.isValid()) {
        return;
    }
    
    auto discoveredRes = resourceVar.value<resInventory::DiscoveredResource>();
    
    // Convert to ResourceItem for compatibility with existing code
    resInventory::ResourceItem item;
    item.setPath(discoveredRes.path);
    item.setDisplayName(discoveredRes.name);
    item.setType(discoveredRes.type);
    item.setTier(discoveredRes.tier);
    item.setCategory(discoveredRes.category);
    item.setEnabled(true);
    
    m_selectedItem = item;
    onInventoryItemSelected(item);
    updateTemplateButtons();
}

void MainWindow::refreshInventory() {
    // Clear the resource store
    m_resourceStore->clear();
    
    // Scan installation tier
    QString installPath = m_resourceManager->effectiveInstallationPath();
    if (!installPath.isEmpty()) {
        m_resourceStore->scanTypeAndStore(*m_scanner, installPath,
                                          resInventory::ResourceType::Template,
                                          resInventory::ResourceTier::Installation,
                                          installPath);
    }
    
    // Scan sibling installations
    auto siblingLocs = m_resourceManager->findSiblingInstallations();
    for (const auto& loc : siblingLocs) {
        if (loc.exists && loc.isEnabled) {
            m_resourceStore->scanTypeAndStore(*m_scanner, loc.path,
                                              resInventory::ResourceType::Template,
                                              resInventory::ResourceTier::Installation,
                                              loc.path);
        }
    }
    
    // Scan machine tier
    auto machineLocs = m_resourceManager->availableMachineLocations();
    for (const auto& loc : machineLocs) {
        if (loc.exists && loc.isEnabled) {
            m_resourceStore->scanTypeAndStore(*m_scanner, loc.path,
                                              resInventory::ResourceType::Template,
                                              resInventory::ResourceTier::Machine,
                                              loc.path);
        }
    }
    
    // Scan user tier
    auto userLocs = m_resourceManager->availableUserLocations();
    for (const auto& loc : userLocs) {
        if (loc.exists && loc.isEnabled) {
            m_resourceStore->scanTypeAndStore(*m_scanner, loc.path,
                                              resInventory::ResourceType::Template,
                                              resInventory::ResourceTier::User,
                                              loc.path);
        }
    }
    
    // Model automatically rebuilds via signals
    m_templateTree->expandAll();
}

void MainWindow::populateEditorFromSelection(const resInventory::ResourceItem& item) {
    // Clear all fields first to ensure clean state
    m_prefixEdit->clear();
    m_bodyEdit->clear();
    m_descriptionEdit->clear();
    m_sourceEdit->clear();
    m_versionEdit->clear();
    
    m_prefixEdit->setText(item.name());

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
                m_versionEdit->setText(QStringLiteral("0"));
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
                    QString sourceTag = selectedObj.value("_source").toString(item.sourceLocationKey());

                    m_prefixEdit->setText(prefix);
                    m_bodyEdit->setPlainText(bodyText);
                    m_descriptionEdit->setText(desc);
                    m_sourceEdit->setText(sourceTag);
                    // Version (simple integer, default to 0 if missing)
                    const auto verVal = selectedObj.value(QStringLiteral("version"));
                    QString version = QStringLiteral("0");
                    if (verVal.isDouble()) {
                        version = QString::number(static_cast<int>(verVal.toDouble()));
                    } else if (verVal.isString()) {
                        version = verVal.toString();
                    }
                    m_versionEdit->setText(version);
                } else {
                    // Fallback: treat as plain text
                    m_bodyEdit->setPlainText(QString::fromUtf8(data));
                    m_descriptionEdit->setText(item.description());
                    m_sourceEdit->setText(item.sourceLocationKey());
                    m_versionEdit->setText(QStringLiteral("0"));
                }
            }
        } else {
            // Non-JSON: treat as plain text template
            m_bodyEdit->setPlainText(QString::fromUtf8(data));
            m_descriptionEdit->setText(item.description());
            m_sourceEdit->setText(item.sourceLocationKey());
            m_versionEdit->setText(QStringLiteral("0"));
        }
    } else {
        m_bodyEdit->setPlainText("");
        m_descriptionEdit->setText(item.description());
        m_sourceEdit->setText(item.sourceLocationKey());
        m_versionEdit->setText(QStringLiteral("0"));
    }

    // Ensure fields reflect editability: read-only when not in Edit mode
    const bool editable = m_editMode;
    m_prefixEdit->setReadOnly(!editable);
    m_descriptionEdit->setReadOnly(!editable);
    m_bodyEdit->setReadOnly(!editable);

    updateTemplateButtons();
}

QString MainWindow::incrementVersion(const QString& version) const {
    // Version is a simple integer (0, 1, 2, etc.)
    bool ok;
    int currentVersion = version.toInt(&ok);
    if (!ok) {
        // If not a valid integer, default to 0
        currentVersion = 0;
    }
    return QString::number(currentVersion + 1);
}

QString MainWindow::userTemplatesRoot() const {
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + 
           QStringLiteral("/ScadTemplates");
}

bool MainWindow::saveTemplateToUser(const scadtemplates::Template& tmpl, const QString& version) {
    // Decide whether to overwrite an existing writable template or create a new file in the User templates folder
    QString filePath;
    
    // Check current selection context
    auto nodeType = m_templateModel->data(m_templateTree->currentIndex(), resInventory::TemplateTreeModel::NodeTypeRole).toInt();
    bool isTemplateNode = nodeType == static_cast<int>(resInventory::TemplateTreeNode::NodeType::Template);
    bool canOverwrite = isTemplateNode && !m_selectedItem.path().isEmpty() &&
                        (m_selectedItem.tier() == resInventory::ResourceTier::User);
    
    if (canOverwrite) {
        // Overwrite the existing user template file
        filePath = m_selectedItem.path();
    } else {
        // Determine a writable User templates folder
        QString targetDir = m_saveDestinationPath;
        if (targetDir.isEmpty()) {
            targetDir = findNewResourcesTemplatesFolder();
        }
        if (targetDir.isEmpty()) {
            QMessageBox::warning(this, tr("Error"),
                tr("No writable User templates folder available."));
            return false;
        }
        
        if (!QDir(targetDir).exists()) {
            if (!QDir().mkpath(targetDir)) {
                QMessageBox::warning(this, tr("Error"),
                    tr("Failed to create user templates directory: %1").arg(targetDir));
                return false;
            }
        }
        
        // Compose file name from prefix and ensure uniqueness
        QString fileName = QString::fromStdString(tmpl.getPrefix()) + ".json";
        filePath = generateUniqueFileName(targetDir, fileName);
    }
    
    // Create JSON structure
    QJsonObject snippetObj;
    
    // Body as array of lines
    QStringList bodyLines = QString::fromStdString(tmpl.getBody()).split('\n');
    QJsonArray bodyArray;
    for (const QString& line : bodyLines) {
        bodyArray.append(line);
    }
    
    snippetObj["prefix"] = QString::fromStdString(tmpl.getPrefix());
    snippetObj["body"] = bodyArray;
    snippetObj["description"] = QString::fromStdString(tmpl.getDescription());
    snippetObj["version"] = version;
    snippetObj["_source"] = m_sourceEdit->text();
    
    // Wrap in named object
    QJsonObject root;
    root[QString::fromStdString(tmpl.getPrefix())] = snippetObj;
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
            tr("Failed to save template file: %1").arg(filePath));
        return false;
    }
    
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    // Clear save destination after successful save
    m_saveDestinationPath.clear();
    
    return true;
}

void MainWindow::applyFilterToTree(const QString& text) {
    if (text.isEmpty()) {
        // Clear search - show all and clear selection
        m_templateTree->collapseAll();
        m_templateTree->expandAll();
        m_templateTree->clearSelection();
        return;
    }
    
    // Expand all to search through all items
    m_templateTree->expandAll();
    
    // Get current selection
    QModelIndex currentSelection;
    QModelIndexList selectedIndexes = m_templateTree->selectionModel()->selectedRows();
    if (!selectedIndexes.isEmpty()) {
        currentSelection = selectedIndexes.first();
    }
    
    // Function to search from a given starting point
    auto searchFromIndex = [this, &text](const QModelIndex& startIndex, bool wrapAround) -> QModelIndex {
        // Collect all template indices in order
        QVector<QModelIndex> templateIndices;
        
        std::function<void(const QModelIndex&)> collectTemplates;
        collectTemplates = [this, &collectTemplates, &templateIndices](const QModelIndex& parent) {
            for (int row = 0; row < m_templateModel->rowCount(parent); ++row) {
                QModelIndex idx = m_templateModel->index(row, 0, parent);
                auto nodeType = m_templateModel->data(idx, resInventory::TemplateTreeModel::NodeTypeRole).toInt();
                
                if (nodeType == static_cast<int>(resInventory::TemplateTreeNode::NodeType::Template)) {
                    templateIndices.append(idx);
                } else {
                    collectTemplates(idx);
                }
            }
        };
        
        collectTemplates(QModelIndex());
        
        // Find starting position
        int startPos = 0;
        if (startIndex.isValid()) {
            for (int i = 0; i < templateIndices.size(); ++i) {
                if (templateIndices[i] == startIndex) {
                    startPos = i + 1;  // Start from next item
                    break;
                }
            }
        }
        
        // Search from start position to end
        for (int i = startPos; i < templateIndices.size(); ++i) {
            QString name = m_templateModel->data(templateIndices[i], Qt::DisplayRole).toString();
            if (name.contains(text, Qt::CaseInsensitive)) {
                return templateIndices[i];
            }
        }
        
        // If wrapping, search from beginning to start position
        if (wrapAround && startIndex.isValid()) {
            for (int i = 0; i < startPos - 1; ++i) {
                QString name = m_templateModel->data(templateIndices[i], Qt::DisplayRole).toString();
                if (name.contains(text, Qt::CaseInsensitive)) {
                    return templateIndices[i];
                }
            }
        }
        
        return QModelIndex();
    };
    
    // Search starting from current selection, with wrap-around
    QModelIndex found = searchFromIndex(currentSelection, true);
    
    if (found.isValid()) {
        m_templateTree->setCurrentIndex(found);
        m_templateTree->scrollTo(found, QAbstractItemView::EnsureVisible);
    } else if (currentSelection.isValid()) {
        // No match found - keep current selection or clear it
        m_templateTree->clearSelection();
        m_templateTree->scrollTo(m_templateModel->index(0, 0, QModelIndex()), QAbstractItemView::PositionAtTop);
    } else {
        // No selection to begin with, no match found - show top and clear selection
        m_templateTree->clearSelection();
        m_templateTree->scrollTo(m_templateModel->index(0, 0, QModelIndex()), QAbstractItemView::PositionAtTop);
    }
}

void MainWindow::updateTemplateButtons() {
    bool hasSelection = !m_selectedItem.path().isEmpty();
    bool isEditing = m_editMode;
    
    // Determine if selected item is a template (not a location/folder)
    auto nodeType = m_templateModel->data(m_templateTree->currentIndex(), resInventory::TemplateTreeModel::NodeTypeRole).toInt();
    bool isTemplate = nodeType == static_cast<int>(resInventory::TemplateTreeNode::NodeType::Template);
    
    // Determine if selected template is writable (in User tier)
    bool isWritableTemplate = false;
    if (hasSelection && isTemplate) {
        auto tier = m_templateModel->data(m_templateTree->currentIndex(), resInventory::TemplateTreeModel::TierRole).toInt();
        if (tier == static_cast<int>(resInventory::ResourceTier::User)) {
            isWritableTemplate = true;
        }
    }
    
    // Determine if a user tier location is selected (allows New and Copy to set destination)
    bool isUserTierLocation = false;
    if (!hasSelection) {
        // Check if it's a location node
        auto nodeType2 = m_templateModel->data(m_templateTree->currentIndex(), resInventory::TemplateTreeModel::NodeTypeRole).toInt();
        auto tier2 = m_templateModel->data(m_templateTree->currentIndex(), resInventory::TemplateTreeModel::TierRole).toInt();
        if (nodeType2 == static_cast<int>(resInventory::TemplateTreeNode::NodeType::Location) &&
            tier2 == static_cast<int>(resInventory::ResourceTier::User)) {
            isUserTierLocation = true;
        }
    }
    
    // New: enabled whenever not editing (allow creating templates anytime)
    m_newBtn->setEnabled(!isEditing);
    
    // Copy: can copy from anywhere (not just writable), disabled while editing and when location selected
    m_copyBtn->setEnabled(!isEditing && hasSelection && isTemplate);
    
    // Edit: only for writable templates, disabled while editing
    m_editBtn->setEnabled(!isEditing && isWritableTemplate);
    
    // Delete: only for writable templates, disabled for locations
    m_deleteBtn->setEnabled(!isEditing && isWritableTemplate);
    
    // Save enabled whenever editing (destination handling occurs on save)
    m_saveBtn->setEnabled(isEditing);
    m_cancelBtn->setEnabled(isEditing);
    
    // Update tooltips
    m_saveBtn->setToolTip(isEditing ? tr("Save changes") : tr("Save only active in Edit mode"));
    m_cancelBtn->setToolTip(isEditing ? tr("Cancel changes") : tr("Cancel only active in Edit mode"));
}

void MainWindow::onPreferences() {
    PreferencesDialog dialog(m_resourceManager.get(), this);
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

// ============================================================================
// Drag and Drop Implementation
// ============================================================================

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    // Handle drag/drop events for leftPanel
    if (event->type() == QEvent::DragEnter) {
        QDragEnterEvent* dragEvent = static_cast<QDragEnterEvent*>(event);
        if (dragEvent->mimeData()->hasUrls()) {
            dragEvent->acceptProposedAction();
            return true;
        }
    } else if (event->type() == QEvent::Drop) {
        QDropEvent* dropEvent = static_cast<QDropEvent*>(event);
        const QMimeData* mimeData = dropEvent->mimeData();
        
        if (mimeData->hasUrls()) {
            QList<QUrl> urls = mimeData->urls();
            if (!urls.isEmpty()) {
                dropEvent->acceptProposedAction();
                processDroppedTemplates(urls);
                return true;
            }
        }
    }
    
    return QMainWindow::eventFilter(watched, event);
}

QString MainWindow::findNewResourcesTemplatesFolder() const {
    // Get all user locations from the resource manager
    auto userLocs = m_resourceManager->availableUserLocations();
    
    // Look for a writable user location with a templates folder
    for (const auto& loc : userLocs) {
        if (loc.exists && loc.isWritable) {
            QDir templatesDir(QDir(loc.path).filePath("templates"));
            if (templatesDir.exists()) {
                return templatesDir.absolutePath();
            }
        }
    }
    
    // No writable templates folder found
    return QString();
}

bool MainWindow::validateTemplateFile(const QString& filePath, QString& errorMsg) const {
    QFileInfo fileInfo(filePath);
    
    // Phase 1: Extension check
    if (fileInfo.suffix().toLower() != "json") {
        errorMsg = tr("Not a JSON file");
        return false;
    }
    
    // Phase 2: JSON parsing
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMsg = tr("Cannot open file: %1").arg(file.errorString());
        return false;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (doc.isNull()) {
        errorMsg = tr("Invalid JSON: %1").arg(parseError.errorString());
        return false;
    }
    
    // Phase 3: Template schema validation using TemplateParser
    scadtemplates::TemplateParser parser;
    scadtemplates::ParseResult result = parser.parseJson(jsonData.toStdString());
    
    if (!result.success) {
        errorMsg = tr("Invalid template format: %1").arg(QString::fromStdString(result.errorMessage));
        return false;
    }
    
    if (result.templates.empty()) {
        errorMsg = tr("No valid templates found in file");
        return false;
    }
    
    return true;
}

QString MainWindow::generateUniqueFileName(const QString& targetDir, const QString& baseName) const {
    QFileInfo baseInfo(baseName);
    QString name = baseInfo.completeBaseName();
    QString ext = baseInfo.suffix();
    
    QString targetPath = QDir(targetDir).filePath(baseName);
    if (!QFile::exists(targetPath)) {
        return targetPath;
    }
    
    // Generate numeric suffix
    int counter = 1;
    QString newName;
    do {
        newName = QString("%1_%2.%3").arg(name).arg(counter).arg(ext);
        targetPath = QDir(targetDir).filePath(newName);
        counter++;
    } while (QFile::exists(targetPath));
    
    return targetPath;
}

void MainWindow::processDroppedTemplates(const QList<QUrl>& urls) {
    // Find target folder: newresources/templates within first available user location
    QString targetFolder;
    auto userLocs = m_resourceManager->availableUserLocations();
    for (const auto& loc : userLocs) {
        if (loc.exists && loc.isWritable) {
            targetFolder = QDir(loc.path).filePath("newresources/templates");
            break;
        }
    }
    
    // Check if newresources/templates exists (user must create it manually)
    if (!QDir(targetFolder).exists()) {
        QMessageBox::warning(this, tr("Cannot Accept Templates"),
            tr("The 'newresources/templates' folder does not exist.\n\n"
               "Please create it manually in your User location:\n%1\n\n"
               "Then try dropping the files again.").arg(targetFolder));
        return;
    }
    
    QStringList acceptedFiles;
    QStringList rejectedFiles;
    
    // Validate all files first
    for (const QUrl& url : urls) {
        if (!url.isLocalFile()) {
            rejectedFiles.append(QString("%1: Not a local file").arg(url.toString()));
            continue;
        }
        
        QString filePath = url.toLocalFile();
        QFileInfo fileInfo(filePath);
        
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            rejectedFiles.append(QString("%1: Not a valid file").arg(fileInfo.fileName()));
            continue;
        }
        
        QString errorMsg;
        if (!validateTemplateFile(filePath, errorMsg)) {
            rejectedFiles.append(QString("%1: %2").arg(fileInfo.fileName()).arg(errorMsg));
            continue;
        }
        
        // Validation passed - copy file
        QString targetPath = generateUniqueFileName(targetFolder, fileInfo.fileName());
        
        if (QFile::copy(filePath, targetPath)) {
            acceptedFiles.append(fileInfo.fileName());
        } else {
            rejectedFiles.append(QString("%1: Failed to copy file").arg(fileInfo.fileName()));
        }
    }
    
    // Show results
    showDropResults(acceptedFiles, rejectedFiles);
    
    // Refresh inventory if any files were accepted
    if (!acceptedFiles.isEmpty()) {
        refreshInventory();
        statusBar()->showMessage(tr("Imported %n template file(s)", "", acceptedFiles.count()), 5000);
    }
}

void MainWindow::showDropResults(const QStringList& accepted, const QStringList& rejected) {
    QString message;
    
    if (!accepted.isEmpty()) {
        message += tr("<b>Successfully imported:</b><ul>");
        for (const QString& file : accepted) {
            message += QString("<li>%1</li>").arg(file.toHtmlEscaped());
        }
        message += "</ul>";
    }
    
    if (!rejected.isEmpty()) {
        if (!message.isEmpty()) {
            message += "<br>";
        }
        message += tr("<b>Rejected:</b><ul>");
        for (const QString& file : rejected) {
            message += QString("<li>%1</li>").arg(file.toHtmlEscaped());
        }
        message += "</ul>";
    }
    
    if (message.isEmpty()) {
        message = tr("No files to process");
    }
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Template Import Results"));
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(message);
    msgBox.setIcon(accepted.isEmpty() ? QMessageBox::Warning : QMessageBox::Information);
    msgBox.exec();
}

void MainWindow::onToggleScannerLogging() {
    m_scannerLoggingEnabled = !m_scannerLoggingEnabled;
    resInventory::ResourceScannerDirListing::enableLogging(m_scannerLoggingEnabled);
    
    QString status = m_scannerLoggingEnabled ? tr("enabled") : tr("disabled");
    statusBar()->showMessage(tr("Scanner logging %1 ").arg(status));
}

void MainWindow::onToggleSiblingInstallations() {
    // Re-scan inventory with sibling setting now auto-enabled
    refreshInventory();
    statusBar()->showMessage(tr("Sibling installations are now included in scan"), 3000);
}

