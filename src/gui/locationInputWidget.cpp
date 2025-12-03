#include "gui/locationInputWidget.hpp"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>

LocationInputWidget::LocationInputWidget(QWidget* parent)
    : QWidget(parent)
    , m_readOnly(false)
{
    m_groupBox = new QGroupBox(tr("Add New Location"));
    QVBoxLayout* groupLayout = new QVBoxLayout;
    
    // Path row
    QHBoxLayout* pathLayout = new QHBoxLayout;
    QLabel* pathLabel = new QLabel(tr("Path:"));
    pathLayout->addWidget(pathLabel);
    
    m_pathEdit = new QLineEdit;
    m_pathEdit->setPlaceholderText(tr("Enter or browse for a folder path"));
    connect(m_pathEdit, &QLineEdit::textChanged, this, &LocationInputWidget::onPathChanged);
    pathLayout->addWidget(m_pathEdit);
    
    m_browseButton = new QPushButton(tr("Browse..."));
    connect(m_browseButton, &QPushButton::clicked, this, &LocationInputWidget::onBrowse);
    pathLayout->addWidget(m_browseButton);
    
    groupLayout->addLayout(pathLayout);
    
    // Name row
    QHBoxLayout* nameLayout = new QHBoxLayout;
    QLabel* nameLabel = new QLabel(tr("Name:"));
    nameLayout->addWidget(nameLabel);
    
    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText(tr("Display name (optional)"));
    connect(m_nameEdit, &QLineEdit::textChanged, this, &LocationInputWidget::inputChanged);
    nameLayout->addWidget(m_nameEdit);
    
    groupLayout->addLayout(nameLayout);
    
    // Add button
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    
    m_addButton = new QPushButton(tr("Add"));
    m_addButton->setEnabled(false); // Disabled until path is entered
    connect(m_addButton, &QPushButton::clicked, this, &LocationInputWidget::addClicked);
    buttonLayout->addWidget(m_addButton);
    
    groupLayout->addLayout(buttonLayout);
    
    m_groupBox->setLayout(groupLayout);
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_groupBox);
    setLayout(mainLayout);
}

QString LocationInputWidget::path() const
{
    return m_pathEdit->text().trimmed();
}

QString LocationInputWidget::name() const
{
    return m_nameEdit->text().trimmed();
}

void LocationInputWidget::setPath(const QString& path)
{
    m_pathEdit->setText(path);
}

void LocationInputWidget::setName(const QString& name)
{
    m_nameEdit->setText(name);
}

void LocationInputWidget::clear()
{
    m_pathEdit->clear();
    m_nameEdit->clear();
}

void LocationInputWidget::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    m_pathEdit->setEnabled(!readOnly);
    m_nameEdit->setEnabled(!readOnly);
    m_browseButton->setEnabled(!readOnly);
    m_addButton->setEnabled(!readOnly && !m_pathEdit->text().trimmed().isEmpty());
}

void LocationInputWidget::onBrowse()
{
    QString startDir = m_pathEdit->text().isEmpty() ? QDir::homePath() : m_pathEdit->text();
    
    QString dir = QFileDialog::getExistingDirectory(this, 
        tr("Select Resource Directory"),
        startDir);
    
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
        
        // Auto-fill name with the folder name
        QFileInfo info(dir);
        m_nameEdit->setText(info.fileName());
    }
}

void LocationInputWidget::onPathChanged()
{
    // Enable Add button only if path is not empty
    bool hasPath = !m_pathEdit->text().trimmed().isEmpty();
    m_addButton->setEnabled(hasPath && !m_readOnly);
    
    emit inputChanged();
}
