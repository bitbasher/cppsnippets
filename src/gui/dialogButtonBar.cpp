#include "gui/dialogButtonBar.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

DialogButtonBar::DialogButtonBar(QWidget* parent)
    : QWidget(parent)
{
    QGroupBox* groupBox = new QGroupBox(tr("Actions"));
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    m_restoreDefaultsButton = new QPushButton(tr("Restore Defaults"));
    m_restoreDefaultsButton->setToolTip(tr("to remove all user defined resource locations"));
    connect(m_restoreDefaultsButton, &QPushButton::clicked, 
            this, &DialogButtonBar::restoreDefaultsClicked);
    buttonLayout->addWidget(m_restoreDefaultsButton);
    
    buttonLayout->addStretch();
    
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogButtonBar::accepted);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogButtonBar::rejected);
    buttonLayout->addWidget(m_buttonBox);
    
    groupBox->setLayout(buttonLayout);
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(groupBox);
    setLayout(mainLayout);
}
