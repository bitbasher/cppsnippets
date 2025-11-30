/**
 * @file mainwindow.cpp
 * @brief Main window implementation
 */

#include "mainwindow.h"
#include <cppsnippets/cppsnippets.h>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_snippetManager(std::make_unique<cppsnippets::SnippetManager>())
    , m_snippetList(nullptr)
    , m_prefixEdit(nullptr)
    , m_bodyEdit(nullptr)
    , m_descriptionEdit(nullptr)
    , m_searchEdit(nullptr) {
    
    setupUi();
    setupMenus();
    
    setWindowTitle(QString("CppSnippets v%1").arg(cppsnippets::getVersion()));
    resize(800, 600);
    
    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    
    // Left panel - snippet list
    QGroupBox* listGroup = new QGroupBox(tr("Snippets"), this);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search snippets..."));
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearch);
    listLayout->addWidget(m_searchEdit);
    
    m_snippetList = new QListWidget(this);
    connect(m_snippetList, &QListWidget::itemSelectionChanged, 
            this, &MainWindow::onSnippetSelected);
    listLayout->addWidget(m_snippetList);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* newBtn = new QPushButton(tr("New"), this);
    QPushButton* deleteBtn = new QPushButton(tr("Delete"), this);
    connect(newBtn, &QPushButton::clicked, this, &MainWindow::onNewSnippet);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteSnippet);
    buttonLayout->addWidget(newBtn);
    buttonLayout->addWidget(deleteBtn);
    listLayout->addLayout(buttonLayout);
    
    mainLayout->addWidget(listGroup, 1);
    
    // Right panel - snippet editor
    QGroupBox* editorGroup = new QGroupBox(tr("Snippet Editor"), this);
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
    m_bodyEdit->setFont(QFont("Courier", 10));
    editorLayout->addWidget(m_bodyEdit);
    
    QPushButton* saveBtn = new QPushButton(tr("Save Snippet"), this);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveSnippet);
    editorLayout->addWidget(saveBtn);
    
    mainLayout->addWidget(editorGroup, 2);
}

void MainWindow::setupMenus() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    
    QAction* openAction = fileMenu->addAction(tr("&Open..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Snippets File"), QString(),
            tr("JSON Files (*.json);;All Files (*)"));
        if (!fileName.isEmpty()) {
            if (m_snippetManager->loadFromFile(fileName.toStdString())) {
                refreshSnippetList();
                statusBar()->showMessage(tr("Loaded snippets from %1").arg(fileName));
            } else {
                QMessageBox::warning(this, tr("Error"), 
                    tr("Failed to load snippets from %1").arg(fileName));
            }
        }
    });
    
    QAction* saveAction = fileMenu->addAction(tr("&Save..."));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Snippets File"), QString(),
            tr("JSON Files (*.json);;All Files (*)"));
        if (!fileName.isEmpty()) {
            if (m_snippetManager->saveToFile(fileName.toStdString())) {
                statusBar()->showMessage(tr("Saved snippets to %1").arg(fileName));
            } else {
                QMessageBox::warning(this, tr("Error"),
                    tr("Failed to save snippets to %1").arg(fileName));
            }
        }
    });
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    // Help menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    
    QAction* aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("About CppSnippets"),
            tr("CppSnippets v%1\n\n"
               "A comprehensive snippets tool for C++ programs.\n\n"
               "Copyright (c) 2025 Jeff Hayes\n"
               "MIT License").arg(cppsnippets::getVersion()));
    });
}

void MainWindow::refreshSnippetList() {
    m_snippetList->clear();
    
    auto snippets = m_snippetManager->getAllSnippets();
    for (const auto& snippet : snippets) {
        m_snippetList->addItem(QString::fromStdString(snippet.getPrefix()));
    }
}

void MainWindow::onNewSnippet() {
    m_prefixEdit->clear();
    m_bodyEdit->clear();
    m_descriptionEdit->clear();
    m_prefixEdit->setFocus();
    m_snippetList->clearSelection();
}

void MainWindow::onDeleteSnippet() {
    auto items = m_snippetList->selectedItems();
    if (items.isEmpty()) {
        return;
    }
    
    QString prefix = items.first()->text();
    if (m_snippetManager->removeSnippet(prefix.toStdString())) {
        refreshSnippetList();
        onNewSnippet();
        statusBar()->showMessage(tr("Deleted snippet: %1").arg(prefix));
    }
}

void MainWindow::onSaveSnippet() {
    QString prefix = m_prefixEdit->text().trimmed();
    QString body = m_bodyEdit->toPlainText();
    QString description = m_descriptionEdit->text().trimmed();
    
    if (prefix.isEmpty() || body.isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
            tr("Prefix and body are required."));
        return;
    }
    
    cppsnippets::Snippet snippet(prefix.toStdString(), 
                                  body.toStdString(),
                                  description.toStdString());
    
    if (m_snippetManager->addSnippet(snippet)) {
        refreshSnippetList();
        statusBar()->showMessage(tr("Saved snippet: %1").arg(prefix));
    }
}

void MainWindow::onSnippetSelected() {
    auto items = m_snippetList->selectedItems();
    if (items.isEmpty()) {
        return;
    }
    
    QString prefix = items.first()->text();
    auto snippet = m_snippetManager->findByPrefix(prefix.toStdString());
    
    if (snippet) {
        m_prefixEdit->setText(QString::fromStdString(snippet->getPrefix()));
        m_bodyEdit->setPlainText(QString::fromStdString(snippet->getBody()));
        m_descriptionEdit->setText(QString::fromStdString(snippet->getDescription()));
    }
}

void MainWindow::onSearch(const QString& text) {
    m_snippetList->clear();
    
    std::vector<cppsnippets::Snippet> snippets;
    if (text.isEmpty()) {
        snippets = m_snippetManager->getAllSnippets();
    } else {
        snippets = m_snippetManager->search(text.toStdString());
    }
    
    for (const auto& snippet : snippets) {
        m_snippetList->addItem(QString::fromStdString(snippet.getPrefix()));
    }
}
