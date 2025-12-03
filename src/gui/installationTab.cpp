#include "gui/installationTab.hpp"
#include "gui/resourceLocationWidget.hpp"

#include <QVBoxLayout>

InstallationTab::InstallationTab(QWidget* parent)
    : QWidget(parent)
    , m_fallbackMode(false)
{
    // allowAdd=true so we have the input widget, but start with it hidden
    // allowRemove=false since user shouldn't remove installation locations
    m_locationWidget = new ResourceLocationWidget(tr("Installation Resources"), true, false);
    m_locationWidget->setInputVisible(false);  // Hidden by default
    connect(m_locationWidget, &ResourceLocationWidget::locationsChanged,
            this, &InstallationTab::locationsChanged);
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_locationWidget);
    setLayout(mainLayout);
}

void InstallationTab::setFallbackMode(bool fallback)
{
    m_fallbackMode = fallback;
    m_locationWidget->setInputVisible(fallback);
}
