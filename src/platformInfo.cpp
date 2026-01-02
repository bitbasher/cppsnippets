/**
 * @file platformInfo.cpp
 * @brief Implementation of PlatformInfo class
 */

#include <platformInfo/platformInfo.hpp>
#include <QSysInfo>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

namespace platformInfo {

PlatformInfo::PlatformInfo()
    : m_osVersionRef()
{
    // Detect current OS and set the version reference
    ExtnOSType detectedType = currentOSType();
    const ExtnOSVersRef* ref = ExtnOSVersRef::findByOSType(detectedType);
    if (ref) {
        m_osVersionRef = *ref;
    }
}

PlatformInfo::PlatformInfo(const PlatformInfo& other)
    : m_osVersionRef(other.m_osVersionRef)
{
}

PlatformInfo& PlatformInfo::operator=(const PlatformInfo& other) {
    if (this != &other) {
        m_osVersionRef = other.m_osVersionRef;
    }
    return *this;
}

ExtnOSType PlatformInfo::currentOSType() const {
    QString product = QSysInfo::productType().toLower();
    
    if (product == QLatin1String("windows")) {
        return ExtnOSType::Windows;
    } else if (product == QLatin1String("macos") || product == QLatin1String("osx")) {
        return ExtnOSType::MacOS;
    } else if (product == QLatin1String("ios")) {
        return ExtnOSType::IOS;
    } else if (product == QLatin1String("tvos")) {
        return ExtnOSType::TvOS;
    } else if (product == QLatin1String("watchos")) {
        return ExtnOSType::WatchOS;
    } else if (product == QLatin1String("android")) {
        return ExtnOSType::Android;
    } else if (product.contains(QLatin1String("linux")) || 
               product == QLatin1String("ubuntu") ||
               product == QLatin1String("debian") ||
               product == QLatin1String("fedora") ||
               product == QLatin1String("rhel") ||
               product == QLatin1String("centos") ||
               product == QLatin1String("arch") ||
               product == QLatin1String("opensuse") ||
               product == QLatin1String("manjaro")) {
        return ExtnOSType::Linux;
    } else if (product.contains(QLatin1String("bsd"))) {
        return ExtnOSType::BSD;
    } else if (product.contains(QLatin1String("solaris"))) {
        return ExtnOSType::Solaris;
    }
    
    return ExtnOSType::Unknown;
}

const ExtnOSVersRef* PlatformInfo::currentOSVersionRef() const {
    return ExtnOSVersRef::findByOSType(m_osVersionRef.osType());
}

QString PlatformInfo::productType() {
    return QSysInfo::productType();
}

QString PlatformInfo::productVersion() {
    return QSysInfo::productVersion();
}

QString PlatformInfo::kernelType() {
    return QSysInfo::kernelType();
}

QString PlatformInfo::kernelVersion() {
    return QSysInfo::kernelVersion();
}

QString PlatformInfo::cpuArchitecture() {
    return QSysInfo::currentCpuArchitecture();
}

QString PlatformInfo::buildCpuArchitecture() {
    return QSysInfo::buildCpuArchitecture();
}

QString PlatformInfo::prettyProductName() {
    return QSysInfo::prettyProductName();
}

QString PlatformInfo::machineHostName() {
    return QSysInfo::machineHostName();
}

QList<QScreen*> PlatformInfo::screens() {
    return QGuiApplication::screens();
}

void PlatformInfo::debugPrint() const {
    qDebug() << "=== Platform Information ===";
    qDebug() << "Product Type:" << productType();
    qDebug() << "Product Version:" << productVersion();
    qDebug() << "Pretty Name:" << prettyProductName();
    qDebug() << "Kernel Type:" << kernelType();
    qDebug() << "Kernel Version:" << kernelVersion();
    qDebug() << "CPU Architecture:" << cpuArchitecture();
    qDebug() << "Build Architecture:" << buildCpuArchitecture();
    qDebug() << "Host Name:" << machineHostName();
    qDebug() << "OS Type:" << static_cast<int>(m_osVersionRef.osType());
    qDebug() << "OS Company:" << m_osVersionRef.osCompany();
    qDebug() << "OS Title:" << m_osVersionRef.osTitle();
    qDebug() << "OS ID:" << m_osVersionRef.osID();
}

} // namespace platformInfo
