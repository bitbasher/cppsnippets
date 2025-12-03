/**
 * @file mainwindow.cpp
 * @brief Main window implementation with text editor and preferences
 */

#include "mainwindow.h"
#include "gui/preferencesdialog.h"
#include <scadtemplates/scadtemplates.h>
#include <platformInfo/resourceLocationManager.h>
#include <resInventory/resourceScanner.h>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QListWidget>
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_templateManager(std::make_unique<scadtemplates::TemplateManager>())
    , m_resourceManager(std::make_unique<platformInfo::ResourceLocationManager>())
    , m_inventoryManager(std::make_unique<resInventory::ResourceInventoryManager>())
    , m_settings(std::make_unique<QSettings>(QStringLiteral("OpenSCAD"), QStringLiteral("ScadTemplates")))
    , m_templateList(nullptr)
    , m_prefixEdit(nullptr)
    , m_bodyEdit(nullptr)
    , m_descriptionEdit(nullptr)
    , m_searchEdit(nullptr)
    , m_editor(nullptr)
{
    // Set application path for resource manager
    m_resourceManager->setApplicationPath(QCoreApplication::applicationDirPath());
    
    // Build resource inventory from all known locations
    m_inventoryManager->buildInventory(*m_resourceManager);
    
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
    
    m_templateList = new QListWidget(this);
    connect(m_templateList, &QListWidget::itemSelectionChanged, 
            this, &MainWindow::onTemplateSelected);
    connect(m_templateList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        // Insert template into editor on double-click
        if (item && m_editor) {
            auto tmpl = m_templateManager->findByPrefix(item->text().toStdString());
            if (tmpl) {
                m_editor->insertPlainText(QString::fromStdString(tmpl->getBody()));
            }
        }
    });
    listLayout->addWidget(m_templateList);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* newBtn = new QPushButton(tr("New"), this);
    QPushButton* deleteBtn = new QPushButton(tr("Delete"), this);
    connect(newBtn, &QPushButton::clicked, this, &MainWindow::onNewTemplate);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteTemplate);
    buttonLayout->addWidget(newBtn);
    buttonLayout->addWidget(deleteBtn);
    listLayout->addLayout(buttonLayout);
    
    leftLayout->addWidget(listGroup);
    
    // Template editor section
    QGroupBox* editorGroup = new QGroupBox(tr("Template Editor"), this);
    QVBoxLayout* editorLayout = new QVBoxLayout(editorGroup);
    
    QHBoxLayout* prefixLayout = new QHBoxLayout();
    prefixLayout->addWidget(new QLabel(tr("Prefix:"), this));
    m_prefixEdit = new QLineEdit(this);
    prefixLayout->addWidget(m_prefixEdit);
    editorLayout->addLayout(prefixLayout);
    
    QHBoxLayout* descLayout = new QHBoxLayout();
    descLayout->addWidget(new QLabel(tr("Description:"), this));
    m_descriptionEdit = new QLineEdit(this);
    descLayout->addWidget(m_descriptionEdit);
    editorLayout->addLayout(descLayout);
    
    editorLayout->addWidget(new QLabel(tr("Body:"), this));
    m_bodyEdit = new QTextEdit(this);
    m_bodyEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_bodyEdit->setMaximumHeight(150);
    editorLayout->addWidget(m_bodyEdit);
    
    QPushButton* saveBtn = new QPushButton(tr("Save Template"), this);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveTemplate);
    editorLayout->addWidget(saveBtn);
    
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
            if (m_templateManager->loadFromFile(fileName.toStdString())) {
                refreshTemplateList();
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

void MainWindow::refreshTemplateList() {
    m_templateList->clear();
    
    auto templates = m_templateManager->getAllTemplates();
    for (const auto& tmpl : templates) {
        m_templateList->addItem(QString::fromStdString(tmpl.getPrefix()));
    }
}

void MainWindow::onNewTemplate() {
    m_prefixEdit->clear();
    m_bodyEdit->clear();
    m_descriptionEdit->clear();
    m_prefixEdit->setFocus();
    m_templateList->clearSelection();
}

void MainWindow::onDeleteTemplate() {
    auto items = m_templateList->selectedItems();
    if (items.isEmpty()) {
        return;
    }
    
    QString prefix = items.first()->text();
    
    if (QMessageBox::question(this, tr("Delete Template"),
            tr("Delete template '%1'?").arg(prefix),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (m_templateManager->removeTemplate(prefix.toStdString())) {
            refreshTemplateList();
            onNewTemplate();
            statusBar()->showMessage(tr("Deleted template: %1").arg(prefix));
        }
    }
}

void MainWindow::onSaveTemplate() {
    QString prefix = m_prefixEdit->text().trimmed();
    QString body = m_bodyEdit->toPlainText();
    QString description = m_descriptionEdit->text().trimmed();
    
    if (prefix.isEmpty() || body.isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
            tr("Prefix and body are required."));
        return;
    }
    
    scadtemplates::Template tmpl(prefix.toStdString(), 
                                  body.toStdString(),
                                  description.toStdString());
    
    if (m_templateManager->addTemplate(tmpl)) {
        refreshTemplateList();
        statusBar()->showMessage(tr("Saved template: %1").arg(prefix));
    }
}

void MainWindow::onTemplateSelected() {
    auto items = m_templateList->selectedItems();
    if (items.isEmpty()) {
        return;
    }
    
    QString prefix = items.first()->text();
    auto tmpl = m_templateManager->findByPrefix(prefix.toStdString());
    
    if (tmpl) {
        m_prefixEdit->setText(QString::fromStdString(tmpl->getPrefix()));
        m_bodyEdit->setPlainText(QString::fromStdString(tmpl->getBody()));
        m_descriptionEdit->setText(QString::fromStdString(tmpl->getDescription()));
    }
}

void MainWindow::onSearch(const QString& text) {
    m_templateList->clear();
    
    std::vector<scadtemplates::Template> templates;
    if (text.isEmpty()) {
        templates = m_templateManager->getAllTemplates();
    } else {
        templates = m_templateManager->search(text.toStdString());
    }
    
    for (const auto& tmpl : templates) {
        m_templateList->addItem(QString::fromStdString(tmpl.getPrefix()));
    }
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
