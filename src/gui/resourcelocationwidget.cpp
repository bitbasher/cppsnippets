/**
 * @file resourcelocationwidget.cpp
 * @brief Implementation of ResourceLocationWidget
 */

#include "gui/resourceLocationWidget.hpp"
#include "gui/locationInputWidget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QDir>

ResourceLocationWidget::ResourceLocationWidget(const QString& title,
                                                 bool allowAdd,
                                                 bool allowRemove,
                                                 QWidget* parent)
    : QWidget(parent)
    , m_groupBox(nullptr)
    , m_listWidget(nullptr)
    , m_inputWidget(nullptr)
    , m_removeButton(nullptr)
    , m_readOnly(false)
    , m_allowAdd(allowAdd)
{
    setupUi(title, allowAdd, allowRemove);
}

void ResourceLocationWidget::setupUi(const QString& title, bool allowAdd, bool allowRemove)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Group box with title for the list
    m_groupBox = new QGroupBox(title, this);
    QVBoxLayout* groupLayout = new QVBoxLayout(m_groupBox);
    
    // List widget for locations
    m_listWidget = new QListWidget(m_groupBox);
    m_listWidget->setObjectName("m_listWidget");
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_listWidget, &QListWidget::itemChanged, this, &ResourceLocationWidget::onItemChanged);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &ResourceLocationWidget::onSelectionChanged);
    groupLayout->addWidget(m_listWidget);
    
    // Remove button
    if (allowRemove) {
        QHBoxLayout* buttonLayout = new QHBoxLayout;
        m_removeButton = new QPushButton(tr("Remove"), m_groupBox);
        m_removeButton->setObjectName("m_removeButton");
        m_removeButton->setEnabled(false);
        connect(m_removeButton, &QPushButton::clicked, this, &ResourceLocationWidget::onRemoveLocation);
        buttonLayout->addWidget(m_removeButton);
        buttonLayout->addStretch();
        groupLayout->addLayout(buttonLayout);
    }
    
    mainLayout->addWidget(m_groupBox);
    
    // Input widget for adding new locations
    if (allowAdd) {
        m_inputWidget = new LocationInputWidget(this);
        connect(m_inputWidget, &LocationInputWidget::addClicked, this, &ResourceLocationWidget::onAddLocation);
        mainLayout->addWidget(m_inputWidget);
    }
    
    // Actions group box
    auto* actionsGroup = new QGroupBox(tr("Actions"), this);
    auto* actionsLayout = new QHBoxLayout(actionsGroup);
    m_rescanButton = new QPushButton(tr("Rescan Locations"), actionsGroup);
    m_rescanButton->setObjectName("m_rescanButton");
    m_rescanButton->setToolTip(tr("Rescan the filesystem to refresh location information"));
    connect(m_rescanButton, &QPushButton::clicked, this, &ResourceLocationWidget::rescanLocationsClicked);
    actionsLayout->addWidget(m_rescanButton);
    actionsLayout->addStretch();
    mainLayout->addWidget(actionsGroup);
    
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
    
    if (m_inputWidget) m_inputWidget->setEnabled(!readOnly);
    if (m_removeButton) m_removeButton->setEnabled(!readOnly);
    if (m_rescanButton) m_rescanButton->setEnabled(!readOnly);
    
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

void ResourceLocationWidget::setInputVisible(bool visible)
{
    if (m_inputWidget) {
        m_inputWidget->setVisible(visible);
    }
}

bool ResourceLocationWidget::isInputVisible() const
{
    return m_inputWidget ? m_inputWidget->isVisible() : false;
}

void ResourceLocationWidget::onAddLocation()
{
    if (m_readOnly || !m_inputWidget) return;
    
    QString path = m_inputWidget->path().trimmed();
    if (path.isEmpty()) return;
    
    // Check if path already exists in list
    for (const auto& loc : m_locations) {
        if (loc.path == path) {
            return; // Already exists
        }
    }
    
    platformInfo::ResourceLocation newLoc;
    newLoc.path = path;
    newLoc.displayName = m_inputWidget->name().trimmed();
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
    m_inputWidget->clear();
    
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
