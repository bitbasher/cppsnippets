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
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_listWidget, &QListWidget::itemChanged, this, &ResourceLocationWidget::onItemChanged);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &ResourceLocationWidget::onSelectionChanged);
    groupLayout->addWidget(m_listWidget);
    
    // Remove button
    if (allowRemove) {
        QHBoxLayout* buttonLayout = new QHBoxLayout;
        m_removeButton = new QPushButton(tr("Remove"), m_groupBox);
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
    
    setLayout(mainLayout);
}

void ResourceLocationWidget::setLocations(const QList<platformInfo::ResourceLocation>& locations)
{
    m_locations = locations;
    
    m_listWidget->clear();
    for (const auto& loc : m_locations) {
        QListWidgetItem* item = new QListWidgetItem(m_listWidget);
        QString displayName = loc.getDisplayName();
        QString displayText = displayName.isEmpty() ? loc.path() : 
                              QString("%1 (%2)").arg(displayName, loc.path());
        item->setText(displayText);
        item->setData(Qt::UserRole, loc.path());
        
        // Determine if this item should be checkable
        // FIXME : need a correct way to set the checkbox
        
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState( Qt::Checked );
        
        // Show status via appearance
        // FIXME : need to fix this carefully
        item->setForeground(Qt::gray);
        if (loc.path().startsWith(QLatin1Char('('))) {
            item->setToolTip(loc.description());
        } else {
            item->setToolTip(tr("Path does not exist"));
        }
        if (!loc.hasResourceFolders()) {
            item->setForeground(Qt::darkGray);
            item->setToolTip(tr("Path has resource folders"));
        }
    }
}

QList<platformInfo::ResourceLocation> ResourceLocationWidget::locations() const
{
    QList<platformInfo::ResourceLocation> result;
    
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        QString path = item->data(Qt::UserRole).toString();
        
        // Find the original location and update enabled state
        for (const auto& loc : m_locations) {
            if (loc.path() == path) {
                platformInfo::ResourceLocation updated = loc;
                updated.setEnabled(item->checkState() == Qt::Checked);
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
    
    // Check if path already in list
    // FIXME : better to use .contains() on the QList?
    for (const auto& loc : m_locations) {
        if (loc.path() == path) {
            return; // Already on list
        }
    }
    
    platformInfo::ResourceLocation newLoc;
    newLoc.setPath(path);
    
    m_locations.append(newLoc);
    
    // Add to list widget
    QListWidgetItem* item = new QListWidgetItem(m_listWidget);
    QString displayName = newLoc.getDisplayName();
    QString displayText = displayName.isEmpty() ? newLoc.path() : 
                          QString("%1 (%2)").arg(displayName, newLoc.path());
    item->setText(displayText);
    item->setData(Qt::UserRole, newLoc.path());
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    
    // FIXME : make a correct check to gray out a disabled path
    if ( true) {
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
        if (m_locations[i].path() == path) {
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
