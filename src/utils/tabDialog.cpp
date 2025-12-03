#include "tabDialog.hpp"
#include "generalTab.hpp"
#include "permsTab.hpp"
#include "appsTab.hpp"

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFileInfo>

TabDialog::TabDialog(const QString& fileName, QWidget* parent)
    : QDialog(parent)
{
    QFileInfo fileInfo(fileName);

    // Create tabs - each is its own widget class
    tabWidget = new QTabWidget;
    tabWidget->addTab(new GeneralTab(fileInfo), tr("General"));
    tabWidget->addTab(new PermissionsTab(fileInfo), tr("Permissions"));
    tabWidget->addTab(new ApplicationsTab(fileInfo), tr("Applications"));

    // Create button box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Layout applied at the end
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Tab Dialog"));
}
