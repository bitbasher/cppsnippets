#include "gui/sysInfoDialog.hpp"
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSysInfo>

SysInfoDialog::SysInfoDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("System Information"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    QString text = tr("System Information:\n\n"
                     "Build ABI: %1\n"
                     "Build CPU Architecture: %2\n"
                     "Current CPU Architecture: %3\n"
                     "Kernel Type: %4\n"
                     "Kernel Version: %5\n"
                     "Machine Host Name: %6\n"
                     "Pretty Product Name: %7\n"
                     "Product Type: %8\n"
                     "Product Version: %9\n"
                     "Boot Unique ID: %10\n"
                     "Machine Unique ID: %11\n"
                     "Endian: %12\n"
                     "Word Size: %13-bit")
        .arg(QSysInfo::buildAbi())
        .arg(QSysInfo::buildCpuArchitecture())
        .arg(QSysInfo::currentCpuArchitecture())
        .arg(QSysInfo::kernelType())
        .arg(QSysInfo::kernelVersion())
        .arg(QSysInfo::machineHostName())
        .arg(QSysInfo::prettyProductName())
        .arg(QSysInfo::productType())
        .arg(QSysInfo::productVersion())
        .arg(QString::fromUtf8(QSysInfo::bootUniqueId().toHex()))
        .arg(QString::fromUtf8(QSysInfo::machineUniqueId().toHex()))
        .arg(QSysInfo::ByteOrder == QSysInfo::BigEndian ? "Big Endian" : (QSysInfo::ByteOrder == QSysInfo::LittleEndian ? "Little Endian" : "Unknown"))
        .arg(QSysInfo::WordSize);
    QLabel* label = new QLabel(text, this);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(label);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttonBox);
    setLayout(layout);
}
