/**
 * @file ResourceTier.hpp
 * @brief Resource location tier/level enumeration
 */

#pragma once

#include <QString>

namespace resourceMetadata {

/**
 * @brief Resource location tier/level
 * 
 * Resources are organized into three tiers based on their scope and accessibility:
 * - Installation: Built-in resources from the application installation (read-only)
 * - Machine: System-wide resources available to all users (read-only)
 * - User: Personal resources in the user's home directory (read-write)
 */
enum class ResourceTier {
    Installation,   ///< Built-in application resources (read-only)
    Machine,        ///< System-wide resources for all users (read-only)
    User            ///< Per-user resources (read-write)
};

/**
 * @brief Get display name for a resource tier (constexpr, no DLL issues)
 * 
 * @param tier The tier to get the display name for
 * @return C-string display name for the tier
 */
constexpr const char* tierToString(ResourceTier tier) noexcept {
    switch(tier) {
        case ResourceTier::Installation: return "Installation";
        case ResourceTier::Machine: return "Machine";
        case ResourceTier::User: return "User";
    }
    return "Unknown";
}

/**
 * @brief Get display name for a resource tier as QString
 * 
 * @param tier The tier to get the display name for
 * @return QString display name for the tier
 */
inline QString tierDisplayName(ResourceTier tier) {
    return QString::fromUtf8(tierToString(tier));
}

/**
 * @brief Convert string to ResourceTier enum
 * 
 * @param str String representation of tier ("Installation", "Machine", or "User")
 * @return ResourceTier enum value (defaults to User if not recognized)
 */
inline ResourceTier stringToTier(const QString& str) {
    if (str == QLatin1String("Installation")) return ResourceTier::Installation;
    if (str == QLatin1String("Machine"))      return ResourceTier::Machine;
    if (str == QLatin1String("User"))         return ResourceTier::User;
    return ResourceTier::User; // Default fallback
}

} // namespace resourceMetadata
