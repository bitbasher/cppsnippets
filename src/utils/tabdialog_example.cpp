/**
 * @file tabdialog_example.cpp
 * @brief Example of Qt TabDialog pattern for reference
 * 
 * This file demonstrates the proper Qt pattern for creating tabbed dialogs
 * where each tab is its own QWidget subclass. This is a standalone example
 * and is NOT part of the ScadTemplates application.
 * 
 * Key patterns demonstrated:
 * - Each tab is a separate QWidget subclass
 * - Layouts are applied at the end of constructors
 * - The dialog simply assembles the tab widgets
 */

#include <QApplication>
#include <QDialog>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QFileInfo>
#include <QFrame>

// ============================================================================
// GeneralTab - displays general file information
// ============================================================================
class GeneralTab : public QWidget {
    Q_OBJECT
public:
    explicit GeneralTab(const QFileInfo& fileInfo, QWidget* parent = nullptr);
};

GeneralTab::GeneralTab(const QFileInfo& fileInfo, QWidget* parent)
    : QWidget(parent)
{
    QLabel* fileNameLabel = new QLabel(tr("File Name:"));
    QLineEdit* fileNameEdit = new QLineEdit(fileInfo.fileName());

    QLabel* pathLabel = new QLabel(tr("Path:"));
    QLabel* pathValueLabel = new QLabel(fileInfo.absoluteFilePath());
    pathValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel* sizeLabel = new QLabel(tr("Size:"));
    qlonglong size = fileInfo.size() / 1024;
    QLabel* sizeValueLabel = new QLabel(tr("%1 K").arg(size));
    sizeValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel* lastReadLabel = new QLabel(tr("Last Read:"));
    QLabel* lastReadValueLabel = new QLabel(fileInfo.lastRead().toString());
    lastReadValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel* lastModLabel = new QLabel(tr("Last Modified:"));
    QLabel* lastModValueLabel = new QLabel(fileInfo.lastModified().toString());
    lastModValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    // Layout applied at the end
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(fileNameLabel);
    mainLayout->addWidget(fileNameEdit);
    mainLayout->addWidget(pathLabel);
    mainLayout->addWidget(pathValueLabel);
    mainLayout->addWidget(sizeLabel);
    mainLayout->addWidget(sizeValueLabel);
    mainLayout->addWidget(lastReadLabel);
    mainLayout->addWidget(lastReadValueLabel);
    mainLayout->addWidget(lastModLabel);
    mainLayout->addWidget(lastModValueLabel);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

// ============================================================================
// PermissionsTab - displays file permissions
// ============================================================================
class PermissionsTab : public QWidget {
    Q_OBJECT
public:
    explicit PermissionsTab(const QFileInfo& fileInfo, QWidget* parent = nullptr);
};

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

// ============================================================================
// ApplicationsTab - displays application associations
// ============================================================================
class ApplicationsTab : public QWidget {
    Q_OBJECT
public:
    explicit ApplicationsTab(const QFileInfo& fileInfo, QWidget* parent = nullptr);
};

ApplicationsTab::ApplicationsTab(const QFileInfo& fileInfo, QWidget* parent)
    : QWidget(parent)
{
    QLabel* topLabel = new QLabel(tr("Open with:"));

    QListWidget* applicationsListBox = new QListWidget;
    QStringList applications;
    for (int i = 1; i <= 30; ++i)
        applications.append(tr("Application %1").arg(i));
    applicationsListBox->insertItems(0, applications);

    QCheckBox* alwaysCheckBox;
    if (fileInfo.suffix().isEmpty())
        alwaysCheckBox = new QCheckBox(tr("Always use this application to "
                                          "open this type of file"));
    else
        alwaysCheckBox = new QCheckBox(tr("Always use this application to "
                                          "open files with the extension '%1'").arg(fileInfo.suffix()));

    // Layout applied at the end
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    layout->addWidget(applicationsListBox);
    layout->addWidget(alwaysCheckBox);
    setLayout(layout);
}

// ============================================================================
// TabDialog - assembles the tabs
// ============================================================================
class TabDialog : public QDialog {
    Q_OBJECT
public:
    explicit TabDialog(const QString& fileName, QWidget* parent = nullptr);
private:
    QTabWidget* tabWidget;
    QDialogButtonBox* buttonBox;
};

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

// ============================================================================
// Main - example entry point
// ============================================================================
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    QString fileName = (argc > 1) ? argv[1] : QApplication::applicationFilePath();
    
    TabDialog dialog(fileName);
    return dialog.exec();
}
