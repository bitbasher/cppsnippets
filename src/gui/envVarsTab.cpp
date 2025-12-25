/**
 * @file envVarsTab.cpp
 * @brief Implementation of environment variables editor tab
 */

#include "gui/envVarsTab.h"
#include "platformInfo/resourceLocationManager.h"
#include "platformInfo/resourcePaths.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QProcessEnvironment>

EnvVarsTab::EnvVarsTab(platformInfo::ResourceLocationManager* manager, QWidget* parent)
    : QWidget(parent)
    , m_manager(manager)
    , m_hasChanges(false)
{
    setupUI();
    loadEnvVars();
}

EnvVarsTab::~EnvVarsTab() = default;

void EnvVarsTab::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    auto* infoLabel = new QLabel(
        tr("Configure environment variables for use in resource path templates. "
           "Variables can use ${VAR} or %VAR% syntax."), this);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
    
    // Horizontal split: list on left, editor on right
    auto* contentLayout = new QHBoxLayout();
    
    // Left: List of env vars
    auto* listGroup = new QGroupBox(tr("Environment Variables"), this);
    auto* listLayout = new QVBoxLayout(listGroup);
    m_listWidget = new QListWidget(this);
    m_listWidget->setObjectName("m_listWidget");
    m_listWidget->setSortingEnabled(true);
    listLayout->addWidget(m_listWidget);
    contentLayout->addWidget(listGroup, 1);
    
    // Right: Editor
    auto* editorGroup = new QGroupBox(tr("Edit Variable"), this);
    auto* editorLayout = new QVBoxLayout(editorGroup);
    
    auto* formLayout = new QFormLayout();
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setObjectName("m_nameEdit");
    m_valueEdit = new QLineEdit(this);
    m_valueEdit->setObjectName("m_valueEdit");
    formLayout->addRow(tr("Name:"), m_nameEdit);
    formLayout->addRow(tr("Value:"), m_valueEdit);
    editorLayout->addLayout(formLayout);
    
    // Preview
    editorLayout->addWidget(new QLabel(tr("Expansion Preview:"), this));
    m_previewLabel = new QLabel(this);
    m_previewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setMinimumHeight(60);
    editorLayout->addWidget(m_previewLabel);
    
    // Editor buttons
    auto* editorButtonLayout = new QHBoxLayout();
    m_copyButton = new QPushButton(tr("Copy"), this);
    m_copyButton->setObjectName("m_copyButton");
    m_saveButton = new QPushButton(tr("Save"), this);
    m_saveButton->setObjectName("m_saveButton");
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_cancelButton->setObjectName("m_cancelButton");
    m_revertButton = new QPushButton(tr("Revert"), this);
    m_revertButton->setObjectName("m_revertButton");
    m_revertButton->setToolTip(tr("Revert changes to this variable made during this session"));
    editorButtonLayout->addWidget(m_copyButton);
    editorButtonLayout->addWidget(m_saveButton);
    editorButtonLayout->addWidget(m_cancelButton);
    editorButtonLayout->addWidget(m_revertButton);
    editorButtonLayout->addStretch();
    editorLayout->addLayout(editorButtonLayout);
    
    contentLayout->addWidget(editorGroup, 1);
    mainLayout->addLayout(contentLayout);
    
    // Bottom: Global action buttons (Env Vars scope)
    auto* actionLayout = new QHBoxLayout();
    m_restoreDefaultsButton = new QPushButton(tr("Reload Env Vars"), this);
    m_restoreDefaultsButton->setObjectName("m_restoreDefaultsButton");
    m_restoreDefaultsButton->setToolTip(tr("Reload system environment plus saved overrides from settings"));
    actionLayout->addWidget(m_restoreDefaultsButton);
    actionLayout->addStretch();
    mainLayout->addLayout(actionLayout);
    
    // Connect signals
    connect(m_listWidget, &QListWidget::currentTextChanged, this, &EnvVarsTab::onListSelectionChanged);
    connect(m_nameEdit, &QLineEdit::textChanged, this, &EnvVarsTab::onNameChanged);
    connect(m_valueEdit, &QLineEdit::textChanged, this, &EnvVarsTab::onValueChanged);
    connect(m_copyButton, &QPushButton::clicked, this, &EnvVarsTab::onCopyClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &EnvVarsTab::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &EnvVarsTab::onCancelClicked);
    connect(m_revertButton, &QPushButton::clicked, this, &EnvVarsTab::onRevertVarClicked);
    connect(m_restoreDefaultsButton, &QPushButton::clicked, this, &EnvVarsTab::onRestoreDefaultsClicked);
    
    updateButtonStates();
}

void EnvVarsTab::loadEnvVars() {
    m_editingEnvVars.clear();
    m_listWidget->clear();
    
    // Populate from system environment first
    QProcessEnvironment sysEnv = QProcessEnvironment::systemEnvironment();
    const QStringList keys = sysEnv.keys();
    for (const QString& key : keys) {
        const QString val = sysEnv.value(key);
        m_editingEnvVars.insert(key, val);
        m_listWidget->addItem(key);
    }

    // Overlay with user overrides from settings (manager ResourcePaths)
    const auto& overrides = m_manager->resourcePaths().envVars();
    for (const auto& entry : overrides) {
        m_editingEnvVars[entry.name] = entry.value;
        // Ensure listed once
        QList<QListWidgetItem*> items = m_listWidget->findItems(entry.name, Qt::MatchExactly);
        if (items.isEmpty()) {
            m_listWidget->addItem(entry.name);
        }
    }
    
    m_hasChanges = false;
    updateButtonStates();
}

void EnvVarsTab::saveEnvVars() {
    // Save only overrides (differences vs system env) and newly defined vars
    platformInfo::ResourcePaths& paths = m_manager->resourcePaths();
    QProcessEnvironment sysEnv = QProcessEnvironment::systemEnvironment();
    QList<platformInfo::EnvVarEntry> toSave;

    for (auto it = m_editingEnvVars.constBegin(); it != m_editingEnvVars.constEnd(); ++it) {
        const QString name = it.key();
        const QString value = it.value();
        const QString sysVal = sysEnv.contains(name) ? sysEnv.value(name) : QString();
        // Save if new var or value differs from system
        if (!sysEnv.contains(name) || value != sysVal) {
            platformInfo::EnvVarEntry entry; entry.name = name; entry.value = value; toSave.append(entry);
        }
    }

    paths.setEnvVars(toSave);
    
    m_manager->saveEnvVarsToSettings();
    m_hasChanges = false;
    
    emit envVarsChanged();
}

void EnvVarsTab::onListSelectionChanged() {
    const QString selected = m_listWidget->currentItem() ? m_listWidget->currentItem()->text() : QString();
    
    if (selected.isEmpty()) {
        m_nameEdit->clear();
        m_valueEdit->clear();
        m_currentEditName.clear();
    } else {
        m_nameEdit->setText(selected);
        m_valueEdit->setText(m_editingEnvVars.value(selected));
        m_currentEditName = selected;
    }
    
    updatePreview();
    updateButtonStates();
}

void EnvVarsTab::onNameChanged(const QString& text) {
    updatePreview();
    updateButtonStates();
}

void EnvVarsTab::onValueChanged(const QString& text) {
    updatePreview();
    updateButtonStates();
}

void EnvVarsTab::onCopyClicked() {
    const QString originalName = m_currentEditName;
    if (originalName.isEmpty()) {
        return;
    }
    
    // Create copy with "_copy" suffix
    QString newName = originalName + "_copy";
    int counter = 1;
    while (m_editingEnvVars.contains(newName)) {
        newName = originalName + QString("_copy%1").arg(counter++);
    }
    
    m_nameEdit->setText(newName);
    m_nameEdit->selectAll();
    m_nameEdit->setFocus();
}

void EnvVarsTab::onSaveClicked() {
    const QString name = m_nameEdit->text().trimmed();
    const QString value = m_valueEdit->text();
    
    QString errorMsg;
    if (!validateEnvVar(name, value, errorMsg)) {
        QMessageBox::warning(this, tr("Invalid Variable"), errorMsg);
        return;
    }
    
    // Check if renaming from copy operation
    if (!m_currentEditName.isEmpty() && m_currentEditName != name) {
        if (m_editingEnvVars.contains(name)) {
            QMessageBox::warning(this, tr("Duplicate Name"),
                tr("Variable '%1' already exists. Choose a different name.").arg(name));
            return;
        }
    }
    
    // Save to editing map
    m_editingEnvVars[name] = value;
    m_hasChanges = true;
    
    // Update list
    QListWidgetItem* item = m_listWidget->findItems(name, Qt::MatchExactly).value(0);
    if (!item) {
        m_listWidget->addItem(name);
        m_listWidget->setCurrentRow(m_listWidget->count() - 1);
    }
    
    m_currentEditName = name;
    
    emit envVarsChanged();
    updateButtonStates();
}

void EnvVarsTab::onCancelClicked() {
    // Reload from editing map
    if (!m_currentEditName.isEmpty()) {
        m_nameEdit->setText(m_currentEditName);
        m_valueEdit->setText(m_editingEnvVars.value(m_currentEditName));
    } else {
        m_nameEdit->clear();
        m_valueEdit->clear();
    }
    
    updatePreview();
}

void EnvVarsTab::onRevertVarClicked() {
    if (m_currentEditName.isEmpty()) {
        return;
    }
    
    // Check if there's an underlying system env var (case-sensitive)
    QProcessEnvironment sysEnv = QProcessEnvironment::systemEnvironment();
    bool hasSystemVar = sysEnv.contains(m_currentEditName);
    QString systemValue = hasSystemVar ? sysEnv.value(m_currentEditName) : QString();
    
    QMessageBox::StandardButton reply;
    
    if (!hasSystemVar) {
        // User-defined variable with no system counterpart - offer to delete
        reply = QMessageBox::question(this, tr("Undefine Variable"),
            tr("Do you want to undefine this variable?\n\n"
               "There will be no changes to variables defined in the shell."),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            // Remove from editing map and list
            m_editingEnvVars.remove(m_currentEditName);
            QList<QListWidgetItem*> items = m_listWidget->findItems(m_currentEditName, Qt::MatchExactly);
            for (QListWidgetItem* item : items) {
                delete m_listWidget->takeItem(m_listWidget->row(item));
            }
            
            // Clear editor
            m_nameEdit->clear();
            m_valueEdit->clear();
            m_currentEditName.clear();
            m_hasChanges = true;
            
            emit envVarsChanged();
            updatePreview();
            updateButtonStates();
        }
    } else {
        // System variable that's been overridden - offer to revert to system value
        reply = QMessageBox::question(this, tr("Revert to System Value"),
            tr("Reverting back to shell env var content:\n\n%1").arg(systemValue),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            // Revert to system value in editing map
            m_editingEnvVars[m_currentEditName] = systemValue;
            
            // Update editor
            m_nameEdit->setText(m_currentEditName);
            m_valueEdit->setText(systemValue);
            m_hasChanges = true;
            
            emit envVarsChanged();
            updatePreview();
            updateButtonStates();
        }
    }
}

void EnvVarsTab::onRestoreDefaultsClicked() {
    // Reload from system environment + saved overrides
    loadEnvVars();
}

void EnvVarsTab::updatePreview() {
    const QString value = m_valueEdit->text();
    if (value.isEmpty()) {
        m_previewLabel->setText(tr("<i>No value to preview</i>"));
        return;
    }
    
    // Show expansion preview
    const QString expanded = expandPreviewPath(value);
    m_previewLabel->setText(QString("<b>%1</b>").arg(expanded));
}

void EnvVarsTab::updateButtonStates() {
    const QString currentName = m_nameEdit->text().trimmed();
    const bool hasSelection = !m_currentEditName.isEmpty();
    const bool hasText = !currentName.isEmpty();
    
    // A variable is "new" if we're editing a name that's not in the list
    const bool isNewVar = hasText && (m_currentEditName.isEmpty() || m_currentEditName != currentName);
    
    // Revert is enabled for variables that exist in the list (either user-defined or overridden)
    // But disabled when creating a brand new variable
    const bool canRevert = hasSelection && !isNewVar;
    
    m_copyButton->setEnabled(hasSelection);
    m_saveButton->setEnabled(hasText);
    m_cancelButton->setEnabled(hasText);
    m_revertButton->setEnabled(canRevert);
}

bool EnvVarsTab::validateEnvVar(const QString& name, const QString& value, QString& errorMsg) {
    if (name.isEmpty()) {
        errorMsg = tr("Variable name cannot be empty");
        return false;
    }
    
    // Basic name validation (alphanumeric + underscore)
    for (const QChar& ch : name) {
        if (!ch.isLetterOrNumber() && ch != '_') {
            errorMsg = tr("Variable name can only contain letters, numbers, and underscores");
            return false;
        }
    }
    
    return true;
}

QString EnvVarsTab::expandPreviewPath(const QString& path) const {
    // Create temporary ResourcePaths with current editing env vars
    platformInfo::ResourcePaths tempPaths;
    for (auto it = m_editingEnvVars.constBegin(); it != m_editingEnvVars.constEnd(); ++it) {
        tempPaths.addEnvVar(it.key(), it.value());
    }
    
    return tempPaths.expandEnvVars(path);
}
