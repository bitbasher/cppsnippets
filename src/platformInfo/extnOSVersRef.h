/**
 * @file extnOSVersRef.h
 * @brief Extended OS version reference for platform identification
 */

#pragma once

#include "export.h"
#include <QString>
#include <QVector>

namespace platformInfo {

/**
 * @brief Enumeration of supported operating system types
 */
enum class ExtnOSType {
    Unknown = 0,
    Windows,
    MacOS,
    IOS,
    TvOS,
    WatchOS,
    Android,
    VisionOS,
    Linux,
    BSD,
    Solaris,
    ChromeOS,
    Custom1,
    Custom2
};

/**
 * @brief Extended OS version reference information
 * 
 * Provides detailed information about operating systems including
 * company, title, and identifier strings.
 */
class RESOURCEMGMT_API ExtnOSVersRef {
public:
    /**
     * @brief Default constructor - creates Unknown OS reference
     */
    ExtnOSVersRef();
    
    /**
     * @brief Construct with all parameters
     * @param type The OS type enumeration value
     * @param company Company/organization name (e.g., "Microsoft", "Apple")
     * @param title OS title (e.g., "windows", "macos")
     * @param id OS identifier string (e.g., "Windows Desktop")
     */
    ExtnOSVersRef(ExtnOSType type, const QString& company, const QString& title, const QString& id);
    
    /// @brief Get the OS type enumeration
    ExtnOSType osType() const { return m_osType; }
    
    /// @brief Get the company/organization name
    QString osCompany() const { return m_osCompany; }
    
    /// @brief Get the OS title
    QString osTitle() const { return m_osTitle; }
    
    /// @brief Get the OS identifier string
    QString osID() const { return m_osID; }
    
    /**
     * @brief Get all known OS version references
     * @return Vector of all predefined OS references
     */
    static QVector<ExtnOSVersRef> allOSVersions();
    
    /**
     * @brief Find an OS reference by type
     * @param type The OS type to find
     * @return Pointer to matching reference, or nullptr if not found
     */
    static const ExtnOSVersRef* findByOSType(ExtnOSType type);

private:
    ExtnOSType m_osType;
    QString m_osCompany;
    QString m_osTitle;
    QString m_osID;
};

} // namespace platformInfo
