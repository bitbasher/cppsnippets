/**
 * @file resourcelocationwidget.cpp
 * @brief Implementation of ResourceLocationWidget
 */

#include "preferencesdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QFileDialog>
#include <QDir>

ResourceLocationWidget::ResourceLocationWidget(const QString& title,
                                                 bool allowAdd,
                                                 bool allowRemove,
                                                 QWidget* parent)
    : QWidget(parent)
    , m_listWidget(nullptr)
    , m_pathEdit(nullptr)
    , m_nameEdit(nullptr)
    , m_addButton(nullptr)
    , m_removeButton(nullptr)
    , m_browseButton(nullptr)
    , m_readOnly(false)
{
    setupUi(title, allowAdd, allowRemove);
}

void ResourceLocationWidget::setupUi(const QString& title, bool allowAdd, bool allowRemove)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Group box with title
    QGroupBox* groupBox = new QGroupBox(title, this);
    QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);
    
    // List widget for locations
    m_listWidget = new QListWidget(groupBox);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_listWidget, &QListWidget::itemChanged, this, &ResourceLocationWidget::onItemChanged);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &ResourceLocationWidget::onSelectionChanged);
    groupLayout->addWidget(m_listWidget);
    
    // Edit section
    QHBoxLayout* editLayout = new QHBoxLayout();
    
    QLabel* pathLabel = new QLabel(tr("Path:"), groupBox);
    editLayout->addWidget(pathLabel);
    
    m_pathEdit = new QLineEdit(groupBox);
    m_pathEdit->setPlaceholderText(tr("Enter or browse for a folder path"));
    editLayout->addWidget(m_pathEdit);
    
    m_browseButton = new QPushButton(tr("Browse..."), groupBox);
    connect(m_browseButton, &QPushButton::clicked, this, &ResourceLocationWidget::onBrowse);
    editLayout->addWidget(m_browseButton);
    
    groupLayout->addLayout(editLayout);
    
    QHBoxLayout* nameLayout = new QHBoxLayout();
    
    QLabel* nameLabel = new QLabel(tr("Name:"), groupBox);
    nameLayout->addWidget(nameLabel);
    
    m_nameEdit = new QLineEdit(groupBox);
    m_nameEdit->setPlaceholderText(tr("Display name (optional)"));
    nameLayout->addWidget(m_nameEdit);
    
    groupLayout->addLayout(nameLayout);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    if (allowAdd) {
        m_addButton = new QPushButton(tr("Add"), groupBox);
        connect(m_addButton, &QPushButton::clicked, this, &ResourceLocationWidget::onAddLocation);
        buttonLayout->addWidget(m_addButton);
    }
    
    if (allowRemove) {
        m_removeButton = new QPushButton(tr("Remove"), groupBox);
        m_removeButton->setEnabled(false);
        connect(m_removeButton, &QPushButton::clicked, this, &ResourceLocationWidget::onRemoveLocation);
        buttonLayout->addWidget(m_removeButton);
    }
    
    buttonLayout->addStretch();
    groupLayout->addLayout(buttonLayout);
    
    mainLayout->addWidget(groupBox);
    setLayout(mainLayout);
}

void ResourceLocationWidget::setLocations(const QVector<platformInfo::ResourceLocation>& locations)
{
    m_locations = locations;
    
    m_listWidget->clear();
    for (const auto& loc : m_locations) {
        QListWidgetItem* item = new QListWidgetItem(m_listWidget);
        QString displayText = loc.displayName.isEmpty() ? loc.path : 
                              QString("%1 (%2)").arg(loc.displayName, loc.path);
        item->setText(displayText);
        item->setData(Qt::UserRole, loc.path);
        
        // Determine if this item should be checkable
        // Not checkable if: doesn't exist, or exists but has no resource folders
        bool canCheck = loc.exists && loc.hasResourceFolders;
        // Special case: placeholder items (path starts with '(') are never checkable
        if (loc.path.startsWith(QLatin1Char('('))) {
            canCheck = false;
        }
        
        if (canCheck) {
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(loc.isEnabled ? Qt::Checked : Qt::Unchecked);
        } else {
            // Remove checkable flag - item will appear without checkbox
            item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
        }
        
        // Show status via appearance
        if (!loc.exists) {
            item->setForeground(Qt::gray);
            if (loc.path.startsWith(QLatin1Char('('))) {
                item->setToolTip(loc.description);
            } else {
                item->setToolTip(tr("Path does not exist"));
            }
        } else if (!loc.hasResourceFolders) {
            item->setForeground(Qt::darkGray);
            item->setToolTip(tr("Path exists but contains no resource folders"));
        } else if (!loc.isWritable) {
            item->setToolTip(tr("Path is read-only"));
        }
    }
}

QVector<platformInfo::ResourceLocation> ResourceLocationWidget::locations() const
{
    QVector<platformInfo::ResourceLocation> result;
    
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        QString path = item->data(Qt::UserRole).toString();
        
        // Find the original location and update enabled state
        for (const auto& loc : m_locations) {
            if (loc.path == path) {
                platformInfo::ResourceLocation updated = loc;
                updated.isEnabled = (item->checkState() == Qt::Checked);
                result.append(updated);
                break;
            }
        }
    }
    
    return result;
}

QStringList ResourceLocationWidget::enabledPaths() const
{
    QStringList paths;
    
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            paths.append(item->data(Qt::UserRole).toString());
        }
    }
    
    return paths;
}

void ResourceLocationWidget::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    
    if (m_pathEdit) m_pathEdit->setEnabled(!readOnly);
    if (m_nameEdit) m_nameEdit->setEnabled(!readOnly);
    if (m_browseButton) m_browseButton->setEnabled(!readOnly);
    if (m_addButton) m_addButton->setEnabled(!readOnly);
    if (m_removeButton) m_removeButton->setEnabled(!readOnly);
    
    // In read-only mode, don't allow checking/unchecking
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        if (readOnly) {
            item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
        } else {
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        }
    }
}

void ResourceLocationWidget::onAddLocation()
{
    if (m_readOnly) return;
    
    QString path = m_pathEdit->text().trimmed();
    if (path.isEmpty()) return;
    
    // Check if path already exists in list
    for (const auto& loc : m_locations) {
        if (loc.path == path) {
            return; // Already exists
        }
    }
    
    platformInfo::ResourceLocation newLoc;
    newLoc.path = path;
    newLoc.displayName = m_nameEdit->text().trimmed();
    newLoc.isEnabled = true;
    newLoc.exists = QDir(path).exists();
    newLoc.isWritable = newLoc.exists; // Simplified check
    
    m_locations.append(newLoc);
    
    // Add to list widget
    QListWidgetItem* item = new QListWidgetItem(m_listWidget);
    QString displayText = newLoc.displayName.isEmpty() ? newLoc.path : 
                          QString("%1 (%2)").arg(newLoc.displayName, newLoc.path);
    item->setText(displayText);
    item->setData(Qt::UserRole, newLoc.path);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    
    if (!newLoc.exists) {
        item->setForeground(Qt::gray);
        item->setToolTip(tr("Path does not exist"));
    }
    
    // Clear inputs
    m_pathEdit->clear();
    m_nameEdit->clear();
    
    emit locationsChanged();
}

void ResourceLocationWidget::onRemoveLocation()
{
    if (m_readOnly) return;
    
    QListWidgetItem* item = m_listWidget->currentItem();
    if (!item) return;
    
    QString path = item->data(Qt::UserRole).toString();
    
    // Remove from internal list
    for (int i = 0; i < m_locations.size(); ++i) {
        if (m_locations[i].path == path) {
            m_locations.removeAt(i);
            break;
        }
    }
    
    // Remove from list widget
    delete m_listWidget->takeItem(m_listWidget->row(item));
    
    emit locationsChanged();
}

void ResourceLocationWidget::onBrowse()
{
    QString dir = QFileDialog::getExistingDirectory(this, 
        tr("Select Resource Directory"),
        m_pathEdit->text().isEmpty() ? QDir::homePath() : m_pathEdit->text());
    
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
        
        // Auto-fill name if empty
        if (m_nameEdit->text().isEmpty()) {
            QDir d(dir);
            m_nameEdit->setText(d.dirName());
        }
    }
}

void ResourceLocationWidget::onItemChanged()
{
    if (!m_readOnly) {
        emit locationsChanged();
    }
}

void ResourceLocationWidget::onSelectionChanged()
{
    updateButtons();
}

void ResourceLocationWidget::updateButtons()
{
    bool hasSelection = (m_listWidget->currentItem() != nullptr);
    
    if (m_removeButton) {
        m_removeButton->setEnabled(hasSelection && !m_readOnly);
    }
}
