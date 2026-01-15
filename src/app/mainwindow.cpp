/**
 * @file mainwindow.cpp
 * @brief Main window implementation with text editor and preferences
 */

#include "mainwindow.h"
#include "gui/preferencesdialog.hpp"
#include "applicationNameInfo.hpp"
#include <scadtemplates/template_manager.hpp>
#include <platformInfo/ResourceLocation.hpp>
#include <resourceScanning/ResourceScanner.hpp>
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
#include <QStandardItemModel>
#include <QTreeView>
#include <QSortFilterProxyModel>

MainWindow::MainWindow(QStandardItemModel* inventory, QWidget *parent)
    : QMainWindow(parent)
    , m_templateManager(std::make_unique<scadtemplates::TemplateManager>())
    , m_resourceManager(std::make_unique<platformInfo::ResourceLocationManager>())
    , m_settings(std::make_unique<QSettings>(QStringLiteral("OpenSCAD"), QStringLiteral("ScadTemplates")))
    , m_inventory(inventory)
{
    // Set application path for resource manager
    m_resourceManager->setApplicationPath(QCoreApplication::applicationDirPath());
    
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
    
    // Create a splitter for resizable panels
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    // Left panel - template browser
    QWidget* leftPanel = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
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
    m_templateTree->setRootIsDecorated(false);
    m_templateTree->setSortingEnabled(true);
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
    
    // Initialize button visibility
    updateTemplateButtons();
    
    leftLayout->addWidget(listGroup);
    
    // Template editor section
    QGroupBox* editorGroup = new QGroupBox(tr("Template Editor"), this);
    QVBoxLayout* editorLayout = new QVBoxLayout(editorGroup);
    
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
    
    leftLayout->addWidget(editorGroup);
    
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
            if (m_templateManager->loadFromFile(fileName)) {
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
            if (m_templateManager->saveToFile(fileName)) {
                statusBar()->showMessage(tr("Saved templates to %1").arg(fileName));
            } else {
                QMessageBox::warning(this, tr("Error"),
                    tr("Failed to save templates to %1").arg(fileName));
            }
        }
    });
    
    // Help menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    
    QAction* aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QString resourceDir = m_resourceManager->findInstallationResourceDir();
        if (resourceDir.isEmpty()) {
            resourceDir = tr("(not found)");
        }
        
        QString resourceCounts = tr("%1 templates").arg(m_inventory->rowCount());
        
        QMessageBox::about(this, tr("About ScadTemplates"),
            tr("ScadTemplates v%1\n\n"
               "A code template and resource management tool for OpenSCAD.\n\n"
               "Platform: %2\n"
               "Resource Directory: %3\n\n"
               "Resources Found:\n%4\n\n"
               "Copyright (c) 2025\n"
               "MIT License")
            .arg(appInfo::version)
            .arg(m_resourceManager->folderName())
            .arg(resourceDir)
            .arg(resourceCounts));
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
    
    if (QMessageBox::question(this, tr("Delete Template"),
            tr("Delete template '%1'?").arg(m_selectedItem.name()),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        // TODO: Implement delete logic
        statusBar()->showMessage(tr("Deleted template: %1").arg(m_selectedItem.name()));
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
    m_editMode = true;
    m_prefixEdit->setReadOnly(false);
    m_descriptionEdit->setReadOnly(false);
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
    
    ResourceTemplate tmpl;
    tmpl.setPrefix(prefix);
    tmpl.setBody(body);
    tmpl.setDescription(description);
    tmpl.setName(prefix);
    tmpl.setFormat(QStringLiteral("text/scad.template"));
    tmpl.setSource(QStringLiteral("cppsnippet-made"));
    
    if (saveTemplateToUser(tmpl)) {
        m_editMode = false;
        refreshInventory();
        statusBar()->showMessage(tr("Saved template: %1").arg(prefix));
        updateTemplateButtons();
    }
}

void MainWindow::onCancelEdit() {
    m_editMode = false;
    m_prefixEdit->setReadOnly(true);
    m_descriptionEdit->setReadOnly(true);
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

void MainWindow::onInventoryItemSelected(const resourceInventory::ResourceItem& item) {
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
    QStandardItem* item = m_inventory->itemFromIndex(index);
    if (!item) return;
    
    // Retrieve full ResourceItem from Qt::UserRole
    QVariant itemData = item->data(Qt::UserRole);
    if (!itemData.isValid()) return;
    
    resourceInventory::ResourceItem resItem = itemData.value<resourceInventory::ResourceItem>();
    if (resItem.sourcePath().isEmpty()) {
        return;
    }
    
    onInventoryItemSelected(resItem);
}

void MainWindow::refreshInventory() {
    // Clear and rescan the model
    m_inventory->clear();
    
    // TODO: Implement actual discovery when ResourceLocationManager is complete
    QList<platformInfo::ResourceLocation> allLocations;
    
    resourceInventory::ResourceScanner scanner;
    scanner.scanToModel(m_inventory, allLocations);
    
    statusBar()->showMessage(tr("Inventory refreshed: %1 templates").arg(m_inventory->rowCount()), 3000);
}

void MainWindow::populateEditorFromSelection(const resourceInventory::ResourceItem& item) {
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
                } else {
                    // Fallback: treat as plain text
                    m_bodyEdit->setPlainText(QString::fromUtf8(data));
                    m_descriptionEdit->setText(item.description());
                    m_sourceEdit->setText(item.sourceLocationKey());
                }
            }
        } else {
            // Non-JSON: treat as plain text template
            m_bodyEdit->setPlainText(QString::fromUtf8(data));
            m_descriptionEdit->setText(item.description());
            m_sourceEdit->setText(item.sourceLocationKey());
        }
    } else {
        m_bodyEdit->setPlainText("");
        m_descriptionEdit->setText(item.description());
        m_sourceEdit->setText(item.sourceLocationKey());
    }

    updateTemplateButtons();
}

QString MainWindow::userTemplatesRoot() const {
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + 
           QStringLiteral("/ScadTemplates");
}

bool MainWindow::saveTemplateToUser(const ResourceTemplate& tmpl) {
    // TODO: Implement save to user templates
    return true;
}

void MainWindow::applyFilterToTree(const QString& text) {
    // TODO: Filter tree based on search text
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
