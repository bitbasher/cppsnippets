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
    
    // Platform name
    QString platformName;
    switch (manager->osType()) {
        case platformInfo::ExtnOSType::Windows:
            platformName = tr("Windows");
            break;
        case platformInfo::ExtnOSType::MacOS:
            platformName = tr("macOS");
            break;
        case platformInfo::ExtnOSType::Linux:
            platformName = tr("Linux");
            break;
        case platformInfo::ExtnOSType::BSD:
            platformName = tr("BSD");
            break;
        default:
            platformName = tr("Unknown");
            break;
    }
    m_platformLabel->setText(platformName + QStringLiteral(" (") + 
                             platformInfo::PlatformInfo::productVersion() + QStringLiteral(")"));
}
