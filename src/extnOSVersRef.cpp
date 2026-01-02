/**
 * @file extnOSVersRef.cpp
 * @brief Implementation of ExtnOSVersRef class
 */

#include "platformInfo/extnOSVersRef.hpp"

namespace platformInfo {

ExtnOSVersRef::ExtnOSVersRef()
    : m_osType(ExtnOSType::Unknown)
    , m_osCompany(QStringLiteral("unknown"))
    , m_osTitle(QStringLiteral("unknown"))
    , m_osID(QStringLiteral("unknown"))
{
}

ExtnOSVersRef::ExtnOSVersRef(ExtnOSType type, const QString& company, const QString& title, const QString& id)
    : m_osType(type)
    , m_osCompany(company)
    , m_osTitle(title)
    , m_osID(id)
{
}

QVector<ExtnOSVersRef> ExtnOSVersRef::allOSVersions() {
    return {
        ExtnOSVersRef(ExtnOSType::Unknown, QStringLiteral("unknown"), QStringLiteral("unknown"), QStringLiteral("unknown")),
        ExtnOSVersRef(ExtnOSType::Windows, QStringLiteral("Microsoft"), QStringLiteral("windows"), QStringLiteral("Windows Desktop")),
        ExtnOSVersRef(ExtnOSType::MacOS, QStringLiteral("Apple"), QStringLiteral("macos"), QStringLiteral("Mac OS Desktop")),
        ExtnOSVersRef(ExtnOSType::IOS, QStringLiteral("Apple"), QStringLiteral("ios"), QStringLiteral("iOS Mobile")),
        ExtnOSVersRef(ExtnOSType::TvOS, QStringLiteral("Apple"), QStringLiteral("tvos"), QStringLiteral("TvOS Devices")),
        ExtnOSVersRef(ExtnOSType::WatchOS, QStringLiteral("Apple"), QStringLiteral("watchos"), QStringLiteral("Watch OS")),
        ExtnOSVersRef(ExtnOSType::Android, QStringLiteral("Google"), QStringLiteral("android"), QStringLiteral("Android OS")),
        ExtnOSVersRef(ExtnOSType::VisionOS, QStringLiteral("Apple"), QStringLiteral("visionos"), QStringLiteral("Vision OS")),
        ExtnOSVersRef(ExtnOSType::Linux, QStringLiteral("Linux Foundation"), QStringLiteral("linux"), QStringLiteral("Linux Desktop")),
        ExtnOSVersRef(ExtnOSType::BSD, QStringLiteral("BSD Community"), QStringLiteral("bsd"), QStringLiteral("BSD Unix")),
        ExtnOSVersRef(ExtnOSType::Solaris, QStringLiteral("Oracle"), QStringLiteral("solaris"), QStringLiteral("Solaris OS")),
        ExtnOSVersRef(ExtnOSType::ChromeOS, QStringLiteral("Google"), QStringLiteral("chromeos"), QStringLiteral("Chrome OS")),
        ExtnOSVersRef(ExtnOSType::Custom1, QStringLiteral("Custom"), QStringLiteral("custom1"), QStringLiteral("Custom OS 1")),
        ExtnOSVersRef(ExtnOSType::Custom2, QStringLiteral("Custom"), QStringLiteral("custom2"), QStringLiteral("Custom OS 2"))
    };
}

const ExtnOSVersRef* ExtnOSVersRef::findByOSType(ExtnOSType type) {
    static QVector<ExtnOSVersRef> versions = allOSVersions();
    
    for (const auto& ref : versions) {
        if (ref.osType() == type) {
            return &ref;
        }
    }
    // Return Unknown if not found
    for (const auto& ref : versions) {
        if (ref.osType() == ExtnOSType::Unknown) {
            return &ref;
        }
    }
    return nullptr;
}

} // namespace platformInfo
