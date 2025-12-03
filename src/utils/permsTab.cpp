#include "permsTab.hpp"

#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QFrame>

PermissionsTab::PermissionsTab(const QFileInfo& fileInfo, QWidget* parent)
    : QWidget(parent)
{
    QGroupBox* permissionsGroup = new QGroupBox(tr("Permissions"));

    QCheckBox* readable = new QCheckBox(tr("Readable"));
    if (fileInfo.isReadable())
        readable->setChecked(true);

    QCheckBox* writable = new QCheckBox(tr("Writable"));
    if (fileInfo.isWritable())
        writable->setChecked(true);

    QCheckBox* executable = new QCheckBox(tr("Executable"));
    if (fileInfo.isExecutable())
        executable->setChecked(true);

    QGroupBox* ownerGroup = new QGroupBox(tr("Ownership"));

    QLabel* ownerLabel = new QLabel(tr("Owner"));
    QLabel* ownerValueLabel = new QLabel(fileInfo.owner());
    ownerValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel* groupLabel = new QLabel(tr("Group"));
    QLabel* groupValueLabel = new QLabel(fileInfo.group());
    groupValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    // Build layouts
    QVBoxLayout* permissionsLayout = new QVBoxLayout;
    permissionsLayout->addWidget(readable);
    permissionsLayout->addWidget(writable);
    permissionsLayout->addWidget(executable);
    permissionsGroup->setLayout(permissionsLayout);

    QVBoxLayout* ownerLayout = new QVBoxLayout;
    ownerLayout->addWidget(ownerLabel);
    ownerLayout->addWidget(ownerValueLabel);
    ownerLayout->addWidget(groupLabel);
    ownerLayout->addWidget(groupValueLabel);
    ownerGroup->setLayout(ownerLayout);

    // Main layout applied at the end
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(permissionsGroup);
    mainLayout->addWidget(ownerGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}
