#include "gui/platformInfoWidget.hpp"
#include "platformInfo/resourceLocationManager.hpp"
#include "platformInfo/platformInfo.hpp"

#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>

PlatformInfoWidget::PlatformInfoWidget(QWidget* parent)
    : QWidget(parent)
{
    QGroupBox* groupBox = new QGroupBox(tr("Platform Information"));
    QFormLayout* formLayout = new QFormLayout;
    
    m_platformLabel = new QLabel;
    formLayout->addRow(tr("Detected Platform:"), m_platformLabel);
    
    groupBox->setLayout(formLayout);
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(groupBox);
    setLayout(mainLayout);
}

void PlatformInfoWidget::updateFromManager(platformInfo::ResourceLocationManager* manager)
{
    if (!manager) return;
    
    // Platform name - use prettyProductName() which includes OS name and version
    QString platformName = platformInfo::PlatformInfo::prettyProductName();
 
    m_platformLabel->setText(platformName);
}
