/**
 * @file resourcelocationwidget.cpp
 * @brief Implementation of ResourceLocationWidget
 */

#include "gui/resourceLocationWidget.hpp"
#include "gui/resourceLocationModel.h"
#include "gui/locationInputWidget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QDir>

ResourceLocationWidget::ResourceLocationWidget(const QString& title,
                                                 bool allowAdd,
                                                 bool allowRemove,
                                                 QWidget* parent)
    : QWidget(parent)
    , m_groupBox(nullptr)
    , m_listView(nullptr)
    , m_model(nullptr)
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
    
    // Create model and list view
    m_model = new ResourceLocationModel(this);
    m_listView = new QListView(m_groupBox);
    m_listView->setObjectName("m_listView");
    m_listView->setModel(m_model);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Connect model changes
    connect(m_model, &QAbstractListModel::dataChanged, this, &ResourceLocationWidget::locationsChanged);
    
    groupLayout->addWidget(m_listView);
    
    // Remove button
    if (allowRemove) {
        QHBoxLayout* buttonLayout = new QHBoxLayout;
        m_removeButton = new QPushButton(tr("Remove"), m_groupBox);
        m_removeButton->setObjectName("m_removeButton");
        m_removeButton->setEnabled(false);
        connect(m_removeButton, &QPushButton::clicked, this, &ResourceLocationWidget::onRemoveLocation);
        connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &ResourceLocationWidget::onSelectionChanged);
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

void ResourceLocationWidget::setLocations(const QList<platformInfo::ResourceLocation>& locations)
{
    m_model->setLocations(locations);
}

QList<platformInfo::ResourceLocation> ResourceLocationWidget::locations() const
{
    return m_model->locations();
}

QStringList ResourceLocationWidget::enabledPaths() const
{
    return m_model->enabledPaths();
}

ResourceLocationModel* ResourceLocationWidget::model() const
{
    return m_model;
}

void ResourceLocationWidget::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    
    if (m_inputWidget) m_inputWidget->setEnabled(!readOnly);
    if (m_removeButton) m_removeButton->setEnabled(!readOnly);
    if (m_rescanButton) m_rescanButton->setEnabled(!readOnly);
    
    m_listView->setEnabled(!readOnly);
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
    for (const auto& loc : m_model->locations()) {
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
    
    QList<platformInfo::ResourceLocation> updated = m_model->locations();
    updated.append(newLoc);
    m_model->setLocations(updated);
    
    // Clear inputs
    m_inputWidget->clear();
    
    emit locationAdded();
}

void ResourceLocationWidget::onRemoveLocation()
{
    if (m_readOnly) return;
    
    QModelIndex current = m_listView->currentIndex();
    if (!current.isValid()) return;
    
    QList<platformInfo::ResourceLocation> locs = m_model->locations();
    if (current.row() >= 0 && current.row() < locs.size()) {
        locs.removeAt(current.row());
        m_model->setLocations(locs);
    }
    
    emit locationsChanged();
}

void ResourceLocationWidget::onSelectionChanged()
{
    updateButtons();
}

void ResourceLocationWidget::updateButtons()
{
    bool hasSelection = m_listView->currentIndex().isValid();
    
    if (m_removeButton) {
        m_removeButton->setEnabled(hasSelection && !m_readOnly);
    }
}
