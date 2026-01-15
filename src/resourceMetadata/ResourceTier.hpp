/**
 * @file ResourceTier.hpp
 * @brief Resource location tier/level enumeration
 */

#pragma once

#include <QString>
#include <QMap>

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
 * @brief Static map of ResourceTier to display name
 */
inline static const QMap<ResourceTier, QString> s_tierNames = {
    {ResourceTier::Installation, QStringLiteral("Installation")},
    {ResourceTier::Machine, QStringLiteral("Machine")},
    {ResourceTier::User, QStringLiteral("User")}
};
/**
 * @brief Static list of all ResourceTiers
 */
inline static const QList<ResourceTier> s_allTiersList = {
    ResourceTier::Installation,
    ResourceTier::Machine,
    ResourceTier::User
};

/**
 * @brief Get display name for a resource tier
 * 
 * @param tier The tier to get the display name for
 * @return QString display name for the tier
 */
inline QString tierToString(ResourceTier tier) {
    return s_tierNames.value(tier, QStringLiteral("Unknown"));
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
