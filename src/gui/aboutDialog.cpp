#include "gui/aboutDialog.hpp"
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include "applicationNameInfo.hpp"
#include <platformInfo/resourceLocationManager.hpp>

AboutDialog::AboutDialog(QWidget* parent, const QString& version, const QString& platform, const QString& resourceDir)
    : QDialog(parent)
{
    setWindowTitle(tr("About ScadTemplates"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    QString text = tr("ScadTemplates v%1\n\n"
                     "A code template and resource management tool for OpenSCAD.\n\n"
                     "Platform: %2\n"
                     "Resource Directory: %3\n\n"
                     "Copyright (c) 2025\n"
                     "MIT License")
        .arg(version)
        .arg(platform)
        .arg(resourceDir);
    QLabel* label = new QLabel(text, this);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(label);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttonBox);
    setLayout(layout);
}
