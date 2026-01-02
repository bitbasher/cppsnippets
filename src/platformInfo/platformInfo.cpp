/**
 * @file platformInfo.cpp
 * @brief Implementation of PlatformInfo class
 */

#include "platformInfo.hpp"

namespace platformInfo {

// Platform info is all static, no initialization needed
PlatformInfo::PlatformInfo() {}

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
    qDebug() << "Executable Path:" << getCurrentExecutablePath();
    qDebug() << "Executable Dir:" << getCurrentExecutableDirPath();
    
    QList<QScreen*> screenList = screens();
    qDebug() << "Screen Count:" << screenList.size();
    int i = 0;
    for (QScreen* screen : screenList) {
        qDebug() << "  Screen" << i++ << ":"
                 << "Name:" << screen->name()
                 << "Geometry:" << screen->geometry()
                 << "DPI:" << screen->logicalDotsPerInch();
    }
}

} // namespace platformInfo
